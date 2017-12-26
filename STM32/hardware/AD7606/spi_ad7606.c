/*
*********************************************************************************************************
*	                                  
*	ģ������ : AD7606����ģ��
*	�ļ����� : spi_AD7606.c
*	��    �� :   v1.0
*	˵    �� :   ����AD7606 ADCת���� SPI�ӿ�
*
*
*********************************************************************************************************
*/
#include "stm32f10x.h"
#include <stdio.h>
#include "spi_ad7606.h"

//*******************************************************************************************************
#define ChannNum 8 					    /* �ɼ�ͨ�� */
//#define FIFO_SIZE	1*1024*2		/* ��С��Ҫ����48K (CPU�ڲ�RAM ֻ��64K) */

typedef struct
{
	uint16_t usRead;
	uint16_t usWrite;
	uint16_t usCount;
	uint16_t usBuf[ChannNum];
}FIFO_T;

FIFO_T	g_tAD;	/* ����һ�����������������ڴ洢AD�ɼ����ݣ�������д��SD�� */

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
*	�� �� ��: bsp_InitAD7606
*	����˵��: ��ʼ��AD7606 SPI����
*	��    �Σ���
*	�� �� ֵ: ��
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

	/* ʹ��GPIOʱ�� */
	RCC_APB2PeriphClockCmd(AD_RESET_GPIO_CLK | AD_CONVST_GPIO_CLK | AD_RANGE_GPIO_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(AD_OS0_GPIO_CLK   | AD_OS1_GPIO_CLK    | AD_OS2_GPIO_CLK,   ENABLE);

	/* ����RESET GPIO */
	GPIO_InitStructure.GPIO_Pin 	= AD_RESET_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;
	GPIO_Init(AD_RESET_GPIO_PORT, &GPIO_InitStructure);
	
	/* ����CONVST GPIO */
	GPIO_InitStructure.GPIO_Pin 	= AD_CONVST_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;
	GPIO_Init(AD_CONVST_GPIO_PORT, &GPIO_InitStructure);

	/* ����RANGE GPIO */
	GPIO_InitStructure.GPIO_Pin 	= AD_RANGE_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;
	GPIO_Init(AD_RANGE_GPIO_PORT, &GPIO_InitStructure);
	
	/* ����OS0-2 GPIO */
	GPIO_InitStructure.GPIO_Pin = AD_OS0_PIN;
	GPIO_Init(AD_OS0_GPIO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = AD_OS1_PIN;
	GPIO_Init(AD_OS1_GPIO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = AD_OS2_PIN;
	GPIO_Init(AD_OS2_GPIO_PORT, &GPIO_InitStructure);

	/* ���ù�����ģʽ */
	AD7606_SetOS(0);

	/* ����GPIO�ĳ�ʼ״̬ , Ӳ����λ��AD7606*/
	AD7606_Reset();
	
	/*CONVST������Ϊ�ߵ�ƽ */
	AD_CONVST_HIGH();

	/*����TIM2��ʱ�ж� */
	bsp_TIM4_Configuration();
}

/*
*********************************************************************************************************
*	�� �� ��: AD7606_Reset
*	����˵��: Ӳ����λAD7606
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void AD7606_Reset(void)
{
	/* AD7606�Ǹߵ�ƽ��λ��Ҫ����С����50ns */
	
	AD_RESET_LOW();
	
	AD_RESET_HIGH();
	AD_RESET_HIGH();
	AD_RESET_HIGH();
	AD_RESET_HIGH();
	
	AD_RESET_LOW();
}

/*
*********************************************************************************************************
*	�� �� ��: AD7606_SetOS
*	����˵��: ���ù�����ģʽ�������˲���Ӳ����ƽ��ֵ)
*	��    �Σ�_ucMode : 0-6  0��ʾ�޹�������1��ʾ2����2��ʾ4����3��ʾ8����4��ʾ16��
*				5��ʾ32����6��ʾ64��
*	�� �� ֵ: ��
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
	else	/* ��0���� */
	{
		AD_OS2_0();
		AD_OS1_0();
		AD_OS0_0();
	}
}

/*
*********************************************************************************************************
*	�� �� ��: AD7606_StartConv
*	����˵��: ����AD7606��ADCת��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void AD7606_StartConv(void)
{
	/* �����ؿ�ʼת�����͵�ƽ����ʱ������25ns  */
	AD_CONVST_LOW();
	AD_CONVST_LOW();
	AD_CONVST_LOW();	/* ����ִ��2�Σ��͵�ƽԼ50ns */

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
*	�� �� ��: bsp_TIM4_Configuration
*	����˵��: ����TIM4��ʱ��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_TIM4_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* TIM4 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);  

	/* Enable the TIM2 gloabal Interrupt [����TIM2ȫ���ж�]*/
	NVIC_InitStructure.NVIC_IRQChannel 									 = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority				 = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd 							 = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
	
/*
*********************************************************************************************************
*	�� �� ��: bsp_SET_TIM4_FREQ
*	����˵��: ����TIM4��ʱ��Ƶ��
*	��    �Σ�_ulFreq : ����Ƶ�ʣ���λHz��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_SET_TIM4_FREQ(uint32_t _ulFreq)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	uint16_t usPrescaler;
	uint16_t usPeriod;

	TIM_DeInit(TIM4);	/* ��λTIM��ʱ�� */
	if (_ulFreq == 0)
	{
		return;		/* ����Ƶ��Ϊ0��ֹͣ���� */
	}

	else if (_ulFreq <= 100)   /* ����Ƶ��С��100Hz */
	{
		usPrescaler = 36000;		/* TM2CLK = 72 000 000/36000 = 2000 */
		usPeriod    = 2000 / _ulFreq;
	}
	else if (_ulFreq <= 200000)	/* ����Ƶ�� ��100Hz - 200kHz */
	{
		usPrescaler = 36 - 1;		  /* TM2CLK = 36 000 000/36 = 2 000 000 */
		usPeriod    = 2000000 / _ulFreq;
	}	
	else	/* ����Ƶ�ʴ��� 200kHz */
	{
		return;
	}
	
	TIM_TimeBaseStructure.TIM_Period			  = usPeriod - 1; 		/* �������� */
	TIM_TimeBaseStructure.TIM_Prescaler 		= usPrescaler;	/* ��Ƶϵ�� */
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0; 		/* */
	TIM_TimeBaseStructure.TIM_CounterMode 	= TIM_CounterMode_Up;  //�����������ϼ���
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	/* Clear TIM2 update pending flag[���TIM2����жϱ�־] */
	TIM_ClearFlag(TIM4, TIM_FLAG_Update);

	TIM_SetCounter(TIM4, 0);

	/* Enable TIM2 Update interrupt [TIM2����ж�����]*/
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);  

	/* TIM2 enable counter [����tim2����]*/
	TIM_Cmd(TIM4, ENABLE);
}  

/*********************************************************************************************************
*	�� �� ��: ReadSingleChannel
*	����˵��: ��ȡAD7606�Ĳ������
*	��    �Σ�
*	�� �� ֵ: ��
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
*	�� �� ��: uint16_t AD7606_ReadChannels
*	����˵��: first pull down CS for 35 ns, after reading, pull up CS [refer to AD7606 data sheet]
*	��    �Σ���
*	�� �� ֵ: ��
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
*	�� �� ��: AD7606_IRQSrc
*	����˵��: ��ʱ���ñ����������ڶ�ȡADת��������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void AD7606_IRQSrc(void)
{
	uint8_t i;
	uint16_t usReadValue;

	TIM_ClearFlag(TIM4, TIM_FLAG_Update);

	/* ��ȡ���� ,ʾ������⣬CS�͵�ƽ����ʱ�� 35us 	*/
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
*	�� �� ��: AD7606_StartRecord
*	����˵��: ��ʼ�ɼ�
*	��    �Σ�_ulFreq : ����Ƶ��, ��λ HZ
*	�� �� ֵ: ��
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
*	�� �� ��: AD7606_StopRecord
*	����˵��: ֹͣ�ɼ�
*	��    �Σ���
*	�� �� ֵ: ��
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

