#include "stm32f10x.h"
#include "sys.h"
#include "hardwares.h"

/* CPU clock is 72MHz, to delay 1us need 72 CPU cycle */
void udelay(int count)
{
	count *= 72;
	while(count--);
}

void RCC_Configuration(void)
{
	//-------使用外部RC晶振-----------
	RCC_DeInit();			//初始化为缺省值
	RCC_HSEConfig(RCC_HSE_ON);	//使能外部的高速时钟
	while(RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET);//等待高速时钟使能就绪

	FLASH_PrefetchBufferCmd (FLASH_PrefetchBuffer_Enable);//Enable Prefetch Buffer
	FLASH_SetLatency        (FLASH_Latency_2);		// Flash 2 wait state
	RCC_HCLKConfig      	  (RCC_SYSCLK_Div1);			//HCLK =  SYSCLK
	RCC_PCLK2Config					(RCC_HCLK_Div1);			//PCLK2 = HCLK
	RCC_PCLK1Config					(RCC_HCLK_Div2);			//PCLK1 = HCLK/2
	RCC_PLLConfig						(RCC_PLLSource_HSE_Div1,RCC_PLLMul_9);		//PLLCLK = 	8MHz * 9 = 72MHz
	RCC_PLLCmd							(ENABLE);						//Enable PLLCLK
	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) ==  RESET);//Wait till PLL is ready
	RCC_SYSCLKConfig				(RCC_SYSCLKSource_PLLCLK);			//Select PLL as system clock source
	while(RCC_GetSYSCLKSource() != 0x08);				//Wait till PLL is used as system clock source

  //=================================================================
}

void Board_Reset_cpu(void)
{
  /* Reset PD2 */
  GPIO_ResetBits(GPIOD, GPIO_Pin_2);
  
}

void Init_CPU(void)
{
	NVIC_SetVectorTable      (NVIC_VectTab_FLASH, 0);
	NVIC_PriorityGroupConfig (NVIC_PriorityGroup_4 );
	__enable_irq( );

	RCC_Configuration ( );
}

void Init_BSPHardWare(void)
{
	Init_LED   ( );
	Init_UART1 ( );
	Init_UartPrint( USART1 );
}

void Init_EXTHardWare( void )
{
	Init_SPI_AD7606();
	AD_RANGE_10V   ();
}

