#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "audio_task.h"
#include "i2s.h"
#include "synth_engine.h"
#include "hardware/dma.h"
#include "log_task.h"
#include "app_config.h"

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

static void dma_i2s_in_handler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xAudioISRSemaphore, &xHigherPriorityTaskWoken);
    dma_hw->ints0 = 1u << i2s.dma_ch_in_data;  // clear the IRQ
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void vAudioTaskInit(void)
{
    // Initialize GPIO for debugging
    gpio_init(PIN_DEBUG_TIMING);
    gpio_set_dir(PIN_DEBUG_TIMING, GPIO_OUT);
    gpio_put(PIN_DEBUG_TIMING, 0);

    // Initialize Synth Engine
    synth_engine_init();

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

            gpio_put(PIN_DEBUG_TIMING, 1);

            /* We're double buffering using chained TCBs. By checking which buffer the
            * DMA is currently reading from, we can identify which buffer it has just
            * finished reading (the completion of which has triggered this interrupt).
            */
            if (*(int32_t**)dma_hw->ch[i2s.dma_ch_in_ctrl].read_addr == i2s.input_buffer) {
                // It is inputting to the second buffer so we can overwrite the first
                synth_engine_process(i2s.output_buffer, AUDIO_BUFFER_FRAMES);
            } else {
                // It is currently inputting the first buffer, so we write to the second
                synth_engine_process(&i2s.output_buffer[STEREO_BUFFER_SIZE], AUDIO_BUFFER_FRAMES);
            }
            gpio_put(PIN_DEBUG_TIMING, 0);
        }
        else
        if (xHandle == xAudioQueue) {
            char msg_buf[64];
            AudioMessage_t msg;
            xQueueReceive(xAudioQueue, &msg, 0);

            snprintf(msg_buf, sizeof(msg_buf), "Note on: %d, Freq: %.2f Hz", msg.note_on, msg.frequency);
            log_msg(msg_buf);
            
            synth_engine_set_frequency(msg.frequency);
            if (msg.note_on) {
                synth_engine_note_on();
            } else {
                synth_engine_note_off();
            }
        }
    }
}