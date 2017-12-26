#ifndef __task1_h__
#define __task1_h__

#include "multi-task.h"

#define TASK1_STACK_SIZE	256

typedef enum {
  PowerStage0,
  PowerStage1,
  PowerStage2,
  PowerStage3,
  PowerStage4,
  PowerStage5,
  PowerStage6,

  PowerStageN
}PowerStage_t;

typedef enum {
  WarningLevel1,
  WarningLevel2,
  WarningLevel3,

  WarningLevelNone
}WarningLevel_t;

extern StackType_t Task1Stack[TASK1_STACK_SIZE] CCM_RAM;  // Put task stack in CCM
extern StaticTask_t Task1Buffer CCM_RAM;  // Put TCB in CCM

void Task1(void* p);

PowerStage_t Board_GetPowerStage(void);
WarningLevel_t Board_WarningLevel(void);

#endif /*__task1_h__*/
