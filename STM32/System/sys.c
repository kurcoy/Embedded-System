#include "sys.h"
#include "stm32f10x_usart.h"

/*
void NVIC_Configuration(void)
{
		NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//设置NVIC中断分组2:2位抢占优先级，2位响应优先级	
		
  NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;//ADC_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}
*/

USART_TypeDef *UART;

/*******************************************************************************
/******************************************************************************/
void Init_UartPrint( USART_TypeDef* USART )
{
	UART = USART;
}

/*******************************************************************************
 * name: UartPrint()
 * role: print string to the UART
 * in: ptr -> point to the string
 * out: 0
 * note: this interface can only use with the UART TX interrupt is DISABLED
*******************************************************************************/
int UartPrint(char * ptr)
{
  if   ( 0 !=UART  )
	while( *ptr != 0 )
  {
    while (!(UART->SR & 0x00000040));
    USART_SendData(UART, *ptr++);
  }
  return 0;
}
