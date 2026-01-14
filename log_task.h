#ifndef LOG_TASK_H
#define LOG_TASK_H

#include "FreeRTOS.h"
#include "task.h"

void vLogTaskInit(void);
void log_msg(const char *msg);
void vLoggingTask(void *pvParameters);
    
#endif // LOG_TASK_H