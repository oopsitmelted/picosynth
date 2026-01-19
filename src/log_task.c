#include "FreeRTOS.h"
#include "task.h"
#include "log_task.h"
#include "queue.h"
#include "semphr.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include <string.h>
#include <stdio.h>

#define UART_PORT uart0
#define UART_BAUD 115200

#define MAX_LOG_MSG_LEN 64

typedef struct {
    char msg[MAX_LOG_MSG_LEN];
    char sender[configMAX_TASK_NAME_LEN];
} LogMessage_t;

static QueueHandle_t xLogQueue = NULL;
static SemaphoreHandle_t xUartTxSem = NULL;
static QueueSetHandle_t xLogQueueSet = NULL;

#define TX_BUFFER_SIZE 512
static uint8_t ucTxBuffer[TX_BUFFER_SIZE];
static volatile size_t uxTxHead = 0;
static volatile size_t uxTxTail = 0;

static bool tx_buffer_put(uint8_t c) {
    size_t next_head = (uxTxHead + 1) % TX_BUFFER_SIZE;
    if (next_head == uxTxTail) return false;
    ucTxBuffer[uxTxHead] = c;
    uxTxHead = next_head;
    return true;
}

static bool tx_buffer_get(uint8_t *c) {
    if (uxTxHead == uxTxTail) return false;
    *c = ucTxBuffer[uxTxTail];
    uxTxTail = (uxTxTail + 1) % TX_BUFFER_SIZE;
    return true;
}

static bool tx_buffer_empty() {
    return uxTxHead == uxTxTail;
}

static void on_uart_irq(void) {
    if (uart_is_writable(UART_PORT)) {
        // Disable TX interrupt
        uart_set_irq_enables(UART_PORT, false, false);
        
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(xUartTxSem, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void vLogTaskInit(void) {
    xLogQueue = xQueueCreate(MAX_LOG_MSG_LEN, sizeof(LogMessage_t));
    xUartTxSem = xSemaphoreCreateBinary();
    xLogQueueSet = xQueueCreateSet(MAX_LOG_MSG_LEN + 1);
    
    xQueueAddToSet(xLogQueue, xLogQueueSet);
    xQueueAddToSet(xUartTxSem, xLogQueueSet);

    uart_set_fifo_enabled(UART_PORT, true);
    uart_init(UART_PORT, UART_BAUD);
    uart_set_format(UART_PORT, 8, 1, UART_PARITY_NONE);
    gpio_set_function(0, GPIO_FUNC_UART); // TX
    gpio_set_function(1, GPIO_FUNC_UART); // RX
    
    irq_set_exclusive_handler(UART0_IRQ, on_uart_irq);
    irq_set_enabled(UART0_IRQ, true);
    uart_set_irq_enables(UART_PORT, false, false);
}

void log_msg(const char *msg) {
    if (xLogQueue == NULL) return; 
    
    LogMessage_t data;
    strncpy(data.msg, msg, MAX_LOG_MSG_LEN);
    data.msg[MAX_LOG_MSG_LEN - 1] = '\0';
    
    TaskHandle_t current = xTaskGetCurrentTaskHandle();
    if (current) {
        strncpy(data.sender, pcTaskGetName(current), configMAX_TASK_NAME_LEN);
    } else {
        strcpy(data.sender, "None");
    }
    data.sender[configMAX_TASK_NAME_LEN - 1] = '\0';
    
    xQueueSendToBack(xLogQueue, &data, 0);
}

void vLoggingTask(void *pvParameters) {
    LogMessage_t msg_data;
    char formatted_buf[128];
    
    log_msg("Logging Task Initialized");

    for(;;)
    {

        QueueSetMemberHandle_t xActivated = xQueueSelectFromSet(xLogQueueSet, portMAX_DELAY); // Block until TX interrupt or message added to queue
        
        if (xActivated == xUartTxSem) {
            xSemaphoreTake(xUartTxSem, 0);
        }

        // Write as much data as possible to UART
        while (!tx_buffer_empty() && uart_is_writable(UART_PORT)) {
            uint8_t c;
            if (tx_buffer_get(&c)) {
                uart_putc_raw(UART_PORT, c);
            }
        }
        
        // Process log queue
        while (uxQueueMessagesWaiting(xLogQueue) > 0) {
            size_t used = (uxTxHead >= uxTxTail) ? (uxTxHead - uxTxTail) : (TX_BUFFER_SIZE - uxTxTail + uxTxHead);
            size_t free = (TX_BUFFER_SIZE - 1) - used;

            if (free < 128) {
                break; // Not enough space, wait for next TX interrupt
            }

            if (xQueueReceive(xLogQueue, &msg_data, 0) == pdTRUE) {
                uint32_t timestamp = ulGetRunTimeCounterValue();
                int len = snprintf(formatted_buf, sizeof(formatted_buf), 
                                   "[%lu] [%s] %s\n", timestamp, msg_data.sender, msg_data.msg);
                
                // Add to circular buffer
                for (int i=0; i<len; i++) {
                    tx_buffer_put(formatted_buf[i]);
                }
            }
        }
        
        if (!tx_buffer_empty()) {
            uart_set_irq_enables(UART_PORT, false, true);
        } else {
            uart_set_irq_enables(UART_PORT, false, false);
        }
        
    }
}