#ifndef __task1_h__
#define __task1_h__

#include "multi-task.h"

#include "init.h"
#include "sys.h"

#define TASK1_STACK_SIZE	128
#define TASK2_STACK_SIZE  256


extern StackType_t Task1Stack[TASK1_STACK_SIZE] CCM_RAM;  // Put task stack in CCM
extern StaticTask_t Task1Buffer CCM_RAM;  // Put TCB in CCM

extern StackType_t  Task2Stack[TASK2_STACK_SIZE] CCM_RAM;  // Put task stack in CCM
extern StaticTask_t Task2Buffer CCM_RAM;  // Put TCB in CCM

void Task1(void* p);
void Task2(void* p);

#endif /*__task1_h__*/
