#include "FreeRTOS.h"
#include "task.h"
#include "midi_task.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"

#define MIDI_UART_ID      uart1
#define MIDI_BAUD_RATE    31250
#define MIDI_TX_PIN       4
#define MIDI_RX_PIN       5

void vMidiTaskISR(void)
{
    // Handle MIDI UART interrupt (e.g., read incoming MIDI data)
    while (uart_is_readable(MIDI_UART_ID)) {
        uint8_t midi_byte = uart_getc(MIDI_UART_ID);
        // Process the received MIDI byte (not implemented here)
    }
}

void vMidiTaskInit(void)
{
    // Initialize UART for MIDI communication
    gpio_set_function((uint)MIDI_TX_PIN, UART_FUNCSEL_NUM(MIDI_UART_ID, MIDI_TX_PIN));
    gpio_set_function((uint)MIDI_RX_PIN, UART_FUNCSEL_NUM(MIDI_UART_ID, MIDI_RX_PIN));
    uart_init(MIDI_UART_ID, MIDI_BAUD_RATE);
    uart_set_hw_flow(MIDI_UART_ID, false, false);
    uart_set_format(MIDI_UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(MIDI_UART_ID, false);
    irq_set_exclusive_handler(UART1_IRQ, vMidiTaskISR); // No IRQ handler for now
}

void vMidiTask(void *pvParameters)
{
    // Enable UART IRQ
    irq_set_enabled(UART1_IRQ, true);
    uart_set_irq_enables(MIDI_UART_ID, true, false);
    
	for( ;; )
    {
        // MIDI processing code would go here.

        // For this example, just delay for a period.
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}