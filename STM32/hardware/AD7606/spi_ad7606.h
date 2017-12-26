/*
*********************************************************************************************************
*	                                  
*	模块名称 : AD7606驱动模块 
*	文件名称 : bsp_ad7606.h
*	版    本 : V1.3
*	说    明 : 头文件
*********************************************************************************************************
*/

#include "stdint.h"
#include "sys.h"

#ifndef __BSP_AD7606_H
#define __BSP_AD7606_H

/* 开关全局中断的宏 */
#ifndef ENABLE_INT
#define ENABLE_INT ()	__set_PRIMASK(0)	/* 使能全局中断 */
#define DISABLE_INT()	__set_PRIMASK(1)	/* 禁止全局中断 */
#endif

/* 定义AD7606的SPI GPIO */

#define AD_SPI													 SPI1
/*　***************** */
#define AD_CS_PIN                        GPIO_Pin_4
#define AD_CS_GPIO_PORT                  GPIOA
#define AD_CS_GPIO_CLK                   RCC_APB2Periph_GPIOA

#define AD_SPI_SCK_PIN                   GPIO_Pin_5
#define AD_SPI_SCK_GPIO_PORT             GPIOA
#define AD_SPI_SCK_GPIO_CLK              RCC_APB2Periph_GPIOA

#define AD_SPI_MISO_PIN                  GPIO_Pin_6
#define AD_SPI_MISO_GPIO_PORT            GPIOA
#define AD_SPI_MISO_GPIO_CLK             RCC_APB2Periph_GPIOA

/*　***************** */
#define AD_RESET_PIN                     GPIO_Pin_7//GPIO_Pin_7
#define AD_RESET_GPIO_PORT               GPIOB //GPIOA
#define AD_RESET_GPIO_CLK                RCC_APB2Periph_GPIOB

#define AD_RANGE_PIN                     GPIO_Pin_6//GPIO_Pin_8
#define AD_RANGE_GPIO_PORT               GPIOB //GPIOA
#define AD_RANGE_GPIO_CLK                RCC_APB2Periph_GPIOB

#define AD_CONVST_PIN                    GPIO_Pin_0
#define AD_CONVST_GPIO_PORT              GPIOB
#define AD_CONVST_GPIO_CLK               RCC_APB2Periph_GPIOB

/*　***************** */
#define AD_OS0_PIN                     	 GPIO_Pin_15
#define AD_OS0_GPIO_PORT                 GPIOD
#define AD_OS0_GPIO_CLK                  RCC_APB2Periph_GPIOD

#define AD_OS1_PIN                       GPIO_Pin_14
#define AD_OS1_GPIO_PORT                 GPIOD
#define AD_OS1_GPIO_CLK                  RCC_APB2Periph_GPIOD

#define AD_OS2_PIN                       GPIO_Pin_13
#define AD_OS2_GPIO_PORT                 GPIOD
#define AD_OS2_GPIO_CLK                  RCC_APB2Periph_GPIOD


#define AD_CS_LOW()     				         AD_CS_GPIO_PORT->BRR = AD_CS_PIN
#define AD_CS_HIGH()     				         AD_CS_GPIO_PORT->BSRR = AD_CS_PIN

#define AD_RESET_LOW()									 AD_RESET_GPIO_PORT->BRR = AD_RESET_PIN
#define AD_RESET_HIGH()									 AD_RESET_GPIO_PORT->BSRR = AD_RESET_PIN
	
#define AD_CONVST_LOW()									 AD_CONVST_GPIO_PORT->BRR = AD_CONVST_PIN
#define AD_CONVST_HIGH()								 AD_CONVST_GPIO_PORT->BSRR = AD_CONVST_PIN

#define AD_RANGE_5V()					 					 AD_RANGE_GPIO_PORT->BRR = AD_RANGE_PIN
#define AD_RANGE_10V()									 AD_RANGE_GPIO_PORT->BSRR = AD_RANGE_PIN

#define AD_OS0_0()											 AD_OS0_GPIO_PORT->BRR = AD_OS0_PIN
#define AD_OS0_1()										   AD_OS0_GPIO_PORT->BSRR = AD_OS0_PIN

#define AD_OS1_0()										   AD_OS1_GPIO_PORT->BRR = AD_OS1_PIN
#define AD_OS1_1()											 AD_OS1_GPIO_PORT->BSRR = AD_OS1_PIN

#define AD_OS2_0()						           AD_OS2_GPIO_PORT->BRR = AD_OS2_PIN
#define AD_OS2_1()						           AD_OS2_GPIO_PORT->BSRR = AD_OS2_PIN

#define AD_MISO_LOW()									   AD_SPI_MISO_GPIO_PORT->BRR  = AD_SPI_MISO_PIN
#define AD_MISO_HIGH()									 AD_SPI_MISO_GPIO_PORT->BSRR = AD_SPI_MISO_PIN

#define AD_SCK_LOW()										 AD_SPI_SCK_GPIO_PORT->BRR  = AD_SPI_SCK_PIN
#define AD_CSK_HIGH()										 AD_SPI_SCK_GPIO_PORT->BSRR = AD_SPI_SCK_PIN

#define AD_MISO_IN											 PAin(6)

/* 供外部调用的函数声明 */
void     Init_SPI_AD7606		(void);
void     AD7606_Reset			  (void);
void     AD7606_SetOS			  (uint8_t  _ucMode);
RETVAL   AD7606_StartRecord (uint32_t TIM4_Freq);
uint16_t AD7606_GetReading  (uint8_t  ChanID);
void     AD7606_StopRecord  (void);

#endif/*End define __BSP_AD7606_H */


