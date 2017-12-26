#include "stdio.h"
#include "multi-task.h"
#include "FreeRTOS.h"
#include "init.h"
#include "task1.h"
#include "hardwares.h"

//*******************************************************************************
//TODO:
//1. clean code
//2. update system return value
//3. load task for initialization and normal loop task
//4. define communication with PC, other MCU
//5. add hardwares, programmable resistor, step motor, LCD
//6.

StackType_t  Task1Stack[TASK1_STACK_SIZE] CCM_RAM;  // Put task stack in CCM
StaticTask_t Task1Buffer CCM_RAM;  								  // Put TCB in CCM

StackType_t  Task2Stack[TASK2_STACK_SIZE] CCM_RAM;
StaticTask_t Task2Buffer CCM_RAM;

void Task1(void* p)
{
  int32_t timeout = 3;
  int32_t i;
  uint16_t reg16;
  uint8_t tmp;

  /* Clear the screen */
  UartPrint ("\r\n 																											\r\n");
  UartPrint ("\r\n 																											\r\n");
  UartPrint ("\r\n 																											\r\n");
  UartPrint ("\r\n 																											\r\n");
  UartPrint ("\r\n********************task function1********************\r\n");

  static portTickType xLastWakeTime;
  const  portTickType xFrequency = pdMS_TO_TICKS(500);
  xLastWakeTime = xTaskGetTickCount();

  while(1)
  {
	  Led2Set( 0 );
	  //udelay (100000);
	  vTaskDelayUntil( &xLastWakeTime,xFrequency );
	  Led2Set( 1 );
	  ////////////////////////////////////////



	  ///////////////////////////////////////
	  Led1Set( 0);
	  //udelay (100000);
	  vTaskDelayUntil( &xLastWakeTime,xFrequency );
	  Led1Set( 1);
  }

  vTaskDelete(NULL);
}


void Task2(void* p)
{
  uint16_t sampleVol[8];
  int32_t  int_sampleVol[8];
  char     transStr[30];
  int      i;

  AD7606_StartRecord(200);

  static portTickType xLastWakeTime;
	const  portTickType xFrequency = pdMS_TO_TICKS(500);
	xLastWakeTime = xTaskGetTickCount();

	if( RET_NOERR == AD7606_StartRecord(200) )
	while(1)
	{
		for(i = 0; i < 8; i++)
		{
			sampleVol[i]     = AD7606_GetReading  ( i );
			int_sampleVol[i] = ((int32_t)10000)*((float)((short)sampleVol[i])/32768);	//µ¥Î»1mv
			sprintf(transStr, "Range%d = %d\t", i, int_sampleVol[i]);
			UartPrint((uint8_t*)transStr);
		}
		UartPrint((uint8_t*)"\r\n");

		vTaskDelayUntil( &xLastWakeTime,xFrequency );
	}

  /* active the remaining task */
 // RunAllTask();
  /* ENABLE the Tx buffer empty interrupt */
 //  USART_ITConfig(UART4, USART_IT_TC, ENABLE);

  vTaskDelete(NULL);
}
