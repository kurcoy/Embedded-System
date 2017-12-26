#include "multi-task.h"
#include "FreeRTOS.h"
#include "task.h"
#include "task1.h"
#include "task2.h"
#include "task3.h"
#include "task4.h"
#include "task5.h"
#include "task_nor_flash.h"
#include "task6.h"
#include "task7.h"
#include "task8.h"
#include "task_lwip.h"
#include "task_socket.h"
#include "task_socket2.h"

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
#if 0
		{
			Task3,
			"Task3",
			TASK3_STACK_SIZE,
			NULL,
			1,
			Task3Stack,
			&Task3Buffer
		},
		{
			Task4,
			"Task4",
			TASK4_STACK_SIZE,
			NULL,
			1,
			Task4Stack,
			&Task4Buffer
		},

		{
			Task5,
			"Task5",
			TASK5_STACK_SIZE,
			NULL,
			1,
			Task5Stack,
			&Task5Buffer
		},

		{
			Task_NorFlash,
			"Task_NorFlash",
			TASK_NOR_FLASH_STACK_SIZE,
			NULL,
			1,
			TaskNorFlashStack,
			&TaskNorFlashBuffer
		},
        	{
            Task6,
			"Task6",
			TASK6_STACK_SIZE,
			NULL,
			1,
			Task6Stack,
			&Task6Buffer
		},
        	{
            		Task7,
			"Task7",
			TASK7_STACK_SIZE,
			NULL,
			1,
			Task7Stack,
			&Task7Buffer
        	},
        	{
            		Task8,
			"Task8",
			TASK8_STACK_SIZE,
			NULL,
			1,
			Task8Stack,
			&Task8Buffer
        },
#endif
        {
        	Task_lwip,
        	"Task_lwip",
        	TASK_LWIP_STACK_SIZE,
        	NULL,
        	1,
        	TaskLwIpStack,
        	&TaskLwIpBuffer
        },
        {
            Task_socket,
            "Task_socket",
            TASK_SOCKET_STACK_SIZE,
            NULL,
            1,
            TaskSocketStack,
            &TaskSocketBuffer
        },
        {
            Task_socket2,
            "Task_socket",
            TASK_SOCKET_STACK_SIZE2,
            NULL,
            1,
            TaskSocketStack2,
            &TaskSocketBuffer2
	}
};

void EarlyTask(void)
{
	int i;

//	for(i=0; i<sizeof(RtosTask)/sizeof(RtosTask[0]); i++)
	for(i=0; i<3; i++)
	{
		xTaskCreateStatic(RtosTask[i].pxTaskCode,
				RtosTask[i].pcName,
				RtosTask[i].ulStackDepth,
				RtosTask[i].pvParameters,
				RtosTask[i].uxPriority,
				RtosTask[i].puxStackBuffer,
				RtosTask[i].pxTaskBuffer);
	}

	vTaskStartScheduler();  // should never return
}

void RunAllTask(void)
{
	int i;

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
}
