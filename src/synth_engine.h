#ifndef SYNTH_ENGINE_H
#define SYNTH_ENGINE_H

#include <stdint.h>
#include <stddef.h>

void synth_engine_init(void);
void synth_engine_set_frequency(float frequency);
void synth_engine_note_on(void);
void synth_engine_note_off(void);
void synth_engine_process(int32_t* output_buffer, size_t num_frames);

#endif /* SYNTH_ENGINE_H */
