#ifndef _task_lwip_h_
#define _task_lwip_h_
#include "multi-task.h"

#define TASK_LWIP_STACK_SIZE 1024

extern StackType_t TaskLwIpStack[TASK_LWIP_STACK_SIZE] CCM_RAM;  // Put task stack in CCM
extern StaticTask_t TaskLwIpBuffer CCM_RAM;  // Put TCB in CCM

extern void Task_lwip(void *p);

#endif /*_task_lwip_h_*/
