#include "FreeRTOS.h"
#include "task.h"
#include "midi_task.h"
#include "semphr.h"
#include "queue.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include <stdio.h>
#include <string.h>
#include "log_task.h"

#define MIDI_UART_ID      uart1
#define MIDI_BAUD_RATE    31250
#define MIDI_TX_PIN       4
#define MIDI_RX_PIN       5

static SemaphoreHandle_t xMidiRxSem = NULL;
static QueueSetHandle_t xMidiQueueSet = NULL;

void vMidiTaskISR(void)
{
    uart_set_irq_enables(MIDI_UART_ID, false, false);
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xMidiRxSem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void vMidiTaskInit(void)
{

    xMidiRxSem = xSemaphoreCreateBinary();
    xMidiQueueSet = xQueueCreateSet(1);
    
    xQueueAddToSet(xMidiRxSem, xMidiQueueSet);

    // Initialize UART for MIDI communication
    gpio_set_function((uint)MIDI_TX_PIN, UART_FUNCSEL_NUM(MIDI_UART_ID, MIDI_TX_PIN));
    gpio_set_function((uint)MIDI_RX_PIN, UART_FUNCSEL_NUM(MIDI_UART_ID, MIDI_RX_PIN));
    uart_init(MIDI_UART_ID, MIDI_BAUD_RATE);
    uart_set_hw_flow(MIDI_UART_ID, false, false);
    uart_set_format(MIDI_UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(MIDI_UART_ID, false);
    irq_set_exclusive_handler(UART1_IRQ, vMidiTaskISR); 

    // Clear any existing data
    while (uart_is_readable(MIDI_UART_ID)) {
        uart_getc(MIDI_UART_ID);
    }
}

void vMidiTask(void *pvParameters)
{
    // Enable UART IRQ
    irq_set_enabled(UART1_IRQ, true);
    uart_set_irq_enables(MIDI_UART_ID, true, false);
    log_msg("MIDI Task Initialized");
    
	for( ;; )
    {
        QueueSetMemberHandle_t xActivated = xQueueSelectFromSet(xMidiQueueSet, portMAX_DELAY); 
        
        if (xActivated == xMidiRxSem) {
            xSemaphoreTake(xMidiRxSem, 0);
        }

        // Read MIDI data
        while (uart_is_readable(MIDI_UART_ID)) {
            uint8_t byte = uart_getc(MIDI_UART_ID);
            // Log received MIDI byte
            char log_buf[32];
            snprintf(log_buf, sizeof(log_buf), "MIDI RX: 0x%02X", byte);
            log_msg(log_buf);   
        }
        uart_set_irq_enables(MIDI_UART_ID, true, false);
    }
}