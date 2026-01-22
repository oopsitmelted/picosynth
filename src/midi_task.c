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
#include <math.h>
#include "log_task.h"
#include "audio_task.h"
#include "app_config.h"
#include "midi_parser.h"

static SemaphoreHandle_t xMidiRxSem = NULL;
static QueueSetHandle_t xMidiQueueSet = NULL;

static void on_note_on(uint8_t note, uint8_t velocity) {
    float frequency = 440.0f * powf(2.0f, (note - 69) / 12.0f);
    vAudioTaskSetFrequency(1, frequency);
    
    char log_buf[32];
    snprintf(log_buf, sizeof(log_buf), "Note On: %d", note);
    log_msg(log_buf);
}

static void on_note_off(uint8_t note) {
    vAudioTaskSetFrequency(0, 0.0f);
    
    char log_buf[32];
    snprintf(log_buf, sizeof(log_buf), "Note Off: %d", note);
    log_msg(log_buf);
}

void vMidiTaskISR(void)
{
    uart_set_irq_enables(UART_ID_MIDI, false, false);
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xMidiRxSem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void vMidiTaskInit(void)
{
    xMidiRxSem = xSemaphoreCreateBinary();
    xMidiQueueSet = xQueueCreateSet(1);
    
    xQueueAddToSet(xMidiRxSem, xMidiQueueSet);

    // Initialize Parser
    midi_parser_init(on_note_on, on_note_off);

    // Initialize UART for MIDI communication
    gpio_set_function((uint)PIN_MIDI_TX, UART_FUNCSEL_NUM(UART_ID_MIDI, PIN_MIDI_TX));
    gpio_set_function((uint)PIN_MIDI_RX, UART_FUNCSEL_NUM(UART_ID_MIDI, PIN_MIDI_RX));
    uart_init(UART_ID_MIDI, BAUD_RATE_MIDI);
    uart_set_hw_flow(UART_ID_MIDI, false, false);
    uart_set_format(UART_ID_MIDI, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(UART_ID_MIDI, true);
    
    // Determine IRQ number based on UART ID
    int uart_irq = (UART_ID_MIDI == uart0) ? UART0_IRQ : UART1_IRQ;
    irq_set_exclusive_handler(uart_irq, vMidiTaskISR); 

    // Clear any existing data
    while (uart_is_readable(UART_ID_MIDI)) {
        uart_getc(UART_ID_MIDI);
    }
}

void vMidiTask(void *pvParameters)
{
    // Enable UART IRQ
    int uart_irq = (UART_ID_MIDI == uart0) ? UART0_IRQ : UART1_IRQ;
    irq_set_enabled(uart_irq, true);
    uart_set_irq_enables(UART_ID_MIDI, true, false);
    log_msg("MIDI Task Initialized");
    
	for( ;; )
    {
        QueueSetMemberHandle_t xActivated = xQueueSelectFromSet(xMidiQueueSet, portMAX_DELAY); 
        
        if (xActivated == xMidiRxSem) {
            xSemaphoreTake(xMidiRxSem, 0);
        }

        // Read MIDI data
        while (uart_is_readable(UART_ID_MIDI)) {
            uint8_t byte = uart_getc(UART_ID_MIDI);
            midi_parser_process_byte(byte);
        }
        uart_set_irq_enables(UART_ID_MIDI, true, false);
    }
}