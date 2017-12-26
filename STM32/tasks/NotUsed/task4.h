#ifndef __task4_h__
#define __task4_h__

#include "multi-task.h"

#define TASK4_STACK_SIZE	256
#define DM7304_ADDRESS	(uint8_t)(0x28) /* mount to I2C2 bus */
#define ADT7461_ADDRESS	(uint8_t)(0x4C) /* mount to I2C3 bus */
#define PCA9555_ADDRESS	(uint8_t)(0x40) /* mount to I2C2 bus */
#define INA220_ADDRESS	(uint8_t)(0x40) /* mount to I2C3 bus */

/* 7 bit address, without the R/W bit */
#define LM92_TEMPERATURE1_ADDRESS	(uint8_t)(0x48)
#define LM92_TEMPERATURE2_ADDRESS	(uint8_t)(0x49)
#define LM92_TEMPERATURE3_ADDRESS	(uint8_t)(0x4a)


#define PCA9555_1_ADDRESS	(uint8_t)(0x20)
#define PCA9555_2_ADDRESS	(uint8_t)(0x21)

#define I2C2_ITSELF_ADDRESS	(0x5a)
#define I2C_TIMEOUT_MAX		(168000)

#define DPM_WP 0x96
#define WRP_OPCODE 0x01
#define WRM_OPCODE 0x02
#define WSR_OPCODE 0x05
#define RRP_OPCODE 0x11
#define RRM_OPCODE 0x12
#define SSR_OPCODE 0x33
#define SRM_OPCODE 0x34
#define CRC_OPCODE 0x2f
#define LSR_OPCODE 0x32
#define RSM_OPCODE 0x40
#define RPSS_OPCODE 0x44
#define RMDP_OPCODE 0x45


#define DPM_SUCCESS 0x01
#define DPM_EXEC_FAIL 0x0

#define DPM_FLAG0 0x1UL
#define DPM_FLAG1 (0x1UL<<1)
#define DPM_FLAG2 (0x1UL<<2)
#define DPM_FLAG3 (0x1UL<<3)
#define DPM_FLAG4 (0x1UL<<4)
#define DPM_FLAG5 (0x1UL<<5)
#define DPM_FLAG6 (0x1UL<<6)
#define DPM_FLAG7 (0x1UL<<7)
#define DPM_FLAG8 (0x1UL<<8)
#define DPM_FLAG9 (0x1UL<<9)
#define DPM_FLAG10 (0x1UL<<10)
#define DPM_FLAG11 (0x1UL<<11)
#define DPM_FLAG12 (0x1UL<<12)
#define DPM_FLAG13 (0x1UL<<13)
#define DPM_FLAG14 (0x1UL<<14)
#define DPM_FLAG15 (0x1UL<<15)
#define DPM_FLAG16 (0x1UL<<16)

#define LM92_TEMPERATURE_REG	(0x0)
#define LM92_CFG_REG			(0x01)
#define LM92_THSYT_REG			(0x02)
#define LM92_TCRIT_REG			(0x03)
#define LM92_TLOW_REG			(0x04)
#define LM92_THIGH_REG			(0x05)
#define LM92_MF_ID_REG			(0x07)


#define INA220_CFG_BRNG			(0x1)
#define INA220_CFG_BRNG_OFFSET	(13)

#define INA220_CFG_PG			(0x1)/* rang 80mv*/
#define INA220_CFG_PG_OFFSET	(11)

#define INA220_CFG_BADC			(0x3)/* 12 bits resolution */
#define INA220_CFG_BADC_OFFSET	(7)

#define INA220_CFG_SADC			(0x3)/* 12 bits resolution */
#define INA220_CFG_SADC_OFFSET	(3)

#define INA220_CFG_MODE			(0x7) /* Shunt and Bus, Continuous */
#define INA220_CFG_MODE_OFFSET	(0)

#define INA220_REG_CFG			(0)
#define INA220_REG_VSHUNT		(1)
#define INA220_REG_VBUS			(2)
#define INA220_REG_POWER		(3)
#define INA220_REG_CURRENT		(4)
#define INA220_REG_CALIBRATION	(5)

#define AT24C02_CHIP1			(0x52)
#define AT24C02_CHIP2			(0x53)
#define AT24C02_PAGE_SIZE		(8) /* 8 bytes per page */
#define AT24C02_CHIP_SIZE		(32*AT24C02_PAGE_SIZE) /* 32 pages = 256 bytes */

#define AT24C64_CHIP1			(0x50)
#define AT24C64_PAGE_SIZE		(32) /* 32 bytes per page */
#define AT24C64_CHIP_SIZE		(256*AT24C64_PAGE_SIZE) /* 256 pages = 8KB */

/* Max_Expected_I/32768 < Current_LSB < Max_Expected_I/4096 => 1.8 x 10^-3 */
/* CAL = 0.04096/(Current_LSB x Rshunt) */
#define INA220_CALIBRATION		(22755)

#define ADT7461_LTV_REG	        (0x0)
#define ADT7461_ETVHB_REG	    (0x1)
#define ADT7461_ETVLB_REG	    (0x10)
#define ADT7461_MF_ID_REG	    (0xFE)
#define Ext_Offset_H_Reg            (0x11)
#define Ext_Offset_L_Reg            (0x12)
#define Config_Write_Reg            (0x09)
#define Config_Read_Reg             (0x03)
#define Conv_Write_Reg              (0x0A)

extern StackType_t Task4Stack[TASK4_STACK_SIZE] CCM_RAM;  // Put task stack in CCM
extern StaticTask_t Task4Buffer CCM_RAM;  // Put TCB in CCM

typedef enum
{
	I2C_ReadOnly = 0,
	I2C_WriteOnly,
	I2C_WriteRead
}I2C_TransimtType_t;

typedef struct
{
	I2C_TransimtType_t dir; /* operation direction */
	uint8_t slave; /* 7 bits of slave address */
	uint8_t wLen;  /* number of bytes to write */
	uint8_t rLen;  /* number of bytes to read */
	uint8_t *write;/* point to write buffer */
	uint8_t *read; /* point to read buffer*/
}I2C_TransmiData_t;

typedef struct
{
	uint8_t addr;
	uint8_t value;
}DM7304_RegTable_t;

typedef struct {
	uint8_t st;
	uint8_t vh;
	uint8_t vl;
	uint8_t io;
	uint8_t tmp;
} Pol_Status_t;

typedef struct {
	uint8_t pss0;
	uint8_t pss1;
	uint8_t pss2;
	uint8_t pss3;
} Pol_PSS_t;

typedef struct {
	uint8_t sta;
	uint8_t stb;
	uint8_t stc;
	uint8_t std;
	uint8_t dpms;
	uint8_t est;
} DPM_RSM_t;

void Task4(void* p);

void dpm_setflag(uint32_t flag);

/*******************************************************************************
 * name: dpm_pol_set_voltage
 * in: vol -> the output voltage in 1mV unit
 *     len -> byte sum to read from DPM
 * out: 0 if success, else non 0
 * role: set the output voltage to the given value
*******************************************************************************/
int dpm_pol_set_voltage(uint32_t vol);

/*******************************************************************************
 * name: PCA9555_InputOutConfig
 * role: configure the PCA9555 IOs as input/output
 * parameters: config -> 1bit/pin MSB, the value updated to global data PCA9555_Config
 * 			   it will be updated to the chip in a periodic task.
 * note: this function configure the specified pin as input or output
 * 		bit value 0 -> output
 * 		bit value 1 -> input
*******************************************************************************/
int PCA9555_InputOutConfig(uint32_t config);

/*******************************************************************************
 * name: PCA9555_Output
 * role: configure the PCA9555 IOs' output level
 * parameters: config -> 1bit/pin MSB, the value updated to global data PCA9555_Out
 * 			   it will be updated to the chip in a periodic task.
 * note: this function configure the specified pins' level
 * 		bit value 0 -> Low level
 * 		bit value 1 -> High level
*******************************************************************************/
int PCA9555_Output(uint32_t value);

/*******************************************************************************
 * name: PCA9555_Input
 * role: get the PCA9555 IOs' cache level, the update rate depend on the calling
 * 		task's period
 * parameters: void
 * note: this function return the all pins' level
 * 		bit value 0 -> Low level
 * 		bit value 1 -> High level
*******************************************************************************/
int PCA9555_Input(void);

void AT24C02_TestRequest(uint8_t addr);
void AT24C64_TestRequest(uint16_t addr);

#endif /*__task4_h__*/
