#ifndef __task6_h__
#define __task6_h__

#include "multi-task.h"
#include "FreeRTOS.h"
#include "stm32f4xx_spi.h"
#include "debug.h"

#define TASK6_STACK_SIZE	256

typedef struct
{
    uint16_t reg_addr;  
    uint16_t data;
    int wr;
} FGPA_DATA_t;

typedef enum
{
	McuSpi3Mux_None = 0,
	McuSpi3Mux_AD9548,
	McuSpi3Mux_ACS9522,
	McuSpi3Mux_KSZ8795,
	McuSpi3Mux_ADS7953,
	McuSpi3Mux_BCM5396,
	McuSpi3Mux_LMK04031
} McuSpi3Mux_t;

extern StackType_t Task6Stack[TASK6_STACK_SIZE] CCM_RAM;  // Put task stack in CCM
extern StaticTask_t Task6Buffer CCM_RAM;  // Put TCB in CCM

void Task6(void* p);

extern uint16_t Write_FPGA(uint16_t reg_addr, uint16_t data);
extern uint16_t Read_FPGA(uint16_t reg_addr);
extern void Fpga_ContrlRequest(FGPA_DATA_t reg);

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
McuSpi3Mux_t Fpga_GetMcuSpi3Sel(void);


//fpga_register_address, set but not use;
#define	FPGA_REG_VER_ADDRESS			0x0002
#define FPGA_TEST_REG_VER_ADDRESS			0x0004
#define FPGA_HARDWARE_REG_ADDRESS 	0X0006
#define FPGA_LOCALBUS_WE_ADDRESS		0X0010
#define	FPGA_LOCALBUS_TEST_ADDRESS		0X0012
#define FPGA_CPU_VDD_ADDRESS		0X0020
#define FPGA_SPI3_SELECT_ADDRESS		0X0022
#define FPGA_UART1OR3_SELECT_ADDRESS		0X0024
#define	FPGA_1PPS_CLOCK_SELECT_ADDRESS		0X0026
#define	FPGA_1PPS_STATUS_ADDRESS		0X0027
#define	FPGA_ADS7953_CHANNEL0_ADDRESS		0X0030
#define	FPGA_ADS7953_CHANNEL1_ADDRESS		0X0031
#define	FPGA_ADS7953_CHANNEL2_ADDRESS		0X0032
#define	FPGA_ADS7953_CHANNEL3_ADDRESS		0X0033
#define	FPGA_ADS7953_CHANNEL4_ADDRESS		0X0034
#define	FPGA_ADS7953_CHANNEL5_ADDRESS		0X0035
#define	FPGA_ADS7953_CHANNEL6_ADDRESS		0X0036
#define	FPGA_ADS7953_CHANNEL7_ADDRESS		0X0037
#define	FPGA_ADS7953_CHANNEL8_ADDRESS		0X0038
#define	FPGA_ADS7953_CHANNEL9_ADDRESS		0X0039
#define	FPGA_ADS7953_CHANNEL10_ADDRESS		0X003A
#define	FPGA_ADS7953_CHANNEL11_ADDRESS		0X003B
#define	FPGA_ADS7953_CHANNEL12_ADDRESS		0X003C
#define	FPGA_ADS7953_CHANNEL13_ADDRESS		0X003D
#define	FPGA_ADS7953_CHANNEL14_ADDRESS		0X003E
#define	FPGA_ADS7953_CHANNEL15_ADDRESS		0X003F
#define	FPGA_AD9548_MODE_CONFIG_ADDRESS		0X0040
#define	FPGA_TIME_MARK_CONFIG_ADDRESS		0X0042
#define	FPGA_ANTENNA_POWER_SELECT_ADDRESS		0X0043
#define	FPGA_ACS9522_CONFIG_ADDRESS		0X0044
#define	FPGA_FAN_SPEED_CTL_ADDRESS		0X0046
#define FPGA_FAN0_SPEED_FEEDBACK_ADDRESS		0X0048
#define FPGA_FAN1_SPEED_FEEDBACK_ADDRESS		0X004A
#define FPGA_FAN2_SPEED_FEEDBACK_ADDRESS		0X004C
#define FPGA_FAN3_SPEED_FEEDBACK_ADDRESS		0X004E
#define	FPGA_RESET_CTL_ADDRESS		0X0050
#define	FPGA_CLOCK_LOCK_STATUS_QUERY_ADDRESS		0X0051
#define	FPGA_INTERRUPT_STATUS_ADDRESS		0X0052
#define	FPGA_CLOCK_UNLOCK_NUM_QUERY_ADDRESS		0X0053
#define FPGA_CLOCK_UNLOCK_NUM_CLEAR_ADDRESS		0X0054
#define FPGA_INTERRUPT_CLEAR_ADDRESS		0X0055
#define FPGA_ACHASSI_CONFIG_SERIAL_DDRESS		0X0056
#define	FPGA_CM55_MODE_SELECT_ADDRESS		0X0057
#define	FPGA_CPU_STATUS_QUERY_ADDRESS		0X0058
#define	FPGA_CPU_TEMP_CURRENT_ADDRESS		0X0059
#define	FPGA_TEMP_ADDRESS		0X60
#define	FPGA_TEMP_0OR1_ADDRESS		0X0061
#define	FPGA_TEMP2_ADDRESS		0X0062





#endif /*__task6_h__*/
