#ifndef __taskt_h__
#define __taskt_h__

#include "multi-task.h"

#define TASK5_STACK_SIZE	256
#define TASK5_PERIOD	20 /* 20Mms*/

extern StackType_t Task5Stack[TASK5_STACK_SIZE] CCM_RAM;  // Put task stack in CCM
extern StaticTask_t Task5Buffer CCM_RAM;  // Put TCB in CCM

void Task5(void* p);


#endif/*__taskt_h__*/
