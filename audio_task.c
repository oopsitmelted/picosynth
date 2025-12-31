#include "FreeRTOS.h"
#include "task.h"
#include "audio_task.h"

void vAudioTask(void *pvParameters)
{
	for( ;; )
    {
        // Audio processing code would go here.

        // For this example, just delay for a period.
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}