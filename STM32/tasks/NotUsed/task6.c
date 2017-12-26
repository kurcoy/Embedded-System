#include "multi-task.h"
#include "FreeRTOS.h"
#include "task6.h"
#include "stm32f4xx_spi.h"
#include "debug.h"
#include "init.h"
#include "task7.h"

StackType_t Task6Stack[TASK6_STACK_SIZE] CCM_RAM;  // Put task stack in CCM
StaticTask_t Task6Buffer CCM_RAM;  // Put TCB in CCM
static volatile int SpiLock = 0;

static int Flagprint = 0;
static char DebugBuffer[64];
static FGPA_DATA_t  WriteReq;

#ifdef TASK6_DEBUG
#define BUFFERSIZE	512

static uint8_t spirx[BUFFERSIZE];
static uint8_t teststr[BUFFERSIZE];
#endif

extern void Gpio6Test(void);


static int FPGA_ContrlRequestFlag = 0;

uint16_t SPI2_ReadWriteByte(uint16_t TxData)
{
    while (SPI_GetFlagStatus(SPI2, SPI_FLAG_TXE) == RESET);  //Wait for sending area empty
    SPI_SendData(SPI2, TxData);                              //send 1 byte data bY spi2
    while (SPI_GetFlagStatus(SPI2, SPI_FLAG_RXNE) == RESET); //Wait for recv 1byte  data
    return SPI_ReceiveData(SPI2);                            //return recv data

}

uint16_t Spi_FPGA_Read_Register(uint16_t data_in, uint16_t reg_addr, uint8_t wr_flag)        //spi readwrite fpga register
{

    uint16_t Temp = 0;
    
    uint16_t wr_bit = 0x0000;
    if(wr_flag == 1)
    {
       wr_bit = 0x8000;
    }
    uint16_t reg = (reg_addr | wr_bit);
    
    GPIO_ResetBits(GPIOI, GPIO_Pin_0);                          /* SPI CS low */
    SPI2_ReadWriteByte((uint16_t)(reg)&0xffff);               //send addr 
    Temp = SPI2_ReadWriteByte((uint16_t)(data_in)&0xffff);           //send data_in
   
#ifdef TASK6_DEBUG
    sprintf(teststr, "\r\nreg_addr:%x\r\n", ((uint16_t)(reg)&0xffff));
    dbg_printf(teststr);
    
    sprintf(teststr, "\r\ndata_in:%x\r\n", ((uint16_t)(data_in)&0xffff));
    dbg_printf(teststr);

    sprintf(spirx, "\r\nTemp:%x\r\n", Temp);
    dbg_printf(spirx);
#endif
    GPIO_SetBits(GPIOI, GPIO_Pin_0); /* SPI CS high */

    return Temp;                            //return fpga inside data
}


uint16_t Read_FPGA(uint16_t reg_addr)//set the FPGA address and data,
{
    uint16_t data_rsp;
    uint16_t default_data = 0x0000;

    /* in case multi-thread operation*/
	while(SpiLock);
	SpiLock++;
    data_rsp = Spi_FPGA_Read_Register(default_data, reg_addr, 0);
    if(Flagprint)
    {
    	Flagprint = 0;
    	sprintf(DebugBuffer, "Read_FPGA 0x%04x 0x%04x\r\n", reg_addr, data_rsp);
    	dbg_printf(DebugBuffer);
    }
    SpiLock--;

    return data_rsp;
}

uint16_t Write_FPGA(uint16_t reg_addr, uint16_t data)
{
    uint16_t data_rsp;
     
    /* in case multi-thread operation*/
    while(SpiLock);
    SpiLock++;
    data_rsp = Spi_FPGA_Read_Register(data, reg_addr, 1);
    SpiLock--;

    if(reg_addr == 0x0024)
    {
      Fpga_CacheUart3Sel((McuUart3Mux_t)data);
    }
    return data_rsp;
}

uint16_t Mcu_Get_16AD_Value(uint8_t channal)
{
     uint16_t addr=0x0030;
     uint16_t AD_Val;
     AD_Val=Read_FPGA(addr+channal);
    //sprintf(DebugBuffer, "The Channel of  %d Voltage is %d\r\n", AD_Val>>12,AD_Val&0x0fff);
   //dbg_printf(DebugBuffer); 
   return (AD_Val&0x0fff);
}


void Fpga_ContrlRequest(FGPA_DATA_t reg)   //use to contrl the fpga register;
{
    FPGA_ContrlRequestFlag = 1;         //flag is 1,means want to write or read fpga register;
    WriteReq = reg;
    dbg_printf("\r\nContrl FPGA regsitors!\r\n");
}
/*******************************************************************************
 * @name: Fpga_GetMcuSpi3Sel
 * @input: void
 * @output: the SPI3 route
 * @role: read the sector information
 * 0. McuSpi3Mux_None
 * 1. McuSpi3Mux_AD9548
 * 2. McuSpi3Mux_ACS9522
 * 3. McuSpi3Mux_KSZ8795
 * 4. McuSpi3Mux_ADS7953
 * 5. McuSpi3Mux_BCM5396
 * 6. McuSpi3Mux_LMK04031
*******************************************************************************/

McuSpi3Mux_t Fpga_GetMcuSpi3Sel(void)
{
	uint16_t ret=0;

	ret = (McuSpi3Mux_t)Read_FPGA(0x0022);

	return ret;
}
/*******************************************************************************
 * @name: Mcu_Get_Fan_Rotate_Speed
 * @input:  fan_id : 0,1,2,3;The fan's ID
 * @output: fan's rotate speed
 * @role:    MCU get the rotate speed of fan

*******************************************************************************/
uint16_t  Mcu_Get_Fan_Rotate_Speed(int fan_id)
{
    uint16_t rotate_base_addre=0x0048;
    return Read_FPGA(rotate_base_addre+2*fan_id);
}
/*******************************************************************************
 * @name: Mcu_Ctrl_Fan_Speed
 * @input:  fan_id : 0,1,2,3;The fan's ID
 *               speed:  0,full speed;
                              1,high speed;
                              2,middle speed;
                              3,zero speed.
 * @output: void
 * @role:    MCU control the speed of fan

*******************************************************************************/
void Mcu_Ctrl_Fan_Speed(int fan_id,uint8_t  speed)
{
    uint16_t temp;
   temp=Read_FPGA(0X0046);
   Write_FPGA(0X0046, temp|(uint16_t)(speed<<(2*fan_id)));
   //sprintf(DebugBuffer, "Read the Fan Speed:0X%x\r\n", temp);
    //dbg_printf(DebugBuffer);
}


void Task6(void* p)
{
	uint16_t data;
	while (1)
	{
        Write_FPGA(0x0022, 0x0006);
		vTaskDelay(500);
		data = Read_FPGA(0x0022);
		sprintf(DebugBuffer, "ReadFPGA 0x0022 0x%04x\r\n", data);
		dbg_printf(DebugBuffer);
		if(data == (uint16_t)0x0006)
		{
			break;
		}
#if 0
        Write_FPGA(0x0022, 0x0001);
		vTaskDelay(500);
		data = Read_FPGA(0x0022);
		sprintf(DebugBuffer, "ReadFPGA 0x0022 0x%04x\r\n", data);
		dbg_printf(DebugBuffer);
		if(data == (uint16_t)0x0001)
		{
			break;
		} 
#endif
		vTaskDelay(500);
	}
    while(1)
    {

        if(FPGA_ContrlRequestFlag)
        {
#ifdef TASK6_DEBUG
            dbg_printf("\r\nTask6 running\r\n");
#endif
			Flagprint = 1;
            FPGA_ContrlRequestFlag = 0;
            if(WriteReq.wr == 1)
            {
                Write_FPGA(WriteReq.reg_addr, WriteReq.data);
            }
            else
            {
                Read_FPGA(WriteReq.reg_addr);
            }
        }

		vTaskDelay(1000); /* 1000ms */
	}

	vTaskDelete(NULL);
}
