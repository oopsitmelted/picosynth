#include "FreeRTOS.h"
#include "task.h"
#include "midi_task.h"

void vMidiTask(void *pvParameters)
{
	for( ;; )
    {
        // MIDI processing code would go here.

        // For this example, just delay for a period.
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}