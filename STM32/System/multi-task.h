#ifndef __multi_task_h__
#define __multi_task_h__

#include "FreeRTOS.h"
#include "task.h"

#define CCM_RAM __attribute__((section(".ccmram")))

void EarlyTask(void);
//void RunAllTask(void);

typedef struct
{
	TaskFunction_t pxTaskCode;
	const char * const pcName;
	const uint32_t ulStackDepth;
	void * const pvParameters;
	UBaseType_t uxPriority;
	StackType_t * const puxStackBuffer;
	StaticTask_t * const pxTaskBuffer;
} TaskProperty_t;

#endif /*__multi_task_h__*/
