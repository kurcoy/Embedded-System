#ifndef _task_socket_h_
#define _task_socket_h_
#include "multi-task.h"

#define TASK_SOCKET_STACK_SIZE 1024

extern StackType_t TaskSocketStack[TASK_SOCKET_STACK_SIZE] CCM_RAM;  // Put task stack in CCM
extern StaticTask_t TaskSocketBuffer CCM_RAM;  // Put TCB in CCM

extern void Task_socket(void *p);

#endif /*_task_socket_h_*/
