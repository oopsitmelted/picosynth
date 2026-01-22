#include "midi_parser.h"
#include <stddef.h>

static midi_note_on_callback_t note_on_cb = NULL;
static midi_note_off_callback_t note_off_cb = NULL;

static uint8_t status = 0;
static uint8_t data_count = 0;
static uint8_t data1 = 0;

void midi_parser_init(midi_note_on_callback_t on_cb, midi_note_off_callback_t off_cb) {
    note_on_cb = on_cb;
    note_off_cb = off_cb;
    status = 0;
    data_count = 0;
}

static void process_message(uint8_t st, uint8_t d1, uint8_t d2) {
    uint8_t command = st & 0xF0;
    
    if (command == 0x90) { // Note On
        if (d2 > 0) {
            if (note_on_cb) note_on_cb(d1, d2);
        } else {
            // Note On with velocity 0 is Note Off
            if (note_off_cb) note_off_cb(d1);
        }
    } else if (command == 0x80) { // Note Off
        if (note_off_cb) note_off_cb(d1);
    }
}

static uint8_t get_expected_data_count(uint8_t status) {
    uint8_t cmd = status & 0xF0;
    // 2 byte messages (1 data byte)
    if (cmd == 0xC0 || cmd == 0xD0) return 1;
    // System Common
    if (cmd == 0xF0) {
        if (status == 0xF1 || status == 0xF3) return 1;
        if (status == 0xF2) return 2;
        return 0; 
    }
    // 3 byte messages (2 data bytes): 80, 90, A0, B0, E0
    return 2; 
}

void midi_parser_process_byte(uint8_t byte) {
    // Real-Time messages (0xF8 - 0xFF) can occur anywhere and should not affect running status
    if (byte >= 0xF8) {
        return;
    }

    if (byte & 0x80) { // Status byte
        status = byte;
        data_count = 0;
        
        // Handle 1-byte messages (no data bytes)
        // e.g., F6 (Tune Request). We should process immediately if we supported it.
        // For now, we just reset state.
        
    } else { // Data byte
        if (status == 0) return; // Ignore data if no valid status

        uint8_t expected = get_expected_data_count(status);
        
        if (expected == 0) return; // Should not happen for handled statuses

        if (data_count == 0) {
            data1 = byte;
            data_count++;
        } else if (data_count == 1) {
            if (expected == 2) {
                process_message(status, data1, byte);
                data_count = 0; // Complete
            } else {
                // Unexpected extra byte for 1-byte data message?
                // Treat as start of next message (Running Status)?
                // e.g. C0 01 02 -> C0 01 (Done), C0 02 (Running Status).
                // So if we are here (count 1), and expected was 1...
                // We should have processed at count 1!
            }
        }
        
        // Re-eval logic for variable length support
        if (data_count == expected) {
            // If expected is 1, data1 is the byte. d2 is 0.
            // If expected is 2, data1 is set, byte is d2.
            uint8_t d2 = (expected == 2) ? byte : 0;
            process_message(status, data1, d2);
            
            // Handle Running Status
            // Reset data_count to 0 to capture next message
            data_count = 0;
        }
    }
}