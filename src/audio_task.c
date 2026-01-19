#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "audio_task.h"
#include "i2s.h"
#include "sine.h"
#include "hardware/dma.h"
#include "log_task.h"

static float phase = 0;
static float freq = 440.0f; // Default frequency A4
static uint8_t note_on = 0;

static __attribute__((aligned(8))) pio_i2s i2s;

typedef struct {
    uint8_t note_on;
    float frequency;
} AudioMessage_t;

#define AUDIO_QUEUE_LENGTH 10
static QueueHandle_t xAudioQueue;
static SemaphoreHandle_t xAudioISRSemaphore;
static QueueSetHandle_t xAudioQueueSet;

void vAudioTaskSetFrequency(uint8_t note_on, float frequency) {
    AudioMessage_t msg;
    msg.frequency = frequency;
    msg.note_on = note_on;
    xQueueSendToBack(xAudioQueue, &msg, 0);
}

static void process_audio(const int32_t* input, int32_t* output, size_t num_frames) {
    float phase_increment = freq * 1024.0f / 48000.0f;

    gpio_put(2, 1);
    for (size_t i = 0; i < num_frames * 2; i = i + 2) {
        int16_t sine_val = sine_table[(uint16_t)phase] / 8;

        if (!note_on) {
            sine_val = 0;
        }

        output[i] = sine_val << 16;
        output[i + 1] = sine_val << 16;

        phase += phase_increment;
        if (phase >= 1024.0f) {
            phase -= 1024.0f;
        }
    }
    gpio_put(2, 0);
}

static void dma_i2s_in_handler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xAudioISRSemaphore, &xHigherPriorityTaskWoken);
    dma_hw->ints0 = 1u << i2s.dma_ch_in_data;  // clear the IRQ
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void vAudioTaskInit(void)
{
    // Initialize GPIO for debugging
    gpio_init(2);
    gpio_set_dir(2, GPIO_OUT);
    gpio_put(2, 0);

    // Set up ISR semaphore
    xAudioISRSemaphore = xSemaphoreCreateBinary();

    // Set up Queue Set
    xAudioQueue = xQueueCreate(AUDIO_QUEUE_LENGTH, sizeof(AudioMessage_t));
    xAudioQueueSet = xQueueCreateSet(AUDIO_QUEUE_LENGTH + 1);
    xQueueAddToSet(xAudioISRSemaphore, xAudioQueueSet);
    xQueueAddToSet(xAudioQueue, xAudioQueueSet);
}

void vAudioTask(void *pvParameters)
{
    log_msg("Audio Task Initialized");

    i2s_program_start_synched(pio0, &i2s_config_default, dma_i2s_in_handler, &i2s);
	for( ;; )
    {
        QueueSetMemberHandle_t xHandle = xQueueSelectFromSet(xAudioQueueSet, portMAX_DELAY);

        if (xHandle == xAudioISRSemaphore) {
            
            xSemaphoreTake(xAudioISRSemaphore, 0);

            /* We're double buffering using chained TCBs. By checking which buffer the
            * DMA is currently reading from, we can identify which buffer it has just
            * finished reading (the completion of which has triggered this interrupt).
            */
            if (*(int32_t**)dma_hw->ch[i2s.dma_ch_in_ctrl].read_addr == i2s.input_buffer) {
                // It is inputting to the second buffer so we can overwrite the first
                process_audio(i2s.input_buffer, i2s.output_buffer, AUDIO_BUFFER_FRAMES);
            } else {
                // It is currently inputting the first buffer, so we write to the second
                process_audio(&i2s.input_buffer[STEREO_BUFFER_SIZE], &i2s.output_buffer[STEREO_BUFFER_SIZE], AUDIO_BUFFER_FRAMES);
            }
        }
        else
        if (xHandle == xAudioQueue) {
            char msg_buf[64];
            AudioMessage_t msg;
            xQueueReceive(xAudioQueue, &msg, 0);

            snprintf(msg_buf, sizeof(msg_buf), "Note on: %d, Freq: %.2f Hz", msg.note_on, msg.frequency);
            log_msg(msg_buf);
            freq = msg.frequency;
            note_on = msg.note_on;
        }
    }
}