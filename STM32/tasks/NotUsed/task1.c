#include "stdio.h"
#include "multi-task.h"
#include "FreeRTOS.h"
#include "init.h"
#include "task1.h"
#include "task4.h"
#include "task6.h"
#include "stm32f4xx_usart.h"

extern const char VersionInfor[];

StackType_t Task1Stack[TASK1_STACK_SIZE] CCM_RAM;  // Put task stack in CCM
StaticTask_t Task1Buffer CCM_RAM;  // Put TCB in CCM

PowerStage_t Powerstate = PowerStage0;
WarningLevel_t Warning = WarningLevelNone;

static char PrintfBuffer[128];

PowerStage_t Board_GetPowerStage(void)
{
  return Powerstate;
}

WarningLevel_t Board_WarningLevel(void)
{
  return Warning;
}

void Task1(void* p)
{
  int32_t timeout = 3;
  int32_t i;
  uint16_t reg16;
  uint8_t tmp;

  /* Clear the screen */
  UartPrint("\r\n 														\r\n");
  UartPrint("\r\n 														\r\n");
  UartPrint("\r\n 														\r\n");
  UartPrint("\r\n 														\r\n");
  UartPrint("**************************************************************\r\n");
  sprintf(PrintfBuffer, "SW version:%s\r\n", VersionInfor);
  UartPrint(PrintfBuffer);
    

  if (Board_CheckCPLDPower() == 1)
  {
	UartPrint("\r\nCPLD power on reset is not OK!\r\n");
  }
  else
  {
	UartPrint("\r\nPower stage1!\r\n");
	Powerstate = PowerStage1;
    
    Board_Reset_cpu();
	vTaskDelay(10);
	Board_ResetCPLD();
	vTaskDelay(10);
	Board_ReleaseCPLD();

#if 0 //skip this step
	/* wait 30ms to check CPLD power completed */
	while (timeout--)
	{
	  tmp = Board_GetCpldState();
	  if (tmp == (uint8_t) 0x12)
	  {
		printf("\r\nCPLD Power Ok!\r\n");
		break;
	  }
	  vTaskDelay(10);
	}

	printf("\r\nCPLD RESETED!\r\n");

	if (timeout <= 0)
	{
	  printf("\r\nCPLD Power not ok![%x]\r\n", tmp);
	  Warning = WarningLevel1;
	}
#endif
	/* DM7304 output default voltage 1.025V */
	Dm7304Release();
	Powerstate = PowerStage2;

	/* monitor the whole board power up in 2 seconds */
	timeout = 100;
	while (timeout--)
	{
	  if (Board_CheckCPLDPower() != 0)
	  {
		UartPrint("\r\nThe whole board power on Ok!\r\n");
		break;
	  }
	  vTaskDelay(10);
	}

	if (timeout <= 0)
	{
	  /* Board power up timeout */
	  UartPrint("\r\nThe whole board power on failed\r\n");
	  Warning = WarningLevel1;
	}

	/* check if need to upgrade FPGA */
	//TODO
	/* Reset FPGA */
	UartPrint("\r\nFPGA reset...\r\n");
	Board_ResetFPGA();
	vTaskDelay(50);
	Board_ReleaseFPGA();

	/* monitor FPGA reset completed in 10 seconds */
	timeout = 1500;
	while (timeout--)
	{
	  if (Board_CheckFPGA() != 0)
	  {
		UartPrint("\r\nFPGA reset OK ...\r\n");
		break;
	  }
	  vTaskDelay(10);
	}
	if (timeout <= 0)
	{
	  UartPrint("\r\nFPGA reset failed!\r\n");
	  /* FPGA reset timeout */
	  Warning = WarningLevel1;
	}
	Powerstate = PowerStage3;
	/* Drive the Front LED in 0.25S period*/
	//TODO
	Board_SdRefClk125mEn(1);

	/* route the MCU's SPI3 to LMK04031 */
    Write_FPGA(0x0022, 0x0006);
	sprintf(PrintfBuffer,"ReadFPGA 0x0022 0x%04x\r\n", Read_FPGA(0x0022));
	UartPrint(PrintfBuffer);

	/* set power state to PowerStage4 to inform FPGA task(task3) to configure the LMK04031 */
	Powerstate = PowerStage4;
	/* check if locked */
	timeout = 200;
	while (timeout--)
	{
	  reg16 = Read_FPGA(0x52);
	  if ((reg16 & 0x04) == 0)
	  {
		sprintf(PrintfBuffer, "\r\nLMK04031 locked[%x]!\r\n", reg16);
		UartPrint(PrintfBuffer);
		break;
	  }
	  vTaskDelay(10);
	}
	if (timeout <= 0)
	{
	  sprintf(PrintfBuffer, "\r\nLMK04031 not locked[%x]!\r\n", reg16);
	  UartPrint(PrintfBuffer);
	  Warning = WarningLevel1;
	}

	/* Release every device via FPGA register 0x0050 */
	reg16 = 0;
	Write_FPGA(0x0050, reg16);
	for (i = 0; i < 16; i++)
	{
	  reg16 |= (uint16_t)(1 << i);
	  Write_FPGA(0x0050, reg16);
	  vTaskDelay(10);
	}

	Powerstate = PowerStage5;

	/* check if CPU voltage locked */
	timeout = 20;
	while (timeout--)
	{
	  reg16 = Read_FPGA(0x20);
	  if ((reg16 & 0x20) != 0)/* CPU voltage is locked */
	  {
		sprintf(PrintfBuffer, "\r\nCPU voltage locked[%x]!\r\n", reg16);
		UartPrint(PrintfBuffer);
		reg16 = (uint16_t)(reg16 & 0x1f);
		/* driver the chip to output the specified voltage */
		if (dpm_pol_set_voltage(1025) != 0)
		{
		  UartPrint("\r\nCPU voltage set failed!\r\n");
		  Warning = WarningLevel2;
		}
		break;
	  }
	  vTaskDelay(10);
	}

	if (timeout <= 0)
	{
	  sprintf(PrintfBuffer, "\r\nFPGA CPU voltage Reg=%x\r\n", reg16);
	  UartPrint(PrintfBuffer);
	  Warning = WarningLevel2;
	}

	UartPrint("\r\nPower on done!\r\n");
    
//    Board_Reset_cpu();

	Powerstate = PowerStage6;

  }

  /* active the remaining task */
  RunAllTask();
  /* ENABLE the Tx buffer empty interrupt */
//  USART_ITConfig(UART4, USART_IT_TC, ENABLE);

  vTaskDelete(NULL);
}
