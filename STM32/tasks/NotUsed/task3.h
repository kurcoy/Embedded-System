#ifndef __task3_h__
#define __task3_h__

#include "multi-task.h"

#define TASK3_STACK_SIZE	256
#define SPI3_CS_SW_MODE

extern StackType_t Task3Stack[TASK3_STACK_SIZE] CCM_RAM;  // Put task stack in CCM
extern StaticTask_t Task3Buffer CCM_RAM;  // Put TCB in CCM

void Task3(void* p);


#endif /*__task3_h__*/
