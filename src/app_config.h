#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

/* -----------------------------------------------------------
 * System Configuration
 * ----------------------------------------------------------- */
#define SYS_CLOCK_KHZ           132000

/* -----------------------------------------------------------
 * Task Configuration
 * ----------------------------------------------------------- */
/* Priorities (Higher number = Higher Priority) */
#define PRIORITY_AUDIO_TASK     ( tskIDLE_PRIORITY + 4 )
#define PRIORITY_MIDI_TASK      ( tskIDLE_PRIORITY + 3 )
#define PRIORITY_LOGGING_TASK   ( tskIDLE_PRIORITY + 2 )
#define PRIORITY_ALIVE_TASK     ( tskIDLE_PRIORITY + 1 )

/* Stack Sizes (in words, not bytes) */
#define STACK_SIZE_AUDIO        ( configMINIMAL_STACK_SIZE + 256 )
#define STACK_SIZE_MIDI         ( configMINIMAL_STACK_SIZE + 128 )
#define STACK_SIZE_LOGGING      ( configMINIMAL_STACK_SIZE + 128 )
#define STACK_SIZE_ALIVE        ( configMINIMAL_STACK_SIZE )

/* -----------------------------------------------------------
 * Pin Definitions
 * ----------------------------------------------------------- */
/* Debug / Status */
#define PIN_LED_ALIVE           PICO_DEFAULT_LED_PIN
#define PIN_DEBUG_TIMING        2

/* MIDI (UART) */
#define PIN_MIDI_TX             4
#define PIN_MIDI_RX             5
#define UART_ID_MIDI            uart1
#define BAUD_RATE_MIDI          31250

/* Logging (UART) */
#define PIN_LOG_TX              0
#define PIN_LOG_RX              1
#define UART_ID_LOG             uart0
#define BAUD_RATE_LOG           115200

/* I2S Audio */
#define PIN_I2S_DOUT            6
#define PIN_I2S_DIN             7
#define PIN_I2S_BCLK            8
#define PIN_I2S_LRCK            9
#define PIN_I2S_SCK             10  /* System Clock / Master Clock if needed */

/* -----------------------------------------------------------
 * Audio Settings
 * ----------------------------------------------------------- */
#define AUDIO_SAMPLE_RATE       48000
#define AUDIO_BUFFER_FRAMES     256
#define AUDIO_BIT_DEPTH         16
#define AUDIO_CHANNELS          2

#endif /* APP_CONFIG_H */
