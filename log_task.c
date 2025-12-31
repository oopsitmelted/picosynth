#include "FreeRTOS.h"
#include "task.h"
#include "log_task.h"

void vLoggingTask(void *pvParameters)
{
	for( ;; )
    {
        // Logging code would go here.

        // For this example, just delay for a period.
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}