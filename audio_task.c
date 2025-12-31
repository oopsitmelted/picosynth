#include "FreeRTOS.h"
#include "task.h"
#include "audio_task.h"
#include "i2s.h"
#include "sine.h"
#include "hardware/dma.h"

float phase = 0;
const float phase_increment = 440.0f * 1024.0f / 48000.0f; // 440Hz tone

static __attribute__((aligned(8))) pio_i2s i2s;

static void process_audio(const int32_t* input, int32_t* output, size_t num_frames) {
    gpio_put(2, 1);
    for (size_t i = 0; i < num_frames * 2; i = i + 2) {
        int16_t sine_val = sine_table[(uint16_t)phase] / 8;
        output[i] = sine_val << 16;
        output[i + 1] = sine_val << 16;
        //output[i] = 0x89ABCDEF;
        phase += phase_increment;
        if (phase >= 1024.0f) {
            phase -= 1024.0f;
        }
    }
    gpio_put(2, 0);
}

static void dma_i2s_in_handler(void) {
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
    dma_hw->ints0 = 1u << i2s.dma_ch_in_data;  // clear the IRQ
}

void vAudioTask(void *pvParameters)
{
    i2s_program_start_synched(pio0, &i2s_config_default, dma_i2s_in_handler, &i2s);
	for( ;; )
    {
        // Audio processing code would go here.

        // For this example, just delay for a period.
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}