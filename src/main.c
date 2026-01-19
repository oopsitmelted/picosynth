/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"

/* Library includes. */
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"

/* Task includes */
#include "log_task.h"
#include "audio_task.h"
#include "midi_task.h"

static void prvSetupHardware( void );
void vApplicationMallocFailedHook( void );
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );

#define AUDIO_TASK_PRIORITY		    ( tskIDLE_PRIORITY + 4 )
#define	MIDI_TASK_PRIORITY		    ( tskIDLE_PRIORITY + 3 )
#define	LOGGING_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define	ALIVE_TASK_PRIORITY		    ( tskIDLE_PRIORITY + 1 )

#define aliveTASK_LED				( PICO_DEFAULT_LED_PIN )

// This function returns a raw 32-bit microsecond counter for task profiling.
uint32_t ulGetRunTimeCounterValue( void )
{
    return time_us_32();
}

/*-----------------------------------------------------------*/

void vAliveTask(void *pvParameters)
{
	for( ;; )
    {
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_xor_mask( 1u << aliveTASK_LED );
        //log_msg("Alive Task Heartbeat");
    }
}

int main( void )
{

    prvSetupHardware();

    /* Create logging task */
    xTaskCreate(vLoggingTask,				        /* The function that implements the task. */
				"Log", 							    /* The text name assigned to the task - for debug only as it is not used by the kernel. */
				configMINIMAL_STACK_SIZE, 			/* The size of the stack to allocate to the task. */
				NULL, 								/* The parameter passed to the task - not used in this case. */
				LOGGING_TASK_PRIORITY, 	            /* The priority assigned to the task. */
				NULL );								/* The task handle is not required, so NULL is passed. */

    /* Create audio task */
    xTaskCreate(vAudioTask,				            /* The function that implements the task. */
				"Audio", 							/* The text name assigned to the task - for debug only as it is not used by the kernel. */
				configMINIMAL_STACK_SIZE, 			/* The size of the stack to allocate to the task. */
				NULL, 								/* The parameter passed to the task - not used in this case. */
				AUDIO_TASK_PRIORITY, 	            /* The priority assigned to the task. */
				NULL );								/* The task handle is not required, so NULL is passed. */

    /* Create MIDI task */
    xTaskCreate(vMidiTask,				            /* The function that implements the task. */
				"MIDI", 							/* The text name assigned to the task - for debug only as it is not used by the kernel. */
				configMINIMAL_STACK_SIZE, 			/* The size of the stack to allocate to the task. */
				NULL, 								/* The parameter passed to the task - not used in this case. */
				MIDI_TASK_PRIORITY, 	            /* The priority assigned to the task. */
				NULL );								/* The task handle is not required, so NULL is passed. */

    /* Create Alive task */
    xTaskCreate(vAliveTask,				            /* The function that implements the task. */
				"Alive", 							/* The text name assigned to the task - for debug only as it is not used by the kernel. */
				configMINIMAL_STACK_SIZE, 			/* The size of the stack to allocate to the task. */
				NULL, 								/* The parameter passed to the task - not used in this case. */
				ALIVE_TASK_PRIORITY, 	            /* The priority assigned to the task. */
				NULL );	

    /* Initialize Tasks */
    vLogTaskInit();
    vAudioTaskInit();
    vMidiTaskInit();

    /* Start the tasks and timer running. */
	vTaskStartScheduler();
    return 0;
}
/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
    // Set a 132.000 MHz system clock to more evenly divide the audio frequencies
    set_sys_clock_khz(132000, true);
    //stdio_init_all();
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, 1);
    gpio_put(PICO_DEFAULT_LED_PIN, !PICO_DEFAULT_LED_PIN_INVERTED);
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
    /* Called if a call to pvPortMalloc() fails because there is insufficient
    free memory available in the FreeRTOS heap.  pvPortMalloc() is called
    internally by FreeRTOS API functions that create tasks, queues, software
    timers, and semaphores.  The size of the FreeRTOS heap is set by the
    configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */

    /* Force an assert. */
    configASSERT( ( volatile void * ) NULL );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) pxTask;

    /* Run time stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected. */

    /* Force an assert. */
    configASSERT( ( volatile void * ) NULL );
}
/*-----------------------------------------------------------*/


