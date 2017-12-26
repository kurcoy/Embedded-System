#include "multi-task.h"
#include "FreeRTOS.h"
#include "task.h"
#include "task1.h"

TaskProperty_t RtosTask[] =
{
		{
			Task1,
			"Task1",
			TASK1_STACK_SIZE,
			NULL,
			1,
			Task1Stack,
			&Task1Buffer
		},

		{
			Task2,
			"Task2",
			TASK2_STACK_SIZE,
			NULL,
			1,
			Task2Stack,
			&Task2Buffer
		},

		//to be added as needed
#if 0
#endif

};

void EarlyTask(void)
{
	int i;

	for(i=0; i<sizeof(RtosTask)/sizeof(RtosTask[0]); i++)
//	for(i=0; i<1; i++)
	{
		xTaskCreateStatic(RtosTask[i].pxTaskCode,
				RtosTask[i].pcName,
				RtosTask[i].ulStackDepth,
				RtosTask[i].pvParameters,
				RtosTask[i].uxPriority,
				RtosTask[i].puxStackBuffer,
				RtosTask[i].pxTaskBuffer);
	}
	//UartPrint("\r\n*****Start to schedule********************\r\n");
	vTaskStartScheduler();  // should never return
	//UartPrint("\r\n*****Schedule failed********************\r\n");
}

void RunAllTask(void)
{
	int i;

/*
#if 1
	for(i=3; i<sizeof(RtosTask)/sizeof(RtosTask[0]); i++)
	{
		xTaskCreateStatic(RtosTask[i].pxTaskCode,
				RtosTask[i].pcName,
				RtosTask[i].ulStackDepth,
				RtosTask[i].pvParameters,
				RtosTask[i].uxPriority,
				RtosTask[i].puxStackBuffer,
				RtosTask[i].pxTaskBuffer);
	}
#endif
*/
}

