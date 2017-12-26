/*
*********************************************************************************************************
*	                                  
*	模块名称 : AD7606驱动模块
*	文件名称 : spi_AD7606.c
*	版    本 :   v1.0
*	说    明 :   驱动AD7606 ADC转换器 SPI接口
*
*
*********************************************************************************************************
*/
#include "stm32f10x.h"
#include <stdio.h>
#include "spi_ad7606.h"

//*******************************************************************************************************
#define ChannNum 8 					    /* 采集通道 */
//#define FIFO_SIZE	1*1024*2		/* 大小不要超过48K (CPU内部RAM 只有64K) */

typedef struct
{
	uint16_t usRead;
	uint16_t usWrite;
	uint16_t usCount;
	uint16_t usBuf[ChannNum];
}FIFO_T;

FIFO_T	g_tAD;	/* 定义一个交换缓冲区，用于存储AD采集数据，并用于写入SD卡 */

//*******************************************************************************************************
void 		 bsp_TIM4_Configuration 		(void);
void 		 AD7606_StartConv						(void);
void 		 bsp_TIM4_Configuration 		(void);
void 		 bsp_SET_TIM4_FREQ	  		  (uint32_t _ulFreq);
uint16_t AD7606_ReadSingleChannel		(void);
void     AD7606_ReadChannels     		(void);
void		 AD7606_IRQSrc							(void);

/*
*********************************************************************************************************
*	函 数 名: bsp_InitAD7606
*	功能说明: 初始化AD7606 SPI口线
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Init_SPI_AD7606(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	SPI_InitTypeDef   SPI_InitStructure;
	
	RCC_APB2PeriphClockCmd(AD_CS_GPIO_CLK | AD_SPI_MISO_GPIO_CLK | AD_SPI_SCK_GPIO_CLK, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin 	= AD_SPI_SCK_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF_PP;
	GPIO_Init(AD_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin  = AD_CS_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //GPIO_Mode_Out_PP;
	GPIO_Init(AD_CS_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = AD_SPI_MISO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(AD_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

	/////////////////////////SPI_Init////////////////////////

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode		  = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize  = SPI_DataSize_16b;
	SPI_InitStructure.SPI_CPOL 			= SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA 			= SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS 			= SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
	SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;
	//SPI_InitStructure.SPI_CRCPolynomial 	  = 7;
	SPI_Init(AD_SPI, &SPI_InitStructure);
	SPI_Cmd (AD_SPI,  ENABLE);

	/* 使能GPIO时钟 */
	RCC_APB2PeriphClockCmd(AD_RESET_GPIO_CLK | AD_CONVST_GPIO_CLK | AD_RANGE_GPIO_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(AD_OS0_GPIO_CLK   | AD_OS1_GPIO_CLK    | AD_OS2_GPIO_CLK,   ENABLE);

	/* 配置RESET GPIO */
	GPIO_InitStructure.GPIO_Pin 	= AD_RESET_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;
	GPIO_Init(AD_RESET_GPIO_PORT, &GPIO_InitStructure);
	
	/* 配置CONVST GPIO */
	GPIO_InitStructure.GPIO_Pin 	= AD_CONVST_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;
	GPIO_Init(AD_CONVST_GPIO_PORT, &GPIO_InitStructure);

	/* 配置RANGE GPIO */
	GPIO_InitStructure.GPIO_Pin 	= AD_RANGE_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;
	GPIO_Init(AD_RANGE_GPIO_PORT, &GPIO_InitStructure);
	
	/* 配置OS0-2 GPIO */
	GPIO_InitStructure.GPIO_Pin = AD_OS0_PIN;
	GPIO_Init(AD_OS0_GPIO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = AD_OS1_PIN;
	GPIO_Init(AD_OS1_GPIO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = AD_OS2_PIN;
	GPIO_Init(AD_OS2_GPIO_PORT, &GPIO_InitStructure);

	/* 设置过采样模式 */
	AD7606_SetOS(0);

	/* 设置GPIO的初始状态 , 硬件复位复AD7606*/
	AD7606_Reset();
	
	/*CONVST脚设置为高电平 */
	AD_CONVST_HIGH();

	/*配置TIM2定时中断 */
	bsp_TIM4_Configuration();
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_Reset
*	功能说明: 硬件复位AD7606
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_Reset(void)
{
	/* AD7606是高电平复位，要求最小脉宽50ns */
	
	AD_RESET_LOW();
	
	AD_RESET_HIGH();
	AD_RESET_HIGH();
	AD_RESET_HIGH();
	AD_RESET_HIGH();
	
	AD_RESET_LOW();
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_SetOS
*	功能说明: 设置过采样模式（数字滤波，硬件求平均值)
*	形    参：_ucMode : 0-6  0表示无过采样，1表示2倍，2表示4倍，3表示8倍，4表示16倍
*				5表示32倍，6表示64倍
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_SetOS(uint8_t _ucMode)
{
	if (_ucMode == 1)
	{
		AD_OS2_0();
		AD_OS1_0();
		AD_OS0_1();
	}
	else if (_ucMode == 2)
	{
		AD_OS2_0();
		AD_OS1_1();
		AD_OS0_0();
	}
	else if (_ucMode == 3)
	{
		AD_OS2_0();
		AD_OS1_1();
		AD_OS0_1();
	}
	else if (_ucMode == 4)
	{
		AD_OS2_1();
		AD_OS1_0();
		AD_OS0_0();
	}
	else if (_ucMode == 5)
	{
		AD_OS2_1();
		AD_OS1_0();
		AD_OS0_1();
	}
	else if (_ucMode == 6)
	{
		AD_OS2_1();
		AD_OS1_1();
		AD_OS0_0();
	}
	else	/* 按0处理 */
	{
		AD_OS2_0();
		AD_OS1_0();
		AD_OS0_0();
	}
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_StartConv
*	功能说明: 启动AD7606的ADC转换
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_StartConv(void)
{
	/* 上升沿开始转换，低电平持续时间至少25ns  */
	AD_CONVST_LOW();
	AD_CONVST_LOW();
	AD_CONVST_LOW();	/* 连续执行2次，低电平约50ns */

	AD_CONVST_LOW();
	AD_CONVST_LOW();
	AD_CONVST_LOW();

	AD_CONVST_LOW();
	AD_CONVST_LOW();
	AD_CONVST_LOW();
	
	AD_CONVST_HIGH();
}

/*
*********************************************************************************************************
*	函 数 名: bsp_TIM4_Configuration
*	功能说明: 配置TIM4定时器
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_TIM4_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* TIM4 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);  

	/* Enable the TIM2 gloabal Interrupt [允许TIM2全局中断]*/
	NVIC_InitStructure.NVIC_IRQChannel 									 = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority				 = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd 							 = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
	
/*
*********************************************************************************************************
*	函 数 名: bsp_SET_TIM4_FREQ
*	功能说明: 设置TIM4定时器频率
*	形    参：_ulFreq : 采样频率，单位Hz，
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_SET_TIM4_FREQ(uint32_t _ulFreq)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	uint16_t usPrescaler;
	uint16_t usPeriod;

	TIM_DeInit(TIM4);	/* 复位TIM定时器 */
	if (_ulFreq == 0)
	{
		return;		/* 采样频率为0，停止采样 */
	}

	else if (_ulFreq <= 100)   /* 采样频率小于100Hz */
	{
		usPrescaler = 36000;		/* TM2CLK = 72 000 000/36000 = 2000 */
		usPeriod    = 2000 / _ulFreq;
	}
	else if (_ulFreq <= 200000)	/* 采样频率 ：100Hz - 200kHz */
	{
		usPrescaler = 36 - 1;		  /* TM2CLK = 36 000 000/36 = 2 000 000 */
		usPeriod    = 2000000 / _ulFreq;
	}	
	else	/* 采样频率大于 200kHz */
	{
		return;
	}
	
	TIM_TimeBaseStructure.TIM_Period			  = usPeriod - 1; 		/* 计数周期 */
	TIM_TimeBaseStructure.TIM_Prescaler 		= usPrescaler;	/* 分频系数 */
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0; 		/* */
	TIM_TimeBaseStructure.TIM_CounterMode 	= TIM_CounterMode_Up;  //计数方向向上计数
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	/* Clear TIM2 update pending flag[清除TIM2溢出中断标志] */
	TIM_ClearFlag(TIM4, TIM_FLAG_Update);

	TIM_SetCounter(TIM4, 0);

	/* Enable TIM2 Update interrupt [TIM2溢出中断允许]*/
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);  

	/* TIM2 enable counter [允许tim2计数]*/
	TIM_Cmd(TIM4, ENABLE);
}  

/*********************************************************************************************************
*	函 数 名: ReadSingleChannel
*	功能说明: 读取AD7606的采样结果
*	形    参：
*	返 回 值: 无
*********************************************************************************************************
*/
uint16_t AD7606_ReadSingleChannel(void)
{
  uint16_t usData = 0;

	// Wait until the transmit buffer is empty
	while (SPI_I2S_GetFlagStatus(AD_SPI, SPI_I2S_FLAG_TXE) == RESET);
	// Send the byte
	SPI_I2S_SendData(AD_SPI, 0xFFFF);

	/* Wait until a data is received */
	while (SPI_I2S_GetFlagStatus(AD_SPI, SPI_I2S_FLAG_RXNE) == RESET);

  /* Get the received data */
	usData = SPI_I2S_ReceiveData(AD_SPI);

  /* Return the shifted data */
  return usData;
}

/*
*********************************************************************************************************
*	函 数 名: uint16_t AD7606_ReadChannels
*	功能说明: first pull down CS for 35 ns, after reading, pull up CS [refer to AD7606 data sheet]
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_ReadChannels( void)
{
	uint8_t  i;
	uint16_t usReadValue = 1;

	//AD_CS_LOW();

	for( int i=0; i<ChannNum; i++ )
	{
		g_tAD.usBuf[i] = 0;
		usReadValue = AD7606_ReadSingleChannel();
		{
			g_tAD.usBuf[i] = usReadValue;
		}
	}


	//AD_CS_HIGH();
	AD7606_StartConv();

}

/*
*********************************************************************************************************
*	函 数 名: AD7606_IRQSrc
*	功能说明: 定时调用本函数，用于读取AD转换器数据
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_IRQSrc(void)
{
	uint8_t i;
	uint16_t usReadValue;

	TIM_ClearFlag(TIM4, TIM_FLAG_Update);

	/* 读取数据 ,示波器监测，CS低电平持续时间 35us 	*/
	AD7606_ReadChannels( );
}

//*********************************************************************************************************
//*********************************************************************************************************
uint16_t AD7606_GetReading  ( uint8_t ChanID )
{
	if( ChanID < ChannNum )
	{
		return g_tAD.usBuf[ChanID];
	}
}


/*
*********************************************************************************************************
*	函 数 名: AD7606_StartRecord
*	功能说明: 开始采集
*	形    参：_ulFreq : 采样频率, 单位 HZ
*	返 回 值: 无
*********************************************************************************************************
*/
RETVAL AD7606_StartRecord(uint32_t TIM4_Freq )
{

	if( TIM4_Freq > 0 && TIM4_Freq <= 200 )
	{
		//AD7606_Reset();
		AD7606_StartConv();

		g_tAD.usRead  = 0;
		g_tAD.usWrite = 0;

		bsp_TIM4_Configuration (  );
		bsp_SET_TIM4_FREQ      (TIM4_Freq);
		return RET_NOERR;
	}

	return RET_ERROR;
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_StopRecord
*	功能说明: 停止采集
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_StopRecord(void)
{
	TIM_Cmd(TIM4, DISABLE);
}

void TIM4_IRQHandler(void)
{
	AD7606_IRQSrc();
}

