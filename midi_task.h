#ifndef MIDI_TASK_H
#define MIDI_TASK_H

#include "FreeRTOS.h"
#include "task.h"

void vMidiTaskInit(void);
void vMidiTask(void *pvParameters);

#endif // MIDI_TASK_H