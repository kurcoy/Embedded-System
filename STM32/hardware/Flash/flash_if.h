/**
  ******************************************************************************
  * @file    flash_if.h 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    31-October-2011
  * @brief   Header for flash_if.c module
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASH_IF_H
#define __FLASH_IF_H

#define BOOT_FLASH_START_ADDRESS          0x08000000
#define USER_FLASH_FIRST_PAGE_ADDRESS 0x08040000 /* Only as example see comment */
#define USER_FLASH_LAST_PAGE_ADDRESS  0x080E0000
#define USER_FLASH_END_ADDRESS        0x080FFFFF

#define FLASH_SECTOR0_START_ADD		0x08000000
#define FLASH_SECTOR1_START_ADD		0x08004000
#define FLASH_SECTOR2_START_ADD		0x08008000
#define FLASH_SECTOR3_START_ADD		0x0800c000
#define FLASH_SECTOR4_START_ADD		0x08010000
#define FLASH_SECTOR5_START_ADD		0x08020000
#define FLASH_SECTOR6_START_ADD		0x08040000
#define FLASH_SECTOR7_START_ADD		0x08060000
#define FLASH_SECTOR8_START_ADD		0x08080000
#define FLASH_SECTOR9_START_ADD		0x080a0000
#define FLASH_SECTOR10_START_ADD	0x080c0000
#define FLASH_SECTOR11_START_ADD	0x080e0000

/* Includes ------------------------------------------------------------------*/
//temporary disable
//#include "main.h"
#include "core_cm3.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define USER_FLASH_SIZE   (USER_FLASH_END_ADDRESS - USER_FLASH_FIRST_PAGE_ADDRESS)

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
uint32_t FLASH_If_Write(__IO uint32_t* Address, uint32_t* Data, uint16_t DataLength);
int8_t FLASH_If_Erase(uint32_t StartSector);
void FLASH_If_Init(void);

#endif /* __FLASH_IF_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
