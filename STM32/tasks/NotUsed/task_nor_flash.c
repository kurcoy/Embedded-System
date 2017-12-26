/**
 * This file is the driver for NOR Flash
 *
 * */

#include "stm32f4xx.h"
#include "multi-task.h"
#include "FreeRTOS.h"
#include "task_nor_flash.h"
#include "debug.h"

StackType_t TaskNorFlashStack[TASK_NOR_FLASH_STACK_SIZE] CCM_RAM;  // Put task stack in CCM
StaticTask_t TaskNorFlashBuffer CCM_RAM;  // Put TCB in CCM

static char NorFlashDebugBuffer[128];
static char eSPIFlashPageBuffer[sFLASH_SPI_PAGESIZE];
static eSPIFLASH_Request_t SPI_Request;

static NorFlashSectorID_t sectorId;

/*******************************************************************************
 * @name: NorFlashReset
 * @input: reset 1 -> reset the NOR flash
 * 				 0 -> release the NOR flash
 * @output:
 * @role reset chip
*******************************************************************************/
void NorFlashReset(int reset)
{
	if (reset)
	{
		GPIO_ResetBits(NOR_FLASH_RESET_GROUP, NOR_FLASH_RESET); /* MCU_NOR_FLASH_RESET_N */

	}
	else
	{
		GPIO_SetBits(NOR_FLASH_RESET_GROUP, NOR_FLASH_RESET); /* MCU_NOR_FLASH_RESET_N */
	}
}

/*******************************************************************************
 * @name: NorFlashWp
 * @input: reset 1 -> enable NOR flash write protection
 * 				 0 -> disable NOR flash write protection
 * @output:
 * @role enable/disable the chip
*******************************************************************************/
void NorFlashWp(int wp)
{
	if(wp)
	{
		GPIO_ResetBits(NOR_FLASH_WP_GROUP, NOR_FLASH_WP); /* MCU_NOR_FLASH_WP */
	}
	else
	{
		GPIO_SetBits(NOR_FLASH_WP_GROUP, NOR_FLASH_WP); /* MCU_NOR_FLASH_WP */
	}
}
/*******************************************************************************
 * @name: NorFlashEraseChip
 * @input: void
 * @output:
 * @role erase the whole chip
 * erase command:
 * cycle | addr | data | addr | data | addr | data | addr | data | addr | data | addr | data|
 *   6   |  555 |   AA |  2AA |  55  | 555  |  80  |  555 |  AA  | 2AA  |  55  |  555 | 10  |
*******************************************************************************/
int NorFlashEraseChip(void)
{
	int ret = 0;
	uint16_t *noraddr555, *noraddr2aa;

	noraddr555 = (uint16_t *)(NOR_FLASH_BASE_ADDRESS + (0x555<<1));
	noraddr2aa = (uint16_t *)(NOR_FLASH_BASE_ADDRESS + (0x2aa<<1));

	*noraddr555 = 0xaa;
	*noraddr2aa = 0x55;

	*noraddr555 = 0x80;
	*noraddr555 = 0xaa;

	*noraddr2aa = 0x55;
	*noraddr555 = 0x10;

	return ret;
}

/*******************************************************************************
 * @name: NorFlashEraseSector
 * @input: sector address, the 16 LSB must be 0
 * @output:
 * @role erase the specified sector
 * erase command:
 * cycle | addr | data | addr | data | addr | data | addr | data | addr | data | addr | data|
 *   6   |  555 |   AA |  2AA |  55  | 555  |  80  |  555 |  AA  | 2AA  |  55  |  SA  | 30  |
*******************************************************************************/
int NorFlashEraseSector(int sec)
{
	int ret = 0;
	uint16_t *noraddr555, *noraddr2aa;

	sec &= ~0xffff;/* 16 LSB should be 0 */

	noraddr555 = (uint16_t *)(NOR_FLASH_BASE_ADDRESS + (0x555<<1));
	noraddr2aa = (uint16_t *)(NOR_FLASH_BASE_ADDRESS + (0x2aa<<1));

	*noraddr555 = 0xaa;
	*noraddr2aa = 0x55;

	*noraddr555 = 0x80;
	*noraddr555 = 0xaa;

	*noraddr2aa = sec;
	*noraddr555 = 0x30;

	return ret;
}

/*******************************************************************************
 * @name: NorFlashProgramWord
 * @input: addr - Word address,
 *         data - data to write
 * @output:
 * @role write data to the address
 * command:
 * cycle | addr | data | addr | data | addr | data | addr | data |
 *   4   |  555 |  AA  | 2AA  |  55  | 555  | A0   |  PA  | PD   |
*******************************************************************************/
int NorFlashProgramWord(uint32_t addr, uint16_t data)
{
	int ret = 0;
	uint16_t *noraddr555, *noraddr2aa, *pa;

	pa = (uint16_t *)(addr & (~0x1));/* two bytes align */

	noraddr555 = (uint16_t *)(NOR_FLASH_BASE_ADDRESS + (0x555<<1));
	noraddr2aa = (uint16_t *)(NOR_FLASH_BASE_ADDRESS + (0x2aa<<1));

	*noraddr555 = 0xaa;
	*noraddr2aa = 0x55;

	*noraddr555 = 0xa0;
	*pa = data;

	return ret;
}

int NorFlashBlankCheck(void)
{
	int ret = 0;

	return ret;
}


/*******************************************************************************
 * @name: NorFlashStatusRead
 * @input: void
 * @output:
 * @role read flash status
 * command:
 * cycle | addr | data | addr | data |
 *    2  | 555  |  70  | XXX  | RD   |
*******************************************************************************/
int NorFlashStatusRead(void)
{
	int ret = 0;
	uint16_t *noraddr555, *noraddr2aa;

	noraddr555 = (uint16_t *)(NOR_FLASH_BASE_ADDRESS + (0x555<<1));
	noraddr2aa = (uint16_t *)(NOR_FLASH_BASE_ADDRESS + (0x2aa<<1));

	*noraddr555 = 0x70;

	ret = *noraddr2aa & 0xffff;

	return ret;
}

/*******************************************************************************
 * @name: NorFlashStatusClear
 * @input: void
 * @output:
 * @role clear flash status register
 * command:
 * cycle | addr | data |
 *    1  | 555  | 71   |
*******************************************************************************/
int NorFlashStatusClear(void)
{
	int ret = 0;
	uint16_t *noraddr555;

	noraddr555 = (uint16_t *)(NOR_FLASH_BASE_ADDRESS + (0x555<<1));

	*noraddr555 = 0x71;

	return ret;
}

/*******************************************************************************
 * @name: NorIdCfiAsoEntry
 * @input: void
 * @output:
 * @role: entry the ID CFI ASO
 * command:
 * cycle | addr | data | addr | data | addr   | data |
 *    3  | 555  |  AA  |  2AA |   55 |(SA)555 | 90   |
*******************************************************************************/
int NorIdCfiAsoEntry(void)
{
	int ret = 0;
	uint16_t *noraddr555, *noraddr2aa;

	noraddr555 = (uint16_t *)((uint32_t)NOR_FLASH_BASE_ADDRESS + (uint32_t)(0x555<<1));
	noraddr2aa = (uint16_t *)((uint32_t)NOR_FLASH_BASE_ADDRESS + (uint32_t)(0x2aa<<1));

	*noraddr555 = (uint16_t)0xaa;
	*noraddr2aa = (uint16_t)0x55;
	*noraddr555 = (uint16_t)0x90;

	return ret;
}

/*******************************************************************************
 * @name: NorAsoExit
 * @input: void
 * @output:
 * @role: exit CFI ASO
 * command:
 * cycle | addr | data |
 *    1  | XXX  |  F0  |
*******************************************************************************/
int NorAsoExit(void)
{
	int ret = 0;
	uint16_t *noraddr555;

	noraddr555 = (uint16_t *)((uint32_t)NOR_FLASH_BASE_ADDRESS + (uint32_t)(0x555<<1));

	*noraddr555 = (uint16_t)0xf0;

	return ret;
}

/*******************************************************************************
 * @name: NorIdCfiAsoExit
 * @input: void
 * @output:
 * @role: exit the ID CFI ASO
 * command:
 * cycle | addr | data |
 *    1  | XXX  |  F0  |
*******************************************************************************/
int NorIdCfiAsoExit(void)
{
	int ret = 0;

	ret = NorAsoExit();

	return ret;
}

/*******************************************************************************
 * @name: NorSecSiliRegAsoEntry
 * @input: void
 * @output:
 * @role: entry the ID Secure Silicon Region (SSR) ASO
 * command:
 * cycle | addr | data | addr | data | addr   | data |
 *    3  | 555  |  AA  |  2AA |   55 |(SA)555 | 88   |
*******************************************************************************/
int NorSecSiliRegAsoEntry(void)
{
	int ret = 0;
	uint16_t *noraddr555, *noraddr2aa;

	noraddr555 = (uint16_t *)(NOR_FLASH_BASE_ADDRESS + (0x555<<1));
	noraddr2aa = (uint16_t *)(NOR_FLASH_BASE_ADDRESS + (0x2aa<<1));

	*noraddr555 = 0xaa;
	*noraddr2aa = 0x55;
	*noraddr555 = 0x88;
	return ret;
}

/*******************************************************************************
 * @name: NorSecSiliRegAsoExit
 * @input: void
 * @output:
 * @role: exit the ID Secure Silicon Region (SSR) ASO
 * command:
 * cycle | addr | data |
 *    1  | XXX  |  F0  |
*******************************************************************************/
int NorSecSiliRegAsoExit(void)
{
	int ret = 0;

	ret = NorAsoExit();

	return ret;
}

/*******************************************************************************
 * @name: NorLockRegAsoEntry
 * @input: void
 * @output:
 * @role: entry Lock Register ASO
 * command:
 * cycle | addr | data | addr | data | addr   | data |
 *    3  | 555  |  AA  |  2AA |   55 |(SA)555 | 40   |
*******************************************************************************/
int NorLockRegAsoEntry(void)
{
	int ret = 0;
	uint16_t *noraddr555, *noraddr2aa;

	noraddr555 = (uint16_t *)(NOR_FLASH_BASE_ADDRESS + (0x555<<1));
	noraddr2aa = (uint16_t *)(NOR_FLASH_BASE_ADDRESS + (0x2aa<<1));

	*noraddr555 = 0xaa;
	*noraddr2aa = 0x55;
	*noraddr555 = 0x40;

	return ret;
}

/*******************************************************************************
 * @name: NorLockRegAsoExit
 * @input: void
 * @output:
 * @role: exit Lock Register ASO
 * command:
 * cycle | addr | data |
 *    1  | XXX  |  F0  |
*******************************************************************************/
int NorLockRegAsoExit(void)
{
	int ret = 0;

	ret = NorAsoExit();

	return ret;
}

/*******************************************************************************
 * @name: NorPasswordAsoEntry
 * @input: void
 * @output:
 * @role: entry Password ASO
 * command:
 * cycle | addr | data | addr | data | addr   | data |
 *    3  | 555  |  AA  |  2AA |   55 |(SA)555 | 40   |
*******************************************************************************/
int NorPasswordAsoEntry(void)
{
	int ret = 0;
	uint16_t *noraddr555, *noraddr2aa;

	noraddr555 = (uint16_t *)(NOR_FLASH_BASE_ADDRESS + (0x555<<1));
	noraddr2aa = (uint16_t *)(NOR_FLASH_BASE_ADDRESS + (0x2aa<<1));

	*noraddr555 = 0xaa;
	*noraddr2aa = 0x55;
	*noraddr555 = 0x60;

	return ret;
}

/*******************************************************************************
 * @name: NorPasswordAsoExit
 * @input: void
 * @output:
 * @role: exit Password ASO
 * command:
 * cycle | addr | data |
 *    1  | XXX  |  F0  |
*******************************************************************************/
int NorPasswordAsoExit(void)
{
	int ret = 0;

	ret = NorAsoExit();

	return ret;
}

/*******************************************************************************
 * @name: NorPPBAsoEntry
 * @input: void
 * @output:
 * @role: entry PPB (Non-Volatile Sector Protection) ASO
 * command:
 * cycle | addr | data | addr | data | addr   | data |
 *    3  | 555  |  AA  |  2AA |   55 |(SA)555 | C0   |
*******************************************************************************/
int NorPPBAsoEntry(void)
{
	int ret = 0;
	uint16_t *noraddr555, *noraddr2aa;

	noraddr555 = (uint16_t *)(NOR_FLASH_BASE_ADDRESS + (0x555<<1));
	noraddr2aa = (uint16_t *)(NOR_FLASH_BASE_ADDRESS + (0x2aa<<1));

	*noraddr555 = 0xaa;
	*noraddr2aa = 0x55;
	*noraddr555 = 0x60;
	return ret;
}

/*******************************************************************************
 * @name: NorPPBAsoExit
 * @input: void
 * @output:
 * @role: exit PPB (Non-Volatile Sector Protection) ASO
 * command:
 * cycle | addr | data |
 *    1  | XXX  |  F0  |
*******************************************************************************/
int NorPPBAsoExit(void)
{
	int ret = 0;

	ret = NorAsoExit();

	return ret;
}

/*******************************************************************************
 * @name: NorPPBLockAsoEntry
 * @input: void
 * @output:
 * @role: entry PPB Lock Bit ASO
 * command:
 * cycle | addr | data | addr | data | addr   | data |
 *    3  | 555  |  AA  |  2AA |   55 |(SA)555 | 50   |
*******************************************************************************/
int NorPPBLockAsoEntry(void)
{
	int ret = 0;
	uint16_t *noraddr555, *noraddr2aa;

	noraddr555 = (uint16_t *)(NOR_FLASH_BASE_ADDRESS + (0x555<<1));
	noraddr2aa = (uint16_t *)(NOR_FLASH_BASE_ADDRESS + (0x2aa<<1));

	*noraddr555 = 0xaa;
	*noraddr2aa = 0x55;
	*noraddr555 = 0x50;

	return ret;
}

/*******************************************************************************
 * @name: NorPPBLockAsoExit
 * @input: void
 * @output:
 * @role: exit PPB Lock Bit ASO
 * command:
 * cycle | addr | data |
 *    1  | XXX  |  F0  |
*******************************************************************************/
int NorPPBLockAsoExit(void)
{
	int ret = 0;

	ret = NorAsoExit();

	return ret;
}

/*******************************************************************************
 * @name: NorDYBAsoEntry
 * @input: void
 * @output:
 * @role: entry DYB (Volatile Sector Protection) ASO
 * command:
 * cycle | addr | data | addr | data | addr   | data |
 *    3  | 555  |  AA  |  2AA |   55 |(SA)555 | E0   |
*******************************************************************************/
int NorDYBAsoEntry(void)
{
	int ret = 0;
	uint16_t *noraddr555, *noraddr2aa;

	noraddr555 = (uint16_t *)(NOR_FLASH_BASE_ADDRESS + (0x555<<1));
	noraddr2aa = (uint16_t *)(NOR_FLASH_BASE_ADDRESS + (0x2aa<<1));

	*noraddr555 = 0xaa;
	*noraddr2aa = 0x55;
	*noraddr555 = 0xe0;

	return ret;
}

/*******************************************************************************
 * @name: NorDYBAsoExit
 * @input: void
 * @output:
 * @role: exit DYB (Volatile Sector Protection) ASO
 * command:
 * cycle | addr | data |
 *    1  | XXX  |  F0  |
*******************************************************************************/
int NorDYBAsoExit(void)
{
	int ret = 0;

	ret = NorAsoExit();

	return ret;
}

/*******************************************************************************
 * @name: NorFlashReadSectorId
 * @input: sa -> the sector address should be 128KB aligned
 * 		   id -> pointer to id struct
 * @output:
 * @role: read the sector information
 * 1. enter the Id-CFI ASO
 * 2. read the information then store in the *id struct
 * 3. exit the Id-CFI ASO
*******************************************************************************/
int NorFlashReadSectorId(uint32_t sa, NorFlashSectorID_t *id)
{
	int ret = 1;
	volatile uint16_t *p;

	if(id)
	{
		ret = 0;
//		p = (uint16_t *)(sa & (~0x1ffff));
		p = (uint16_t *)0x60000000;

		NorIdCfiAsoEntry();
		id->manufatureID = *p++;
		id->deviceID1    = *p++;
		id->protection   = *p++;
		id->indication   = *p++;
		id->rfu[0]       = *p++;
		id->rfu[1]       = *p++;
		id->rfu[2]       = *p++;
		id->rfu[3]       = *p++;
		id->rfu[4]       = *p++;
		id->rfu[5]       = *p++;
		id->rfu[6]       = *p++;
		id->rfu[7]       = *p++;
		id->lowerSWbits  = *p++;
		id->upperSWbits  = *p++;
		id->deviceID2    = *p++;
		id->deviceID3    = *p++;
		NorIdCfiAsoExit();

		p = (uint16_t)0x60000000;
		sprintf(NorFlashDebugBuffer, "NOR Manufacture ID: 0x%x device ID:0x%x\r\n", id->manufatureID, id->deviceID1);
		dbg_printf(NorFlashDebugBuffer);
		sprintf(NorFlashDebugBuffer, "ID2:0x%x ID3:0x%x\r\n", id->deviceID2, id->deviceID3);
		dbg_printf(NorFlashDebugBuffer);
	}

	return ret;
}

void NOR_ReadID(NorFlashSectorID_t* pNOR_ID)
{
	NOR_WRITE(ADDR_SHIFT(0x0555), 0x00AA);
	NOR_WRITE(ADDR_SHIFT(0x02AA), 0x0055);
	NOR_WRITE(ADDR_SHIFT(0x0555), 0x0090);

	pNOR_ID->manufatureID = *(__IO uint16_t *) ADDR_SHIFT(0x0000);
	pNOR_ID->deviceID1 = *(__IO uint16_t *) ADDR_SHIFT(0x0001);
	pNOR_ID->deviceID2 = *(__IO uint16_t *) ADDR_SHIFT(0x000E);
	pNOR_ID->deviceID3 = *(__IO uint16_t *) ADDR_SHIFT(0x000F);

	sprintf(NorFlashDebugBuffer, "NOR Manufacture ID: 0x%x device ID:0x%x\r\n",
			pNOR_ID->manufatureID, pNOR_ID->deviceID1);
	dbg_printf(NorFlashDebugBuffer);
	sprintf(NorFlashDebugBuffer, "ID2:0x%x ID3:0x%x\r\n", pNOR_ID->deviceID2,
			pNOR_ID->deviceID3);
	dbg_printf(NorFlashDebugBuffer);
}

/**
  * @brief  Sends a byte through the SPI interface and return the byte received
  *         from the SPI bus.
  * @param  byte: byte to send.
  * @retval The value of the received byte.
  */
uint8_t sFLASH_SendByte(uint8_t byte)
{
  /*!< Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(sFLASH_SPI, SPI_I2S_FLAG_TXE) == RESET);

  /*!< Send byte through the SPI1 peripheral */
  SPI_I2S_SendData(sFLASH_SPI, byte);

  /*!< Wait to receive a byte */
  while (SPI_I2S_GetFlagStatus(sFLASH_SPI, SPI_I2S_FLAG_RXNE) == RESET);

  /*!< Return the byte read from the SPI bus */
  return SPI_I2S_ReceiveData(sFLASH_SPI);
}

/**
  * @brief  Reads a byte from the SPI Flash.
  * @note   This function must be used only if the Start_Read_Sequence function
  *         has been previously called.
  * @param  None
  * @retval Byte Read from the SPI Flash.
  */
uint8_t sFLASH_ReadByte(void)
{
  return (sFLASH_SendByte(sFLASH_DUMMY_BYTE));
}

/**
  * @brief  Enables the write access to the FLASH.
  * @param  None
  * @retval None
  */
void sFLASH_WriteEnable(void)
{
  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();

  /*!< Send "Write Enable" instruction */
  sFLASH_SendByte(sFLASH_CMD_WREN);

  /*!< Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();
}

/**
  * @brief  Polls the status of the Write In Progress (WIP) flag in the FLASH's
  *         status register and loop until write opertaion has completed.
  * @param  None
  * @retval None
  */
int sFLASH_WaitForWriteEnd(void)
{
  uint8_t flashstatus = 0;
  int timeout = 180000;
  int error = 0;

  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();

  /*!< Send "Read Status Register" instruction */
  sFLASH_SendByte(sFLASH_CMD_RDSR);

  /*!< Loop as long as the memory is busy with a write cycle */
  do
  {
    /*!< Send a dummy byte to generate the clock needed by the FLASH
    and put the value of the status register in FLASH_Status variable */
    flashstatus = sFLASH_SendByte(sFLASH_DUMMY_BYTE);
  }
  while (((flashstatus & sFLASH_WIP_FLAG) == SET) && timeout--); /* Write in progress */

  /*!< Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();

  if(timeout<=0)
  {
	  error = 1;

	  sprintf(NorFlashDebugBuffer, "SPI flash Write timeout\r\n");
	  dbg_printf(NorFlashDebugBuffer);
  }

  return error;
}

/**
  * @brief  Initiates a read data byte (READ) sequence from the Flash.
  *   This is done by driving the /CS line low to select the device, then the READ
  *   instruction is transmitted followed by 3 bytes address. This function exit
  *   and keep the /CS line low, so the Flash still being selected. With this
  *   technique the whole content of the Flash is read with a single READ instruction.
  * @param  ReadAddr: FLASH's internal address to read from.
  * @retval None
  */
void sFLASH_StartReadSequence(uint32_t ReadAddr)
{
  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();

  /*!< Send "Read from Memory " instruction */
  sFLASH_SendByte(sFLASH_CMD_READ);

  /*!< Send the 24-bit address of the address to read from -------------------*/
  /*!< Send ReadAddr high nibble address byte */
  sFLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /*!< Send ReadAddr medium nibble address byte */
  sFLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /*!< Send ReadAddr low nibble address byte */
  sFLASH_SendByte(ReadAddr & 0xFF);
}

/**
  * @brief  Reads a block of data from the FLASH.
  * @param  pBuffer: pointer to the buffer that receives the data read from the FLASH.
  * @param  ReadAddr: FLASH's internal address to read from.
  * @param  NumByteToRead: number of bytes to read from the FLASH.
  * @retval None
  */
void sFLASH_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();

  /*!< Send "Read from Memory " instruction */
  sFLASH_SendByte(sFLASH_CMD_READ);

  /*!< Send ReadAddr high nibble address byte to read from */
  sFLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /*!< Send ReadAddr medium nibble address byte to read from */
  sFLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /*!< Send ReadAddr low nibble address byte to read from */
  sFLASH_SendByte(ReadAddr & 0xFF);

  while (NumByteToRead--) /*!< while there is data to be read */
  {
    /*!< Read a byte from the FLASH */
    *pBuffer = sFLASH_SendByte(sFLASH_DUMMY_BYTE);
    /*!< Point to the next location where the byte read will be saved */
    pBuffer++;
  }

  /*!< Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();
}
/**
  * @brief  Writes more than one byte to the FLASH with a single WRITE cycle
  *         (Page WRITE sequence).
  * @note   The number of byte can't exceed the FLASH page size.
  * @param  pBuffer: pointer to the buffer  containing the data to be written
  *         to the FLASH.
  * @param  WriteAddr: FLASH's internal address to write to.
  * @param  NumByteToWrite: number of bytes to write to the FLASH, must be equal
  *         or less than "sFLASH_PAGESIZE" value.
  * @retval None
  * @note Page-Program Time: 4 ms/ 256 bytes (typical)
  */
void sFLASH_WritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
  /*!< Enable the write access to the FLASH */
  sFLASH_WriteEnable();

  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();
  /*!< Send "Write to Memory " instruction */
  sFLASH_SendByte(sFLASH_CMD_WRITE);
  /*!< Send WriteAddr high nibble address byte to write to */
  sFLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
  /*!< Send WriteAddr medium nibble address byte to write to */
  sFLASH_SendByte((WriteAddr & 0xFF00) >> 8);
  /*!< Send WriteAddr low nibble address byte to write to */
  sFLASH_SendByte(WriteAddr & 0xFF);

  /*!< while there is data to be written on the FLASH */
  while (NumByteToWrite--)
  {
    /*!< Send the current byte */
    sFLASH_SendByte(*pBuffer);
    /*!< Point on the next byte to be written */
    pBuffer++;
  }

  /*!< Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();

  /*!< Wait the end of Flash writing */
  sFLASH_WaitForWriteEnd();
}

/**
  * @brief  Erases the specified FLASH sector.
  * @param  SectorAddr: address of the sector to erase.
  * @retval None
  * @note Sector-Erase Time: 40 ms (typical)
  */
void sFLASH_EraseSector(uint32_t SectorAddr)
{
  /*!< Send write enable instruction */
  sFLASH_WriteEnable();

  /*!< Sector Erase */
  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();
  /*!< Send Sector Erase instruction */
  sFLASH_SendByte(sFLASH_CMD_SE);
  /*!< Send SectorAddr high nibble address byte */
  sFLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
  /*!< Send SectorAddr medium nibble address byte */
  sFLASH_SendByte((SectorAddr & 0xFF00) >> 8);
  /*!< Send SectorAddr low nibble address byte */
  sFLASH_SendByte(SectorAddr & 0xFF);
  /*!< Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();

  vTaskDelay(40);
  /*!< Wait the end of Flash writing */
  sFLASH_WaitForWriteEnd();
}

/**
  * @brief  Erases the specified FLASH block.
  * @param  SectorAddr: address of the block to erase.
  * @retval None
  * @note Sector-Erase Time: 40 ms (typical) added by lgy 20170502
  */
void sFLASH_EraseBlock(uint32_t BlockAddr)
{
  /*!< Send write enable instruction */
  sFLASH_WriteEnable();

  /*!< Block Erase */
  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();
  /*!< Send Block Erase instruction */
  sFLASH_SendByte(0x20);
  /*!< Send BlockAddr high nibble address byte */
  sFLASH_SendByte((BlockAddr & 0xFF0000) >> 16);
  /*!< Send BlockAddr medium nibble address byte */
  sFLASH_SendByte((BlockAddr & 0xFF00) >> 8);
  /*!< Send BlockAddr low nibble address byte */
  sFLASH_SendByte(BlockAddr & 0xFF);
  /*!< Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();

  vTaskDelay(40);
  /*!< Wait the end of Flash writing */
  sFLASH_WaitForWriteEnd();
}

/**
  * @brief  Erases the entire FLASH.
  * @param  None
  * @retval None
  * @note Chip-Erase Time: 250 ms
  */
void sFLASH_EraseBulk(void)
{
  /*!< Send write enable instruction */
  sFLASH_WriteEnable();

  /*!< Bulk Erase */
  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();
  /*!< Send Bulk Erase instruction  */
  sFLASH_SendByte(sFLASH_CMD_BE);
  /*!< Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();

  /*!< Wait the end of Flash writing */
  vTaskDelay(250);
  sFLASH_WaitForWriteEnd();
}

/**
  * @brief  Writes block of data to the FLASH. In this function, the number of
  *         WRITE cycles are reduced, using Page WRITE sequence.
  * @param  pBuffer: pointer to the buffer  containing the data to be written
  *         to the FLASH.
  * @param  WriteAddr: FLASH's internal address to write to.
  * @param  NumByteToWrite: number of bytes to write to the FLASH.
  * @retval None
  */
void sFLASH_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
  uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

  Addr = WriteAddr % sFLASH_SPI_PAGESIZE;
  count = sFLASH_SPI_PAGESIZE - Addr;
  NumOfPage =  NumByteToWrite / sFLASH_SPI_PAGESIZE;
  NumOfSingle = NumByteToWrite % sFLASH_SPI_PAGESIZE;

  if (Addr == 0) /*!< WriteAddr is sFLASH_PAGESIZE aligned  */
  {
    if (NumOfPage == 0) /*!< NumByteToWrite < sFLASH_PAGESIZE */
    {
      sFLASH_WritePage(pBuffer, WriteAddr, NumByteToWrite);
    }
    else /*!< NumByteToWrite > sFLASH_PAGESIZE */
    {
      while (NumOfPage--)
      {
        sFLASH_WritePage(pBuffer, WriteAddr, sFLASH_SPI_PAGESIZE);
        WriteAddr +=  sFLASH_SPI_PAGESIZE;
        pBuffer += sFLASH_SPI_PAGESIZE;
      }

      sFLASH_WritePage(pBuffer, WriteAddr, NumOfSingle);
    }
  }
  else /*!< WriteAddr is not sFLASH_PAGESIZE aligned  */
  {
    if (NumOfPage == 0) /*!< NumByteToWrite < sFLASH_PAGESIZE */
    {
      if (NumOfSingle > count) /*!< (NumByteToWrite + WriteAddr) > sFLASH_PAGESIZE */
      {
        temp = NumOfSingle - count;

        sFLASH_WritePage(pBuffer, WriteAddr, count);
        WriteAddr +=  count;
        pBuffer += count;

        sFLASH_WritePage(pBuffer, WriteAddr, temp);
      }
      else
      {
        sFLASH_WritePage(pBuffer, WriteAddr, NumByteToWrite);
      }
    }
    else /*!< NumByteToWrite > sFLASH_PAGESIZE */
    {
      NumByteToWrite -= count;
      NumOfPage =  NumByteToWrite / sFLASH_SPI_PAGESIZE;
      NumOfSingle = NumByteToWrite % sFLASH_SPI_PAGESIZE;

      sFLASH_WritePage(pBuffer, WriteAddr, count);
      WriteAddr +=  count;
      pBuffer += count;

      while (NumOfPage--)
      {
        sFLASH_WritePage(pBuffer, WriteAddr, sFLASH_SPI_PAGESIZE);
        WriteAddr +=  sFLASH_SPI_PAGESIZE;
        pBuffer += sFLASH_SPI_PAGESIZE;
      }

      if (NumOfSingle != 0)
      {
        sFLASH_WritePage(pBuffer, WriteAddr, NumOfSingle);
      }
    }
  }
}

/**
  * @brief  Reads FLASH identification.
  * @param  None
  * @retval FLASH identification
  */
uint32_t sFLASH_ReadID(void)
{
  uint32_t Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0, Temp3 = 0;

  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();

  /*!< Send "RDID " instruction */
  sFLASH_SendByte(0x9F);

  /*!< Read a byte from the FLASH */
  Temp0 = sFLASH_SendByte(sFLASH_DUMMY_BYTE);

  /*!< Read a byte from the FLASH */
  Temp1 = sFLASH_SendByte(sFLASH_DUMMY_BYTE);

  /*!< Read a byte from the FLASH */
  Temp2 = sFLASH_SendByte(sFLASH_DUMMY_BYTE);

  /*!< Read a byte from the FLASH */
  Temp3 = sFLASH_SendByte(sFLASH_DUMMY_BYTE);

  /*!< Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();

  Temp = (Temp0<<24) | (Temp1 << 16) | (Temp2 << 8) | Temp3;

  return Temp;
}

void eSPIFLASH_TEST(void)
{
	uint32_t SpiFlashID;
	uint32_t counter;
	uint32_t address;
	uint32_t i;
	uint32_t *p;
	uint32_t error = 0;
	/* 1. verify the device, it should be */
	SpiFlashID = sFLASH_ReadID();
	if(SpiFlashID != 0x62061300)
	{
		sprintf(NorFlashDebugBuffer, "SPI flash id: 0x%08X don't match\r\n", SpiFlashID);
	}

	else
	{
		sprintf(NorFlashDebugBuffer, "SPI flash id: 0x%08X match\r\n", SpiFlashID);
	}

	dbg_printf(NorFlashDebugBuffer);

	/* 2. verify the chip erase command */
	sFLASH_EraseBulk();
	for(counter=0,address=0;(counter<(sFLASH_SPI_CHIPSIZE/sFLASH_SPI_PAGESIZE)) && (!error); counter++)
	{
		sFLASH_ReadBuffer((uint8_t *)eSPIFlashPageBuffer, address, sFLASH_SPI_PAGESIZE);
		p = (uint32_t *)eSPIFlashPageBuffer;
		for(i=0;i<sFLASH_SPI_PAGESIZE/sizeof(uint32_t);i++)
		{
			if(*p++ != 0xFFFFFFFF)/* location can not erase to FF */
			{
				sprintf(NorFlashDebugBuffer, "SPI Erase error: 0x%08X don't match\r\n", address);
				dbg_printf(NorFlashDebugBuffer);
				error = 1;
				break;
			}
		}
		address += sFLASH_SPI_PAGESIZE;
	}
	if(!error)
	{
		dbg_printf("SPI flash Erase verification done!\r\n");
	}
	/* 3. verify the write command */
	for(counter=0,address=0;(counter<(sFLASH_SPI_CHIPSIZE/sFLASH_SPI_PAGESIZE)) && (!error); counter++)
	{
		memset(eSPIFlashPageBuffer, 0xaa, sFLASH_SPI_PAGESIZE);
		sFLASH_WriteBuffer(eSPIFlashPageBuffer, address, sFLASH_SPI_PAGESIZE);
		memset(eSPIFlashPageBuffer, 0, sFLASH_SPI_PAGESIZE);
		sFLASH_ReadBuffer((uint8_t *)eSPIFlashPageBuffer, address, sFLASH_SPI_PAGESIZE);
		p = (uint32_t *)eSPIFlashPageBuffer;
		for(i=0;i<sFLASH_SPI_PAGESIZE/sizeof(uint32_t);i++)
		{
			if(*p != 0xaaaaaaaa)
			{
				sprintf(NorFlashDebugBuffer, "SPI Write error: 0x%08X don't match\r\n", address);
				dbg_printf(NorFlashDebugBuffer);
				error = 2;
				break;
			}
		}
		address += sFLASH_SPI_PAGESIZE;
	}
	if(!error)
	{
		dbg_printf("SPI flash Write verification done!\r\n");
	}
}

void eSPIFLASH_Request(eSPIFLASH_Request_t req)
{
	SPI_Request = req;
}
uint16_t Binary_Search_Address(uint32_t address)
{
    uint16_t  low = 0;
    uint16_t  high = 4095;
    uint16_t  middle;
    uint8_t    pBuffer;
    while(low<=high)
    	{
        middle=(high+low)/2;
	sFLASH_ReadBuffer( &pBuffer, address+16*middle, 1);
        if(pBuffer!=0xff)
			low=middle+1;
	else high=middle;
	}
	return low;
}
uint16_t Nomal_Search_Address(uint32_t address)
{
	uint16_t addre_temp;
	uint8_t    pBuffer;
       for(addre_temp=0;addre_temp<65535-16;addre_temp+=0x0010)
       	{
         sFLASH_ReadBuffer( &pBuffer, address+addre_temp, 1);
		 if(0XFF==pBuffer)
		 	break;
	}
      return addre_temp;
}
/*******************************************************************************
 * @name: Write_One_Alarm
 * @input:  address : the address of Flash, block address = week*the base address of block
 *               data:       the message of alarm
 * @output: void
 * @role:  write  a alarm message to address of Flash

*******************************************************************************/
void  Write_One_Alarm(uint32_t address,AlarmMessage *data)
{
  uint8_t pBuffer[13];
    pBuffer[0]=data->alarm_level;
  pBuffer[1]=data->time.year>>8;
  pBuffer[2]=(uint8_t)(data->time.year);
  pBuffer[3]=data->time.month;
  pBuffer[4]=data->time.date;
  pBuffer[5]=data->time.hour;
  pBuffer[6]=data->time.minute;
  pBuffer[7]=data->time.second;
  pBuffer[8]=data->alarm_kind;
  pBuffer[9]=(data->alarm_content)>>24;
  pBuffer[10]=(data->alarm_content)>>16;
  pBuffer[11]=(data->alarm_content)>>8;
  pBuffer[12]=(data->alarm_content);
 sFLASH_WriteBuffer( (uint8_t *)pBuffer, address+Nomal_Search_Address(address), 13);
 
}
/*******************************************************************************
 * @name: Get_One_Alarm
 * @input:  pBuff : the address of Flash, block address = week*the base address of block
 *               address:       the block address of Flash
 * @output: void
 * @role:    Get one alarm message

*******************************************************************************/
uint8_t  Get_One_Alarm(uint8_t *pBuff,uint32_t address)//add by lgy 20170503
{
   sFLASH_ReadBuffer( pBuff, address, 1);//
    if(0Xff!=*pBuff)//???
	 {
            sFLASH_ReadBuffer( pBuff, address, 13);
           return 0;
	  }  
    else
	   return  1;
}

void Task_NorFlash(void* p)
{
	uint16_t *norflashdata;
	uint32_t SpiFlashID;

	norflashdata = (uint16_t *)0x60000000;
	while(1)
	{
		vTaskDelay(TASK_NOR_FLASH_PERIOD);
#if 0
		NorFlashReadSectorId(0x60000000, &sectorId);
#else
		//NOR_ReadID(&sectorId);
#endif
		if(0 && *norflashdata != 0xaa55)
		{
			sprintf(NorFlashDebugBuffer, "Nor flash not programmed! 0x%x\r\n", *norflashdata);
			dbg_printf(NorFlashDebugBuffer);
			NorFlashProgramWord(0x60000000, 0xaa55);
		}

		if(SPI_Request == eSPIFLASH_Request_Test)
		{
			SPI_Request = eSPIFLASH_Request_Idle;
			eSPIFLASH_TEST();
		}
		else if(SPI_Request == eSPIFLASH_Request_ReadID)
		{
			SPI_Request = eSPIFLASH_Request_Idle;
			SpiFlashID = sFLASH_ReadID();
			sprintf(NorFlashDebugBuffer, "SPI flash id: 0x%08X\r\n", SpiFlashID);
			dbg_printf(NorFlashDebugBuffer);

		}
	}

	vTaskDelete(NULL);
}
