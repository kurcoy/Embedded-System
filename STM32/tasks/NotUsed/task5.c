/* this task is the driver for the lmk04031 */

#include "multi-task.h"
#include "FreeRTOS.h"
#include "task5.h"
#include "stm32f4xx_usart.h"
#include "debug.h"
#include "string.h"
#include <stdarg.h>
#include "task6.h"
#include "task_nor_flash.h"
#include "task4.h"
#include "task7.h"
#include "flash_if.h"

#define BUFFERSIZE	512

#define UART_ECHO_FLAG				(0x1)
#define UART_NEW_LINE_FLAG			(0x1<<1)
#define UART_TRANSMITING_FLAG		(0x1<<2)
#define UART_RECEIVING_FLAG			(0x1<<3)

typedef enum {
	Idle,
	TranslateInput,
	Output
}Task_State_t;

typedef struct
{
	uint8_t *argv[5];
	uint32_t len[5];
}Parmeters_t;

typedef void CmdResponse(char *str);

typedef struct
{
	char *name;
	CmdResponse *resp;
}Command_List_t;

StackType_t Task5Stack[TASK5_STACK_SIZE] CCM_RAM;  // Put task stack in CCM
StaticTask_t Task5Buffer CCM_RAM;  // Put TCB in CCM

static uint32_t UartFlag =0;
static uint32_t ubRxIndex=0;
static uint32_t ubTxIndex=0;
static uint32_t EchoRIndex=0;
static uint32_t EchoTIndex=0;

static uint32_t TxLen = 0;
uint32_t PrintLen= 0;
static uint32_t RxLen = 0;
static uint32_t InputLen= 0;
static uint8_t aRxBuffer[BUFFERSIZE];
static uint8_t aTxBuffer[BUFFERSIZE];
static uint8_t InputStr[BUFFERSIZE];
static uint8_t PrintBuff[BUFFERSIZE];

static Task_State_t state=Idle;

void GetVersion(char *input);
void PowerHandler(char *input);
void Help(char *input);
void WriteFPGAHandler(char *input);
void ReadFPGAHandler(char *input);
void SpiFlashHandler(char *input);
void CM55MsgHandler(char *input);
void PCA9555Handler(char *input);
void AT24C02Handler(char *input);
void AT24C64Handler(char *input);
void LM92Handler(char *input);
void INA220Handler(char *input);
void AD9548Handler(char *input);
void ADT7461Handler(char *input);
void GpsCmdHandler(char *input);
void McuReset(char *input);
void McuUpgrade(char *input);

static char Task5DebugBuffer[64];
Command_List_t CmdList[] =
{
		{"Help", Help},
		{"GetVersion", GetVersion},
		{"Power", PowerHandler},
        {"WriteFPGA", WriteFPGAHandler},
        {"ReadFPGA", ReadFPGAHandler},
		{"SpiFlash", SpiFlashHandler},
		{"GetCM55Msg", CM55MsgHandler},
		{"PCA9555", PCA9555Handler},
		{"AT24C02", AT24C02Handler},
		{"AT24C64", AT24C64Handler},
        {"LM92", LM92Handler},
		{"INA220", INA220Handler},
		{"AD9548", AD9548Handler},
		{"ADT7461", ADT7461Handler},
        {"GPSCMD", GpsCmdHandler},
		{"Reset", McuReset},
		{"Upgrade", McuUpgrade}
};

int dbg_printf(const char *str)
{
    static int lock = 0;
	int ret = 0;

	while(lock);
	lock = 1;


	ret = PrintLen +  strlen(str);

	if(ret > sizeof(PrintBuff))
	{
		ret = -1;
	}
	else
	{
		sprintf(&PrintBuff[PrintLen], str);
		PrintLen += strlen(str);
	}
	
    lock = 0;

	return ret;

}

int translat(char c)
{
    if(c <= '9' && c >= '0') 
        return c - '0';
    if(c >= 'a' && c <= 'f') 
        return c - 87;
    if(c >= 'A' && c <= 'F') 
        return c - 55;
    return -1;
}
int Htoi(char *str)              //hex to int;
{
    int length = strlen(str); 
    if(length == 0) 
        return 0;
    int i, n=0, stat;
    for(i=0; i<length; i++) 
    {
        stat = translat(str[i]); 
        if(stat >= 0)
            n = n*16+stat;
    }
    return n;
}

/* str like:WriteFPGA 0x0020 0x5A5A means (fpga_addr is 0x0020  data is 0x5A5A); or 
            ReadFPGA 0x0020  means(fpga_addr if 0x0020, data will set the default data 0x0000, 
*/
int handle_str(char *ptr, int*data)  
{
    int input_ok = 1;
    char reg_addr[6];
    char input_data[6];

    int num, w_len, r_len;
        
    char* w_key = "WriteFPGA";
    char* r_key = "ReadFPGA";

    char *w_str;
    char *r_str;
    w_str = strstr(ptr, w_key);
    r_str = strstr(ptr, r_key);
    

    if(w_str)
    {
        w_len = strlen(w_str);
        if(w_len >= 23)
        {
            strncpy(reg_addr,w_str+10, 6);
            reg_addr[6] = 0;
             
            strncpy(input_data,w_str+17, 6);
            input_data[6] = 0;
            
            *data = Htoi(reg_addr);           
            *(data+1) = Htoi(input_data);
            
            dbg_printf("\r\nWriteSUCESS\r\n");

            input_ok = 1;
        }
        else
        {
            dbg_printf("Write command erro,like:WriteFPGA 0x0020 0x5A5A!!!\n");
            input_ok = 0;
        }
    }

    if(r_str)
    {

        r_len = strlen(r_str);
        if(r_len >= 15)
        {
            strncpy(reg_addr,r_str+10, 6);
            reg_addr[6] = 0;
             
            *data = Htoi(reg_addr);
            *(data+1) = 0x0000;
            
            input_ok = 1;
            
            dbg_printf("\r\nReadSUCESS\r\n");
        }
        else
        {
            dbg_printf("Read command erro,like:ReadFPGA 0x0020!!!\n");
            input_ok = 0;
        }
    }
    if((NULL == w_str)&&(NULL == r_str))
    {
        return -1;
    
    }

    if(!input_ok)
    {
        return -1;
    }
    
    return 0;
}

uint32_t ParseAargs(uint8_t *input, Parmeters_t *param)
{
	uint32_t argc = 0;
	uint32_t len;

	len = strlen(input);

	for(;len;)
	{
		/* skip the leading white space */
		while(len-- && *input++ == ' ');
		/* point to the beginning of the parameter */
		input -= 1;
		len += 1;
		param->argv[argc] = input;

		/* skip the none white space */
		while(len-- &&(*input++ != ' '));

		param->len[argc] = (unsigned int)input - (unsigned int)(param->argv[argc])-1;

		/* input already point to the white space + 1 */
		*(input-1) = 0; /* change to white space to NULL */

		if(++argc >= 5) /* maxium 5 args are supported */
		{
			break;
		}
	}

	return argc;
}

void Help(char *input)
{
	int i;

	dbg_printf("\r\nSupport Command:\r\n");

	for(i=0; i<(sizeof(CmdList)/sizeof(CmdList[0])); i++)
	{
		dbg_printf(CmdList[i].name);
		dbg_printf("\r\n");
	}
}

void SpiFlashHandler(char *input)
{
	dbg_printf("\r\nSpiFlash...\r\n");
	if(strstr(input, "Test"))
		eSPIFLASH_Request(eSPIFLASH_Request_Test);
	else if(strstr(input, "GetId"))
		eSPIFLASH_Request(eSPIFLASH_Request_ReadID);
    else
    {
        dbg_printf("\r\nSpiFlash <Test> <GetId>\r\n");
    }
}

void McuReset(char *input)
{
  Parmeters_t args;
  uint32_t argc;
  argc = ParseAargs(input, &args);

  if (strncmp(args.argv[1], "-y", 2) == 0)
  {
	/* Generate a software reset */
	NVIC_SystemReset();
	while (1)
	  ;
  }
}

void McuUpgrade(char *input)
{
  FLASH_If_Init();
  /* erase information area */
  FLASH_EraseSector(FLASH_Sector_11, VoltageRange_3);

  McuReset("Reset -y");
}

void GpsCmdHandler(char *input)
{
    
    Parmeters_t args;
	uint32_t argc;
	uint32_t value = 0;
	uint32_t cmd = 0;
    uint8_t class;
    uint32_t satellite_Mode = 0;
    uint32_t power_state = 0;
    uint32_t start_option = 0;
    uint32_t cfg_option = 0;

	argc = ParseAargs(input, &args);
    
    vTaskDelay(10); 
	if(argc)
	{
#if 0
        for(int i = 0; i < 5; i++)
        {
            sprintf(Task5DebugBuffer, "\r\nargs.argv[%d]:%s\r\n", i, args.argv[i]);
            dbg_printf(Task5DebugBuffer);
        
        }
         
        sprintf(Task5DebugBuffer, "\r\nstrcmp:%d\r\n", strncmp(args.argv[2], "GNSS", 4));
        dbg_printf(Task5DebugBuffer);
#endif

		if(!strncmp(args.argv[1], "MON", 3))
		{
            class = UBX_CLASS_MON;
            if(!strncmp(args.argv[2], "GNSS", 4))
            {
                cmd = UBX_MON_GNSS;
            }
            else if(!strncmp(args.argv[2], "HW", 2))
            {
                cmd = UBX_MON_HW;
            }
            else if(!strncmp(args.argv[2], "VER", 3))
            {
                cmd = UBX_MON_VER;
            }
            ubx_cmd(cmd, class);
            
        }
        else if(!strncmp(args.argv[1], "NAV", 3))
        {
            class = UBX_CLASS_NAV;
            if(!strncmp(args.argv[2], "CLOCK", 5))
            {
                cmd = UBX_NAV_CLOCK;
            }
            else if(!strncmp(args.argv[2], "PVT", 3))
            {
                cmd = UBX_NAV_PVT;
            }
            else if(!strncmp(args.argv[2], "SAT", 3))
            {
                cmd = UBX_NAV_SAT;
            }
            else if(!strncmp(args.argv[2], "STATUS", 6))
            {
                cmd = UBX_NAV_STATUS;
            }
            ubx_cmd(cmd, class);
        }
        else if(!strncmp(args.argv[1], "NEMA_MSG", 8))
        {
            class = UBX_CLASS_CFG;
            if(!strncmp(args.argv[2], "DISABLE", 7))
            {
                cmd = UBX_CFG_PRT;
            }
            ubx_cmd(cmd, class);
        }
        else if(!strncmp(args.argv[1], "CFG_CFG", 7))
		{
            if(!strncmp(args.argv[2], "CUR_CFG", 7))
            {
                cfg_option = CURRENT_CONFIG;
            }
            else if(!strncmp(args.argv[2], "DEF_CFG", 7))
            {
                cfg_option = DEFAULT_CONFIG;
            }
            else
            {
                cfg_option = INPUT_ERROR;
            }
            
            if(cfg_option != INPUT_ERROR)
            {
                Gps_Cfg_CFG(cfg_option);
            }
            else
            {
                dbg_printf("\r\nCFG CFG_FLASH ERRO\r\n");
            }
        }
        else if(!strncmp(args.argv[1], "GNSS_MODE", 9))
		{
            if(!strncmp(args.argv[2], "GPS_MODE", 8))
            {
                satellite_Mode = GPS_MODE;
            }
            else if(!strncmp(args.argv[2], "BeiDou_MODE", 11))
            {
                satellite_Mode = BeiDou_MODE;
            }
            else if(!strncmp(args.argv[2], "GPS_BeiDou_MODE", 15))
            {
                satellite_Mode = GPS_BeiDou_MODE;
            }
            else
            {
                 satellite_Mode = INPUT_ERROR;
            }
            if(satellite_Mode != INPUT_ERROR)
            {
                Gps_Cfg_GNSS(satellite_Mode);
            }
            else
            {
                dbg_printf("\r\nCFG GNSS_MODE ERRO\r\n");
            }
		}
        else if(!strncmp(args.argv[1], "PWR", 3))
        {
            if(!strncmp(args.argv[2], "RUNNING", 7))
            {
                power_state = GNSS_RUNNING;
            }
            else if(!strncmp(args.argv[2], "STOPPED", 7))
            {
                power_state = GNSS_STOPPED;
            }
            else if(!strncmp(args.argv[2], "BACKUP", 7))
            {
                power_state = SOFT_BACK_UP;
            }
            else
            {
                power_state = INPUT_ERROR;
            }
            
            if(power_state != INPUT_ERROR)
            {
                Gps_Cfg_PWR(power_state);
            }
            else
            {
                dbg_printf("\r\nCFG POWER STATE ERRO\r\n");
            }

        }
        else if(!strncmp(args.argv[1], "RST", 3))
        {
            if(!strncmp(args.argv[2], "HOTSTART", 8))
            {
                start_option = HOTSTART;
            }
            else if(!strncmp(args.argv[2], "WARMSTART", 9))
            {
                start_option = WARMSTART;
            }
            else if(!strncmp(args.argv[2], "COLDSTART", 9))
            {
                start_option = COLDSTART;
            }
            else
            {
                start_option = INPUT_ERROR;
            }
            
            if(start_option != INPUT_ERROR)
            {
                Gps_Cfg_RST(start_option);
            }
            else
            {
                dbg_printf("\r\nCFG RST START OPTION ERRO\r\n");
            }

        }
        else
        {
            dbg_printf("\r\nGPSCMD <MON GNSS> or GPSCMD <MON HW> or GPSCMD <MON VER>\r\n");
            dbg_printf("\r\nGPSCMD<NAV CLOCK>,GPSCMD<NAV PVT>, GPSCMD<NAV SAT>, GPSCMD<NAV STATUS>, \r\n");
            dbg_printf("\r\nGPSCMD<NEMA_MSG DISABLE> \r\n");
            dbg_printf("\r\nGPSCMD<PWR RUNNING> or GPSCMD <PWR STOPPED> or GPSCMD<PWR BACKUP>  GPSCMD<RST HOTSTART> GPSCMD<RST WARMSTART>  GPSCMD<RST COLDSTART>\r\n");
            dbg_printf("\r\nGPSCMD<GNSS_MODE <GPS_MODE or BeiDou_MODE or GPS_BeiDou_MODE> >\r\n");
            dbg_printf("\r\nGPSCMD<CFG_CFG DEF_CFG> GPSCMD<CFG_CFG CUR_CFG> \r\n");
        
        }
    }
}

void CM55MsgHandler(char *input)
{
    dbg_printf("\r\nCM55 Msg...\r\n");
    if(strstr(input, "cPHDiff"))
    {    
        CM55_Request(CM55_Request_cPHDiff);
    }
    else
	{
		dbg_printf("\r\nGetCM55Msg <cPHDiff>\r\n");
	}
}

/*******************************************************************************
 * name: PCA9555Handler
 * role: command line for the PCA9555 Extended IO
 * 		pca9555 <cfg>	<16bit configuration data/one bit per PIN, MSB>
 * 		pca9555 <out>	<16bit one bit per PIN, MSB>
 * 		pca9555 <in>
*******************************************************************************/
void PCA9555Handler(char *input)
{
	Parmeters_t args;
	uint32_t argc;
	uint32_t value;


	argc = ParseAargs(input, &args);

	if(argc)
	{
		if(!strcmp(args.argv[1], "cfg"))
		{
			value = (uint32_t)Htoi(args.argv[2]);
			PCA9555_InputOutConfig(value);
		}
		else if(!strcmp(args.argv[1], "out"))
		{
			value = (uint32_t)Htoi(args.argv[2]);
			PCA9555_Output(value);

		}
		else if(!strcmp(args.argv[1], "in"))
		{
			PCA9555_Input();
		}
		else
		{
			dbg_printf("\r\npca9555 <cfg/out/in> [data]\r\n");
		}
	}
}

void AT24C02Handler(char *input)
{
	Parmeters_t args;
	uint32_t argc;
	uint16_t value;

	argc = ParseAargs(input, &args);

	if(argc)
	{
		if(!strcmp(args.argv[1], "test"))
		{
			value = (uint16_t)Htoi(args.argv[2]);
			AT24C02_TestRequest((uint8_t)value);
		}
		else
		{
			dbg_printf("\r\nAT24C02 <Test>\r\n");
		}
	}
	else
	{
		dbg_printf("\r\nAT24C02 <Test>\r\n");
	}

}

void AT24C64Handler(char *input)
{
	Parmeters_t args;
	uint32_t argc;
	uint16_t value;

	argc = ParseAargs(input, &args);

	if(argc)
	{
		if(!strcmp(args.argv[1], "test"))
		{
			value = (uint16_t)Htoi(args.argv[2]);
			AT24C64_TestRequest(value);
		}
		else
		{
			dbg_printf("\r\nAT24C64 <Test>\r\n");
		}
	}
	else
	{
		dbg_printf("\r\nAT24C64 <Test>\r\n");
	}

}

void LM92Handler(char *input)
{
	Parmeters_t args;
	uint32_t argc;
	uint16_t value;

	argc = ParseAargs(input, &args);
    
    if(argc)
	{
		if(!strcmp(args.argv[1], "GetLM92Tem"))
		{
			LM92_GetTemp_Request();
		}
		else if(!strcmp(args.argv[1], "GetLM92ID"))
		{
			LM92_GetId_Request();
        }
        else
	    {
		    dbg_printf("\r\nLM92 <GetLM92Tem> <GetLM92ID>\r\n");

	    }
	}
	else
	{
		dbg_printf("\r\nLM92 <GetLM92Tem> <GetLM92ID>\r\n");
	}

}

void INA220Handler(char *input)
{
	Parmeters_t args;
	uint32_t argc;
	uint16_t value;

	argc = ParseAargs(input, &args);

    if(argc)
	{
		if(!strcmp(args.argv[1], "GetCPUCurrent"))
		{
			INA220_GetMsg_Request();
		}
		else
		{
			dbg_printf("\r\nINA220 <GetCPUCurrent>\r\n");
		}
	}
	else
	{
		dbg_printf("\r\nINA220 <GetCPUCurrent>\r\n");
	}

}

void AD9548Handler(char *input)
{
	Parmeters_t args;
	uint32_t argc;
	uint16_t value;

	argc = ParseAargs(input, &args);

	if(argc)
	{

		if(!strcmp(args.argv[1], "getid"))
		{
		    AD9548_GetId_Request();
		}
        else if(!strcmp(args.argv[1], "readdiv"))
		{
			AD9548_Read_Divider_Request();
		}
        else if(!strcmp(args.argv[1], "writediv"))
		{
			AD9548_Write_Divider_Request();
		}
        else if(!strcmp(args.argv[1], "readsycs"))
		{
			AD9548_Read_Sysclk_Status_Request();
		}
        else if(!strcmp(args.argv[1], "readDPLLs"))
		{
			AD9548_Read_DPLL_Status_Request();
		}
        else if(!strcmp(args.argv[1], "readInput_Refs"))
		{
			AD9548_Read_Input_Ref_Status_Request();
		}
        else if(!strcmp(args.argv[1], "cfg122_88Mhz"))
		{
			AD9548_Cfg_122_88Mhz_Request();
		}


		else
		{
		    dbg_printf("\r\nAD9548 <getid> <readdiv> <writediv> <readsycs> <cfg122_88Mhz>\r\n");
		}
	}
	else
	{
		dbg_printf("\r\nAD9548 <getid> <readdiv> <writediv> <readsycs> <cfg122_88Mhz>\r\n");
	}

}

void ADT7461Handler(char *input)
{
	Parmeters_t args;
	uint32_t argc;
	uint16_t value;
    int8_t integer;
    uint8_t decimal;

	argc = ParseAargs(input, &args);

	if(argc)
	{
        if(!strcmp(args.argv[1], "Get7461CPUID"))
		{
			ADT7461_Get_Id_Request();
		}
        else if(!strcmp(args.argv[1], "Get7461CPUTEM"))
        {
            ADT7461_Get_Local_Temp_Request();
        }
        else if(!strcmp(args.argv[1], "Getextemphigh"))
        {
            ADT7461_Get_Extemp_High_Byte_Request();
        }
        else if(!strcmp(args.argv[1], "Getextemplow"))
        {
            ADT7461_Get_Extemp_Low_Byte_Request();
        }
        else if(!strcmp(args.argv[1], "Getexoffhighbyte"))
        {
            ADT7461_Get_Ex_Offset_H_Byte_Request();
        }
        else if(!strcmp(args.argv[1], "Getexofflowbyte"))
        {
            ADT7461_Get_Ex_Offset_L_Byte_Request();
        }
        else if(!strcmp(args.argv[1], "Setoffset"))
		{
			integer = (int8_t)Htoi(args.argv[2]);
            decimal = (uint8_t)Htoi(args.argv[3]); 
            ADT7461_Set_Ex_Offset_Request(integer, decimal);
		}
		else
		{
			dbg_printf("\r\nADT7621 <Get7461CPUID> <Get7461CPUTEM> <Getextemphigh> <Getextemplow> <Getexoffhighbyte> <Getexofflowbyte> <Setoffset integer decimal>\r\n");
		}
	}
	else
	{
		dbg_printf("\r\nADT7621 <Get7461CPUID> <Get7461CPUTEM> <Getextemphigh> <Getextemplow> <Getexoffhighbyte> <Getexofflowbyte>  <Setoffset integer decimal>\r\n");
    }

}


void GetVersion(char *input)
{
	dbg_printf("\r\nMCU SW 1.0\r\n");
}

void PowerHandler(char *input)
{
	Parmeters_t args;
	uint32_t argc;
	uint32_t value;

	argc = ParseAargs(input, &args);
	if (argc)
	{
		if (!strcmp(args.argv[1], "program"))
		{
			dpm_setflag(DPM_FLAG3);
			dpm_setflag(DPM_FLAG4);
		}
		else if (!strcmp(args.argv[1], "enable"))
		{
			dpm_setflag(DPM_FLAG2);
		}
		else if (!strcmp(args.argv[1], "disable"))
		{
			dpm_setflag(DPM_FLAG9);
		}
		else if (!strcmp(args.argv[1], "status"))
		{
			dpm_setflag(DPM_FLAG6);
		}
		else if(!strcmp(args.argv[1], "set"))
		{
			value = (uint32_t)Htoi(args.argv[2]);
			if(value > 15000)
            {
				dbg_printf("DM7304 set voltage too high!\r\n");
            }
            else
            {
                if(dpm_pol_set_voltage(value))

			    {
				    dbg_printf("DM7304 set voltage failed!\r\n");

			    }
                else

			    {
				    dbg_printf("DM7304 set voltage successed!\r\n");

			    }
            }
		}
		else
		{
			dbg_printf("\r\nPower <program> <enable> <disable> <set value>\r\n");
		    dbg_printf("\r\nvalue(hex) should less than 15000\r\n");
        }
	}
	else
	{
		dbg_printf("\r\nPower <program> <enable> <disable> <set value>\r\n");
	    dbg_printf("\r\nvalue(hex) should less than 15000\r\n");
    }
}

void WriteFPGAHandler(char *input)                  //input is from uarttest
{
    int data[2], result;                            //data[0] for addr , data[1] for data;
     
    result = handle_str(input, data);
    
    if(result == 0)
    { 
        FGPA_DATA_t reg;                                //save arry data into struct FPGA_DATA_t 
        reg.reg_addr = (uint16_t)data[0] ;
        reg.data = (uint16_t)data[1];
        reg.wr = 1;

        sprintf(Task5DebugBuffer, "\r\naddr:%x\r\n", reg.reg_addr);
        dbg_printf(Task5DebugBuffer);

        sprintf(Task5DebugBuffer, "\r\ndata:%x\r\n", reg.data);
        dbg_printf(Task5DebugBuffer);
 
        Fpga_ContrlRequest(reg);                        //request to contol FPGA
    }
}

void ReadFPGAHandler(char *input)
{

    int data[2];                                    //data[0] for addr , data[1] for data;
    int result = 0;
    
    result = handle_str(input, data);
    
    if(result == 0)
    {
        FGPA_DATA_t reg;     
    
        reg.reg_addr = (uint16_t)data[0];
        reg.data = 0x0000;                              //dafault data[1] is 0x0000;
        reg.wr = 0;
   
        sprintf(Task5DebugBuffer, "\r\naddr:%x\r\n", reg.reg_addr);
        dbg_printf(Task5DebugBuffer);

        sprintf(Task5DebugBuffer, "\r\ndata:%x\r\n", reg.data);
        dbg_printf(Task5DebugBuffer);

        Fpga_ContrlRequest(reg); 
    } 
}

void ISR_UART4(void)
{
	/* USART in Receiver mode */
	if (USART_GetITStatus(UART4, USART_IT_RXNE) == SET)
	{
		/* Receive Transaction data */
		aRxBuffer[ubRxIndex] = USART_ReceiveData(UART4);

		if((aRxBuffer[ubRxIndex] == '\r') || (aRxBuffer[ubRxIndex] == '\n'))
		{
			UartFlag |= UART_NEW_LINE_FLAG;
		}

		/* echo the receive char which can see */
//		if (aRxBuffer[ubRxIndex] >= 0x20 && aRxBuffer[ubRxIndex] <= 0x7e)
		{
			UartFlag |= UART_ECHO_FLAG;
			USART_SendData(UART4, aRxBuffer[ubRxIndex]);
		}
		if (ubRxIndex < BUFFERSIZE)
		{
			ubRxIndex++;
		}
	}

	/* USART in Transmitter mode */
	if (USART_GetITStatus(UART4, USART_IT_TC) == SET)
	{
		if(UartFlag & UART_ECHO_FLAG)
		{
			/* complete the received char */
			UartFlag &= ~UART_ECHO_FLAG;
			USART_ClearFlag(UART4, USART_FLAG_TC);
		}
		else if (ubTxIndex < TxLen)
		{
			/* Send Next Transaction data */
			USART_SendData(UART4, aTxBuffer[ubTxIndex++]);
		}
		else
		{
			/* All transmited, Disable the Tx buffer empty interrupt */
			ubTxIndex = 0;
			TxLen = 0;
			UartFlag &= ~ UART_TRANSMITING_FLAG;
			USART_ClearFlag(UART4, USART_FLAG_TC);
		}
	}
}

void Task5(void* p)
{
	while (1)
	{
#if 0
		int i = 0;
		if(i++>50)
		{
			i = 0;
			dbg_printf("Test string1\r\n");
			dbg_printf("Test string2\r\n");
		}
#endif

		switch(state)
		{
		case Idle:

			if(!(UartFlag & UART_TRANSMITING_FLAG))/* nothing sending */
			{
				USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
			}
			/* Get new input CMD line */
			if(UartFlag & UART_NEW_LINE_FLAG)
			{
				UartFlag &= ~UART_NEW_LINE_FLAG;/* reset flag */

				USART_ITConfig(UART4, USART_IT_RXNE, DISABLE);

				memcpy(InputStr, aRxBuffer, ubRxIndex);/* copy data to cmd buffer */
				InputStr[ubRxIndex+1] = 0;
				ubRxIndex = 0;/* Reset the Received Index */

				USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);/* re-enable RXNE interrupt */
				state = TranslateInput;
			}
			else if (PrintLen)
			{
				dbg_printf("\n\r#");
				memcpy(aTxBuffer, PrintBuff, PrintLen);
				TxLen = PrintLen;
				PrintLen = 0;

				state = Output;
			}
			break;

		case TranslateInput:
		{
			int i;
			char *str;
			int found = 0;
			for(i=0; i<(sizeof(CmdList)/sizeof(CmdList[0])); i++)
			{
				if(str=strstr(InputStr, CmdList[i].name))
				{
					CmdList[i].resp(str);
					dbg_printf("\r\n#");
					found = 1;
					break;
				}
			}

			if (!found)
			{
				Help(str);
			//	strcat(&aTxBuffer[TxLen], "\n\r#");
			//	TxLen += 3;
			//	UartFlag |= UART_TRANSMITING_FLAG;
			//	state = Output;
			}
			state = Idle;
		}

			break;
		case Output:
			USART_ITConfig(UART4, USART_IT_RXNE, DISABLE);
			USART_SendData(UART4, aTxBuffer[ubTxIndex++]);

			state = Idle;
			break;
		}


		vTaskDelay(TASK5_PERIOD); /* 20ms */
	}

	vTaskDelete(NULL);
}
