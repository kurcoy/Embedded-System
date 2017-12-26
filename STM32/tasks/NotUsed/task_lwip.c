#include "multi-task.h"
#include "FreeRTOS.h"
#include "task_lwip.h"
#include "main.h"
#include "mii.h"

StackType_t TaskLwIpStack[TASK_LWIP_STACK_SIZE] CCM_RAM;  // Put task stack in CCM
StaticTask_t TaskLwIpBuffer CCM_RAM;  // Put TCB in CCM

#define SYSTEMTICK_PERIOD_MS  10

static __IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */

/*******************************************************************************
 * @name: SysRefTime
 * @input: void
 * @output: the Reference time
 * @role: return the reference time every 10ms
*******************************************************************************/
uint32_t SysRefTime(void)
{
  return LocalTime;
}

void Time_Update(void)
{
	LocalTime += SYSTEMTICK_PERIOD_MS;
	TIM_ClearFlag(TIM3, TIM_FLAG_Update);
}

static char task_lwip_buffer[32];

void PtpInit(void);

void Task_lwip(void *p)
{
	uint32_t tmp = 0;
	vTaskDelay(300);/* 300ms */

	ETH_BSP_Config();
	/* enable the 1588 clock */
	PtpInit();

	LwIP_Init();
//	httpd_init();
//	IAP_httpd_init();

#if 0
	EPLWriteReg(0, PHY_PG4_PTP_CTL, P640_PTP_ENABLE);

	tmp = EPLReadReg(0, PHY_PG4_PTP_CTL);
	sprintf(task_lwip_buffer,"PHY_PG4_PTP_CTL = %x\r\n", tmp);
	dbg_printf(task_lwip_buffer);

	tmp = EPLReadReg(0, 2);
	sprintf(task_lwip_buffer,"PHYIDR1 = %x\r\n", tmp);
	dbg_printf(task_lwip_buffer);

	tmp = EPLReadReg(0, 3);
	sprintf(task_lwip_buffer,"PHYIDR2 = %x\r\n", tmp);
	dbg_printf(task_lwip_buffer);
#endif

	while (1)
	{
		LwIP_Periodic_Handle(LocalTime);
	}

	vTaskDelete(NULL);
}
