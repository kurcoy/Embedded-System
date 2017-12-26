#include "multi-task.h"
#include "FreeRTOS.h"
#include "task2.h"
#include "task1.h"

StackType_t Task2Stack[TASK2_STACK_SIZE] CCM_RAM;  // Put task stack in CCM
StaticTask_t Task2Buffer CCM_RAM;  // Put TCB in CCM

void Task2(void* p)
{
  PowerStage_t state;
  WarningLevel_t warning;
  static int toggle = 0;
  while (1)
  {
	state = Board_GetPowerStage();
	warning = Board_WarningLevel();

	if (warning == WarningLevelNone)
	{
	  switch (state)
	  {
		case PowerStage0:
		  Led1Set(0);
		  Led2Set(0);
		  vTaskDelay(100); /* 100 ms */
		  break;
		case PowerStage1:
		  Led1Set(1);/* LED1 On */
		  Led2Set(0);/* LED2 Off */
		  vTaskDelay(100); /* 100 ms */
		  break;
		case PowerStage2:
		  break;
		case PowerStage3:
		  break;
		case PowerStage4:
		  break;
		case PowerStage5:
		  if (toggle != 0)
		  {
			toggle = 0;
			Led1Set(toggle);/* LED1 off */
		  }
		  else
		  {
			toggle = 1;
			Led1Set(toggle);/* LED1 On */
		  }
		  Led2Set(0);/* LED2 Off */
		  vTaskDelay(500); /* 100 ms */
		  break;
		case PowerStage6:
		  break;
		default:
		  break;
	  }
	}
	else
	{
	  Led2Set(0);/* LED2 off */
	  if (toggle != 0)
	  {
		toggle = 0;
		Led1Set(toggle);/* LED1 off */
		Led2Set(toggle);/* LED2 off */
	  }
	  else
	  {
		toggle = 1;
		Led1Set(toggle);/* LED1 oon */
		Led2Set(toggle);/* LED2 oon */
	  }
	  vTaskDelay(200); /* 200 ms */
	}
  }

  vTaskDelete(NULL);
}


