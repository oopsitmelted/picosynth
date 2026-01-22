/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"

/* Library includes. */
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"

/* App Configuration */
#include "app_config.h"

/* Task includes */
#include "log_task.h"
#include "audio_task.h"
#include "midi_task.h"

static void prvSetupHardware( void );
void vApplicationMallocFailedHook( void );
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );

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
        gpio_xor_mask( 1u << PIN_LED_ALIVE );
        //log_msg("Alive Task Heartbeat");
    }
}

int main( void )
{

    prvSetupHardware();

    /* Create logging task */
    xTaskCreate(vLoggingTask,				        /* The function that implements the task. */
				"Log", 							    /* The text name assigned to the task - for debug only as it is not used by the kernel. */
				STACK_SIZE_LOGGING, 			    /* The size of the stack to allocate to the task. */
				NULL, 								/* The parameter passed to the task - not used in this case. */
				PRIORITY_LOGGING_TASK, 	            /* The priority assigned to the task. */
				NULL );								/* The task handle is not required, so NULL is passed. */

    /* Create audio task */
    xTaskCreate(vAudioTask,				            /* The function that implements the task. */
				"Audio", 							/* The text name assigned to the task - for debug only as it is not used by the kernel. */
				STACK_SIZE_AUDIO, 			        /* The size of the stack to allocate to the task. */
				NULL, 								/* The parameter passed to the task - not used in this case. */
				PRIORITY_AUDIO_TASK, 	            /* The priority assigned to the task. */
				NULL );								/* The task handle is not required, so NULL is passed. */

    /* Create MIDI task */
    xTaskCreate(vMidiTask,				            /* The function that implements the task. */
				"MIDI", 							/* The text name assigned to the task - for debug only as it is not used by the kernel. */
				STACK_SIZE_MIDI, 			        /* The size of the stack to allocate to the task. */
				NULL, 								/* The parameter passed to the task - not used in this case. */
				PRIORITY_MIDI_TASK, 	            /* The priority assigned to the task. */
				NULL );								/* The task handle is not required, so NULL is passed. */

    /* Create Alive task */
    xTaskCreate(vAliveTask,				            /* The function that implements the task. */
				"Alive", 							/* The text name assigned to the task - for debug only as it is not used by the kernel. */
				STACK_SIZE_ALIVE, 			        /* The size of the stack to allocate to the task. */
				NULL, 								/* The parameter passed to the task - not used in this case. */
				PRIORITY_ALIVE_TASK, 	            /* The priority assigned to the task. */
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
    set_sys_clock_khz(SYS_CLOCK_KHZ, true);
    //stdio_init_all();
    gpio_init(PIN_LED_ALIVE);
    gpio_set_dir(PIN_LED_ALIVE, 1);
    gpio_put(PIN_LED_ALIVE, !PICO_DEFAULT_LED_PIN_INVERTED);
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


