
#include "bsp_UART.h"

void Init_UART1(void)
{

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	/* provide clock to the UART1 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;
	//NVIC_InitTypeDef NVIC_InitStructure;

	/* UART1 PA10 RX */
	GPIO_InitTypeDef	GPIO_InitStructure;		//声明一个结构体变量
	GPIO_InitStructure.GPIO_Pin	  = GPIO_Pin_10; 	//USART1_RX
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //管脚频率为50MHZ
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;	 //模式为浮空输入
	GPIO_Init(GPIOA,&GPIO_InitStructure);				 //初始化GPIOC寄存器

	/* UART1 PA9 TX */
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9; 	//USART1_TX
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //管脚频率为50MHZ
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;	 //输出模式为复用推挽输出
	GPIO_Init(GPIOA,&GPIO_InitStructure);				 //初始化GPIOA寄存器

	//将USART1的TXD和RXD重映射到PC10和pc9
	//GPIO_PinRemapConfig(GPIO_Remap_USART1,ENABLE);

	USART_InitStruct.USART_BaudRate 					 = 115200;
	USART_InitStruct.USART_WordLength 				 = USART_WordLength_8b;
	USART_InitStruct.USART_StopBits						 = USART_StopBits_1;
	USART_InitStruct.USART_Parity							 = USART_Parity_No;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode								 = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART1, &USART_InitStruct);

	/*
	//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	// Enable the USARTx Interrupt //
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	*/
	USART_Cmd(USART1, ENABLE);
    
	/*
    // DISABLE the Tx buffer empty interrupt
	USART_ITConfig(UART4, USART_IT_TC, DISABLE);
	//Enable the Rx buffer empty interrupt
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
	*/
}
