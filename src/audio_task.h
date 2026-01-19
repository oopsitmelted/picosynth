#ifndef AUDIO_TASK_H
#define AUDIO_TASK_H

#include "FreeRTOS.h"
#include "task.h"

void vAudioTask(void *pvParameters);
void vAudioTaskInit(void);
void vAudioTaskSetFrequency(uint8_t note_on, float frequency);

#endif // AUDIO_TASK_H