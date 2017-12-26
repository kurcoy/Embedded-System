#ifndef __task_nor_flash__
#define __task_nor_flash__

#include "multi-task.h"

#define TASK_NOR_FLASH_STACK_SIZE	256
#define TASK_NOR_FLASH_PERIOD	1000 /* 1000ms*/

#define NOR_FLASH_RESET				GPIO_Pin_11 /* PI11 MCU_NOR_FLASH_RESET_N */
#define NOR_FLASH_RESET_GROUP		GPIOI /* PI11 MCU_NOR_FLASH_RESET_N */

#define NOR_FLASH_WP			 	GPIO_Pin_11 /* PF11 MCU_NOR_FLASH_WP */
#define NOR_FLASH_WP_GROUP			GPIOF /* PF11 MCU_NOR_FLASH_WP */

#define NOR_FLASH_BASE_ADDRESS	0x60000000

#define __IO	volatile
#define NOR_BANK_ADDR        ((uint32_t)0x60000000)
#define ADDR_SHIFT(Address) (NOR_BANK_ADDR + (4 * (Address)))
#define NOR_WRITE(Address, Data)  (*(__IO uint16_t *)(Address) = (Data))


#define sFLASH_SPI SPI1
#define sFLASH_CS_LOW()				GPIO_ResetBits(GPIOA, GPIO_Pin_15)
#define sFLASH_CS_HIGH()			GPIO_SetBits(GPIOA, GPIO_Pin_15)

/* M25P SPI Flash supported commands */
#define sFLASH_CMD_WRITE          0x02  /* Write to Memory instruction */
#define sFLASH_CMD_WRSR           0x01  /* Write Status Register instruction */
#define sFLASH_CMD_WREN           0x06  /* Write enable instruction */
#define sFLASH_CMD_READ           0x03  /* Read from Memory instruction */
#define sFLASH_CMD_RDSR           0x05  /* Read Status Register instruction  */
#define sFLASH_CMD_RDID           0x9F  /* Read identification */
#define sFLASH_CMD_SE             0x20  /* Sector Erase instruction */
#define sFLASH_CMD_BE             0xC7  /* Bulk Erase instruction */

#define sFLASH_WIP_FLAG           0x01  /* Write In Progress (WIP) flag */
#define sFLASH_DUMMY_BYTE         0xA5
#define sFLASH_SPI_PAGESIZE       (0x100) /* 256 bytes */
#define sFLASH_SPI_SECTORSIZE     (4096)  /* 4K bytes */
#define sFLASH_SPI_BLOCKSIZE      (16*sFLASH_SPI_SECTORSIZE)  /* 64K bytes */
#define sFLASH_SPI_CHIPSIZE       (8*sFLASH_SPI_BLOCKSIZE)  /* 512K bytes */

typedef struct
{
	uint16_t manufatureID;/*0001h*/
	uint16_t deviceID1;/*227Eh*/
	uint16_t protection;
	uint16_t indication;
	uint16_t rfu[8];
	uint16_t lowerSWbits;
	uint16_t upperSWbits;
	uint16_t deviceID2;/*2228h = 1 Gb 2223h = 512 Mb 2222h = 256 Mb 2221h = 128 Mb */
	uint16_t deviceID3;/*2201h*/
}NorFlashSectorID_t;

typedef struct
{
   uint16_t year;	
   uint8_t month;	
   uint8_t date;	
   uint8_t hour;	
   uint8_t minute;
   uint8_t second;	
}AlarmTime;

typedef struct
{
   uint8_t alarm_level;
   AlarmTime time;
   uint8_t alarm_kind;
   uint32_t alarm_content;
}AlarmMessage;
typedef enum
{
	eSPIFLASH_Request_Idle = 0,
	eSPIFLASH_Request_Erase,
	eSPIFLASH_Request_ReadID,
	eSPIFLASH_Request_Test
}eSPIFLASH_Request_t;

extern StackType_t TaskNorFlashStack[TASK_NOR_FLASH_STACK_SIZE] CCM_RAM;  // Put task stack in CCM
extern StaticTask_t TaskNorFlashBuffer CCM_RAM;  // Put TCB in CCM

void Task_NorFlash(void* p);
void eSPIFLASH_Request(eSPIFLASH_Request_t req);

#endif/*__task_nor_flash__*/
