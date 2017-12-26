#ifndef _task_socket2_h_
#define _task_socket2_h_
#include "multi-task.h"

#define TASK_SOCKET_STACK_SIZE2 1024

extern StackType_t TaskSocketStack2[TASK_SOCKET_STACK_SIZE2] CCM_RAM;  // Put task stack in CCM
extern StaticTask_t TaskSocketBuffer2 CCM_RAM;  // Put TCB in CCM

extern void Task_socket2(void *p);

#endif /*_task_socket_h_*/
