
#include "bsp_LED.h"

void Led1Set(int level)
{
  if(level != 0)
  {
  	GPIO_SetBits(GPIOB, GPIO_Pin_13);
  }
  else
  {
  	GPIO_ResetBits(GPIOB, GPIO_Pin_13);
  }
}

void Led2Set(int level)
{
  if(level != 0)
  {
  	GPIO_SetBits(GPIOB, GPIO_Pin_14);
  }
  else
  {
  	GPIO_ResetBits(GPIOB, GPIO_Pin_14);

  }
}


void Init_LED (void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitTypeDef GPIO_InitStruct;
	/* MCU LED1 and LED2 */
	GPIO_InitStruct.GPIO_Pin 	 = GPIO_Pin_13 | GPIO_Pin_14;
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	GPIO_SetBits( GPIOB, GPIO_Pin_13 | GPIO_Pin_14 );
}
