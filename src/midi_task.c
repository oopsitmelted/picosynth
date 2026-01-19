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
#include "audio_task.h"
#include <math.h>

#define MIDI_UART_ID      uart1
#define MIDI_BAUD_RATE    31250
#define MIDI_TX_PIN       4
#define MIDI_RX_PIN       5

static SemaphoreHandle_t xMidiRxSem = NULL;
static QueueSetHandle_t xMidiQueueSet = NULL;

// Midi Note on/off state machine

void vMidiProcessNote(uint8_t status, uint8_t data1, uint8_t data2)
{
    uint8_t command = status & 0xF0;
    if (command == 0x90 && data2 > 0) { // Note On
        float frequency = 440.0f * powf(2.0f, (data1 - 69) / 12.0f);
        vAudioTaskSetFrequency(1, frequency); // Note on
    } else if (command == 0x80 || (command == 0x90 && data2 == 0)) { // Note Off
        vAudioTaskSetFrequency(0, 0.0f); // Note off
    }
}

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
            static uint8_t status = 0;
            static uint8_t data1 = 0;
            static uint8_t data2 = 0;
            static uint8_t data_count = 0;

            uint8_t byte = uart_getc(MIDI_UART_ID);
            // Log received MIDI byte
            char log_buf[32];
            snprintf(log_buf, sizeof(log_buf), "MIDI RX: 0x%02X", byte);
            log_msg(log_buf);   

            // Process MIDI byte
            if (byte & 0x80) { // Status byte
                status = byte;
                data_count = 0;
            } else { // Data byte
                if (data_count == 0) {
                    data1 = byte;
                    data_count++;
                } else if (data_count == 1) {
                    data2 = byte;
                    data_count = 0;
                    vMidiProcessNote(status, data1, data2);
                }
            }
        }
        uart_set_irq_enables(MIDI_UART_ID, true, false);
    }
}