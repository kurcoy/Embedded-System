#ifndef __task2_h__
#define __task2_h__

#include "multi-task.h"

#define TASK2_STACK_SIZE	256

extern StackType_t Task2Stack[TASK2_STACK_SIZE] CCM_RAM;  // Put task stack in CCM
extern StaticTask_t Task2Buffer CCM_RAM;  // Put TCB in CCM

void Task2(void* p);


#endif /*__task2_h__*/
