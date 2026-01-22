#include "synth_engine.h"
#include "sine.h"
#include "app_config.h"

static float phase = 0;
static float freq = 440.0f; // Default frequency A4
static uint8_t note_is_on = 0;

void synth_engine_init(void) {
    phase = 0;
    freq = 440.0f;
    note_is_on = 0;
}

void synth_engine_set_frequency(float frequency) {
    freq = frequency;
}

void synth_engine_note_on(void) {
    note_is_on = 1;
}

void synth_engine_note_off(void) {
    note_is_on = 0;
}

void synth_engine_process(int32_t* output_buffer, size_t num_frames) {
    float phase_increment = freq * 1024.0f / (float)AUDIO_SAMPLE_RATE;

    for (size_t i = 0; i < num_frames * 2; i = i + 2) {
        int16_t sine_val = sine_table[(uint16_t)phase] / 8;

        if (!note_is_on) {
            sine_val = 0;
        }

        output_buffer[i] = sine_val << 16;
        output_buffer[i + 1] = sine_val << 16;

        phase += phase_increment;
        if (phase >= 1024.0f) {
            phase -= 1024.0f;
        }
    }
}
