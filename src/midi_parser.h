#ifndef MIDI_PARSER_H
#define MIDI_PARSER_H

#include <stdint.h>

typedef void (*midi_note_on_callback_t)(uint8_t note, uint8_t velocity);
typedef void (*midi_note_off_callback_t)(uint8_t note);

void midi_parser_init(midi_note_on_callback_t on_cb, midi_note_off_callback_t off_cb);
void midi_parser_process_byte(uint8_t byte);

#endif /* MIDI_PARSER_H */
