/* this task is the driver for the lmk04031 */

#include "multi-task.h"
#include "FreeRTOS.h"
#include "task7.h"
#include "stm32f4xx_usart.h"
#include "debug.h"
#include "string.h"
#include <stdarg.h>
#include "task5.h"

#define SEMAPHORE_TEST

#ifdef SEMAPHORE_TEST
#include "semphr.h"
static SemaphoreHandle_t UartxSemaphore;
#endif /*SEMAPHORE_TEST*/

#define TASK7_DEBUG
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

typedef void CmdResponse(char *str);

typedef struct
{
	char *name;
	CmdResponse *resp;
}Command_List_t;

StackType_t Task7Stack[TASK7_STACK_SIZE] CCM_RAM;  // Put task stack in CCM
StaticTask_t Task7Buffer CCM_RAM;  // Put TCB in CCM

static uint32_t UartFlag =0;
static uint32_t ubRxIndex=0;
static uint32_t ubTxIndex=0;
static uint32_t EchoRIndex=0;
static uint32_t EchoTIndex=0;
static uint32_t sign_flag = 1;

uint32_t    gps_cmd_flag = 0;
uint8_t msgbuf[6];
uint8_t ReceiveState = 0;
uint8_t cm55_rcv_flag = 0;

char sw_ver[30];     /* sw version string buffer */
char hw_ver[10];     /* hw version string buffer */
char ex_ver[256];     /* hw version string buffer */
char cm55_str[118];     /* hw version string buffer */
McuUart3Mux_t Uart3Route = McuUart3Mux_None;

static uint32_t TxLen = 0;
static uint32_t PrintLen= 0;
static uint32_t RxLen = 0;
static uint32_t InputLen= 0;
static uint8_t aRxBuffer[BUFFERSIZE];
static uint8_t aTxBuffer[BUFFERSIZE];
static uint8_t  InputStr[BUFFERSIZE];
static uint8_t PrintBuff[BUFFERSIZE];

static uint8_t GpsDebugBuffer[BUFFERSIZE];
static uint8_t CM55DebugBuffer[BUFFERSIZE];


static Task_State_t state=Idle;
CM55_INFO   CM55;


static CM55_Request_t CM55_Req;
static void GetCM55cPHDiff(CM55_INFO *CM55);

static uint8_t GetComma(uint8_t num,char* str);
static int Get_Int_Number(char *s);
static double Get_Double_Number(char *s);
static float Get_Float_Number(char *s);
static void UTC2BTC(DATE_TIME *GPS);
static char* Get_String(char *s);
static int USART_SendString(USART_TypeDef* USARTx, uint8_t* pBuffer, uint16_t len);




uint8_t CFG_GNSS_GPS[] = 
{
    0xB5,0x62,0x06,0x3E,0x34,0x00,0x00,0x00,0x20,0x06,0x00,0x08,0x10,0x00,0x01,0x00,0x01,0x01,
    0x01,0x01,0x03,0x00,0x00,0x00,0x01,0x01,0x03,0x08,0x10,0x00,0x00,0x00,0x01,0x01,0x04,0x00,
    0x08,0x00,0x00,0x00,0x01,0x01,0x05,0x00,0x03,0x00,0x00,0x00,0x01,0x01,0x06,0x08,0x0E,0x00,
    0x00,0x00,0x01,0x01,0x13,0xC5
};

uint8_t CFG_GNSS_BeiDou[] = 
{
    0xB5,0x62,0x06,0x3E,0x34,0x00,0x00,0x00,0x20,0x06,0x00,0x08,0x10,0x00,0x00,0x00,0x01,0x01,
    0x01,0x01,0x03,0x00,0x00,0x00,0x01,0x01,0x03,0x08,0x10,0x00,0x01,0x00,0x01,0x01,0x04,0x00,
    0x08,0x00,0x00,0x00,0x01,0x01,0x05,0x00,0x03,0x00,0x00,0x00,0x01,0x01,0x06,0x08,0x0E,0x00,
    0x00,0x00,0x01,0x01,0x13,0xB5
};

uint8_t CFG_GNSS_GPSandBeiDou[] = 
{
    0xB5,0x62,0x06,0x3E,0x34,0x00,0x00,0x00,0x20,0x06,0x00,0x08,0x10,0x00,0x01,0x00,0x01,0x01,
    0x01,0x01,0x03,0x00,0x00,0x00,0x01,0x01,0x03,0x08,0x10,0x00,0x01,0x00,0x01,0x01,0x04,0x00,
    0x08,0x00,0x00,0x00,0x01,0x01,0x05,0x00,0x03,0x00,0x00,0x00,0x01,0x01,0x06,0x08,0x0E,0x00,
    0x00,0x00,0x01,0x01,0x14,0xE1
};

uint8_t CFG_PWR_RUNNING[] = 
{
    0xB5,0x62,0x06,0x57,0x08,0x00,0x01,0x00,0x00,0x00,0x20,0x4E,0x55,0x52,0x7B,0xC3
};

uint8_t CFG_PWR_STOPPED[] = 
{
    0xB5,0x62,0x06,0x57,0x08,0x00,0x01,0x00,0x00,0x00,0x50,0x4F,0x54,0x53,0xAC,0x85
};

uint8_t CFG_PWR_BACK_UP[] = 
{
    0xB5,0x62,0x06,0x57,0x08,0x00,0x01,0x00,0x00,0x00,0x50,0x4B,0x43,0x42,0x86,0x46
};

uint8_t CFG_RST_HOTSTART[] = 
{
    0xB5,0x62,0x06,0x04,0x04,0x00,0x00,0x00,0x02,0x00,0x10,0x68
};

uint8_t CFG_RST_WARMSTART[] = 
{
    0xB5,0x62,0x06,0x04,0x04,0x00,0x01,0x00,0x02,0x00,0x11,0x6C
};

uint8_t CFG_RST_COLDSTART[] = 
{
    0xB5,0x62,0x06,0x04,0x04,0x00,0xFF,0xB9,0x02,0x00,0xC8,0x8F
};


uint8_t CFG_DEF_CFG[] =
{
    0xB5,0x62,0x06,0x09,0x0D,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,
    0x02,0x1A,0x99
};

uint8_t CFG_CUR_CFG[] =
{
    0xB5,0x62,0x06,0x09,0x0D,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,
    0x02,0x1C,0xAA
};


uint8_t CFG_PRT_UBX[] = 
{
    0xB5,0x62,0x06,0x00,0x14,0x00,0x01,0x00,0x00,0x00,0xD0,0x08,0x00,0x00,0x80,0x25,0x00,0x00,
    0x01,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x9A,0x79
};


// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = / /
// function: int CM55_Parse (char * line, CM55_INFO * CM55)
// function: parsing CM55 information
// parameters: an array to store the original information, resolves to identifiable structure
// the return value:
// 1: CM55 parsing
// 0: no parsing, or data is invalid
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = / /
int CM55_Parse(char *line, CM55_INFO *CM55)
{
    int def_num;
    char *buf = line;

    char *str =  &buf[GetComma(19, buf)];
    def_num = ((*(str + 11) - '0') * 10 + (*(str + 12) - '0'));
    
//    dbg_printf((char *)(InputStr));
    if(def_num == 55)
    {
        CM55 -> Msg_NO = Get_Int_Number(&buf[GetComma(1, buf)]);
        CM55 -> TxRxFlag = Get_Int_Number(&buf[GetComma(2, buf)]);
        CM55 -> CStatus = buf[GetComma(3, buf)];
        CM55 -> TrackStatus = buf[GetComma(4, buf)];
        CM55 -> cPHDiff = Get_Int_Number(&buf[GetComma(5, buf)]);
        CM55 -> cPWM1 = Get_Double_Number(&buf[GetComma(6, buf)]);
        CM55 -> cPWM2 = Get_Double_Number(&buf[GetComma(7, buf)]);
        CM55 -> SYNCNT = Get_Int_Number(&buf[GetComma(8, buf)]);
        CM55 -> HCNT = Get_Int_Number(&buf[GetComma(9, buf)]);
        CM55 -> HPAVG = Get_Double_Number(&buf[GetComma(10, buf)]);
        CM55 -> VCH1 = Get_Double_Number(&buf[GetComma(11, buf)]);
        CM55 -> HPMOD = Get_Double_Number(&buf[GetComma(12, buf)]);
        CM55 -> VCM10 = Get_Double_Number(&buf[GetComma(13, buf)]);
        CM55 -> inT = Get_Double_Number(&buf[GetComma(15, buf)]);
        CM55 -> TcPHDiff = Get_Int_Number(&buf[GetComma(16, buf)]);
        CM55 -> Version = Get_Float_Number(&buf[GetComma(18, buf)]);
        
	    CM55 -> date.year     = ((*(str + 0) - '0') * 1000 + (*(str + 1) - '0') *100 + (*(str + 2) - '0') * 10 + (*(str + 3) - '0')); 
	    CM55 -> date.month   = ((*(str + 5) - '0') * 10 + (*(str + 6) - '0'));
	    CM55 -> date.day    = ((*(str + 8) - '0') * 10 + (*(str + 9) - '0'));
        CM55 -> def_val = def_num;

        return 1;
    }
     
    return 0;
}

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = / /
// function: static int Str_To_Int (char * buf)
// function: to split a string into an integer
// parameters: the string
// the return value: integer after conversion
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = / /
static int Str_To_Int(char *buf)
{
	int rev = 0;
	int dat;
	char *str = buf;
	while(*str != '\0')
	{
		switch(*str)
		{
			case '0':
				dat = 0;
				break;
			case '1':
				dat = 1;
				break;
			case '2':
				dat = 2;
				break;		
			case '3':
				dat = 3;
				break;
			case '4':
				dat = 4;
				break;
			case '5':
				dat = 5;
				break;
			case '6':
				dat = 6;
				break;
			case '7':
				dat = 7;
				break;
			case '8':
				dat = 8;
				break;
			case '9':
				dat = 9;
				break;
		}

		rev = rev * 10 + dat;
		str ++;
	}

	return rev;
}

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = / /
// function: static int Get_Int_Number (char * s)
// function: the given string before the first comma character is converted to an integer
// parameters: the string
// the return value: integer after conversion
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = / /
static int Get_Int_Number(char *s)
{
	char buf[10];
	uint8_t i;
	int temp, rev;
	i = GetComma(1, s);
	i = i - 1;
	strncpy(buf, s, i);
	buf[i] = 0;

    if(buf[0] == '-')
    {
        sign_flag = 0;
        strncpy(buf, s+1, i-1);
        buf[i-1] = 0;
    }

    if(sign_flag)
    {
	    rev = Str_To_Int(buf);
    }
    else
    {
	    temp = Str_To_Int(buf);
        rev = temp*(-1);
    }
    return rev;	
}
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = / /
// function: static float Str_To_Float (char * buf)
// function: convert a string to a floating-point number
// parameters: the string
// the return value: single precision value after conversion
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = / /
static float Str_To_Float(char *buf)
{
	float rev = 0;
	float dat;
	int integer = 1;
	char *str = buf;
	int i;
	while(*str != '\0')
	{
		switch(*str)
		{
			case '0':
				dat = 0;
				break;
			case '1':
				dat = 1;
				break;
			case '2':
				dat = 2;
				break;		
			case '3':
				dat = 3;
				break;
			case '4':
				dat = 4;
				break;
			case '5':
				dat = 5;
				break;
			case '6':
				dat = 6;
				break;
			case '7':
				dat = 7;
				break;
			case '8':
				dat = 8;
				break;
			case '9':
				dat = 9;
				break;
			case '.':
				dat = '.';
				break;
		}
		if(dat == '.')
		{
			integer = 0;
			i = 1;
			str ++;
			continue;
		}
		if( integer == 1 )
		{
			rev = rev * 10 + dat;
		}
		else
		{
			rev = rev + dat / (10 * i);
			i = i * 10 ;
		}
		str ++;
	}
	return rev;

}

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = / /
// function: static float Get_Float_Number (char * s)
// function: the given string after the first comma string into single precision
// parameters: the string
// the return value: single precision after conversion
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = / /											
static float Get_Float_Number(char *s)
{
	char buf[10];
	uint8_t i;
	float rev;
	i = GetComma(1, s);
	i = i - 1;
	strncpy(buf, s, i);
	buf[i] = 0;
	rev = Str_To_Float(buf);
	return rev;	
}

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = / /
// function: static double Str_To_Double (char * buf)
// function converts a string into double-precision floating-point number
// parameters: the string
// the return value: the converted double-precision floating-point number
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = / /
static double Str_To_Double(char *buf)
{
	double rev = 0;
	double dat;
	int integer = 1;
	char *str = buf;
	int i;
	while(*str != '\0')
	{
		switch(*str)
		{
			case '0':
				dat = 0;
				break;
			case '1':
				dat = 1;
				break;
			case '2':
				dat = 2;
				break;		
			case '3':
				dat = 3;
				break;
			case '4':
				dat = 4;
				break;
			case '5':
				dat = 5;
				break;
			case '6':
				dat = 6;
				break;
			case '7':
				dat = 7;
				break;
			case '8':
				dat = 8;
				break;
			case '9':
				dat = 9;
				break;
			case '.':
				dat = '.';
				break;
		}
		if(dat == '.')
		{
			integer = 0;
			i = 1;
			str ++;
			continue;
		}
		if( integer == 1 )
		{
			rev = rev * 10 + dat;
		}
		else
		{
			rev = rev + dat / (10 * i);
			i = i * 10 ;
		}
		str ++;
	}
	return rev;
}

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = / /
// function: static double Get_Double_Number (char * s)
// function: the given string after the first comma string into a double precision
// parameters: the string
// the return value: double after conversion
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = / /
static double Get_Double_Number(char *s)
{
	char buf[10];
	uint8_t i, tmp;
	double rev;
	i = GetComma(1, s);
	i = i - 1;
	strncpy(buf, s, i);
	buf[i] = 0;

    if(buf[0] == '-')
    {
        sign_flag = 0;
        strncpy(buf, s+1, i-1);
        buf[i-1] = 0;
    }
    if(sign_flag)
    {
	    rev = Str_To_Double(buf);
    }
    else
    {
	    tmp = Str_To_Double(buf);
        rev = tmp*(-1);
    }

	return rev;	
}

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = / /
// function: static uchar GetComma (uchar num, char * STR)
// function: calculation string in the position of the comma
// parameters: find a comma is the number of which one, of the string
// return values: 0
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = / /

static uint8_t GetComma(uint8_t num,char *str)
{
	uint8_t i,j = 0;
	int len = strlen(str);

	for(i = 0;i < len;i ++)
	{
		if(str[i] == ',')
			j++;
		if(j == num)
			return i + 1;	
	}

	return 0;	
}

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = / /
// function: void UTC2BTC DATE_TIME * (GPS)
// function: conversion time is Beijing time zone
// parameters: the structure of storage time
// the return value: no
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = / /
static void UTC2BTC(DATE_TIME *GPS)
{
	GPS->second ++;  
	if(GPS->second > 59)
	{
		GPS->second = 0;
		GPS->minute ++;
		if(GPS->minute > 59)
		{
			GPS->minute = 0;
			GPS->hour ++;
		}
	}	

    GPS->hour = GPS->hour + 8;
	if(GPS->hour > 23)
	{
		GPS->hour -= 24;
		GPS->day += 1;
		if(GPS->month == 2 ||
		   		GPS->month == 4 ||
		   		GPS->month == 6 ||
		   		GPS->month == 9 ||
		   		GPS->month == 11 )
		{
			if(GPS->day > 30)
			{
		   		GPS->day = 1;
				GPS->month++;
			}
		}
		else
		{
			if(GPS->day > 31)
			{	
		   		GPS->day = 1;
				GPS->month ++;
			}
		}
		if(GPS->year % 4 == 0 )
		{
	   		if(GPS->day > 29 && GPS->month == 2)
			{		
	   			GPS->day = 1;
				GPS->month ++;
			}
		}
		else
		{
	   		if(GPS->day > 28 &&GPS->month == 2)
			{
	   			GPS->day = 1;
				GPS->month ++;
			}
		}
		if(GPS->month > 12)
		{
			GPS->month -= 12;
			GPS->year ++;
		}		
	}
}

McuUart3Mux_t Fpga_CacheUart3Sel(McuUart3Mux_t route)
{
  Uart3Route = route;

  return Uart3Route;
}

/*******************************************************************************
 * @name: Fpga_GetMcuUart3Sel
 * @input: void
 * @output: the UART3 route
 * @role: read the sector information
 * 0. McuUart3Mux_None
 * 1. McuUart3Mux_GPS
 * 2. McuUart3Mux_TOD
 * 3. McuUart3Mux_CM55
 * 4. McuUart3Mux_ACS9522
*******************************************************************************/
McuUart3Mux_t Fpga_GetMcuUart3Sel(void)
{
    return Uart3Route;

}

void ISR_UART3(void)
{

    uint8_t Clear = Clear;

    Uart3Route = Fpga_GetMcuUart3Sel();
    
    /* USART in Receiver mode */
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
	    switch (Uart3Route)
	    {
	        case McuUart3Mux_CM55:
		    aRxBuffer[ubRxIndex] = USART3->DR;

		    if ((aRxBuffer[ubRxIndex - 1] == '\r')
			    && (aRxBuffer[ubRxIndex] == '\n'))
		    {
		        ReceiveState = 1;
		    }

		    if (ubRxIndex < sizeof(aRxBuffer)-1)
		    {
		        ubRxIndex++;
		    }
		    break;

	        case McuUart3Mux_GPS:
		        
            aRxBuffer[ubRxIndex] = USART3->DR;
		    if (ubRxIndex < sizeof(aRxBuffer)-1)
		    {
		        ubRxIndex++;
		    }
            break;
	        
            default:
		    Clear = USART3->DR;
		    break;
	    }
    }
    else if (USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)
    {
	    switch (Uart3Route)
	    {
	        case McuUart3Mux_CM55:
		    break;

	        case McuUart3Mux_GPS:
		    ReceiveState = 1;
		    break;

	        default:
		    break;
	    }
	    /* clear the pending bit */
	    Clear = USART3->SR;
	    Clear = USART3->DR;
    }
    else
    {
    
        /* nothing */
    }

}


static int USART_SendString(USART_TypeDef* USARTx, uint8_t* pBuffer, uint16_t len)
{
    uint32_t i = 0;
    for(i = 0; i < len; i++)
    {
        USART_SendData(USARTx, pBuffer[i]);
        while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
    }
    return i;
}

/**
 * Send config cfg cfg msg
 * UBX-CFG-CFG
 *
 * cfg_option: CUR_CFG, DEF_CFG;
 * 
 */
void Gps_Cfg_CFG(uint32_t cfg_option)
{
    switch(cfg_option)
    {
        case CURRENT_CONFIG:
            gps_cmd_flag = 1;
            
            dbg_printf("\r\ncfg_option: save current cfg to flash \r\n");

            USART_SendString(USART3, (uint8_t*)CFG_CUR_CFG, sizeof(CFG_CUR_CFG)/sizeof(CFG_CUR_CFG[0]));
        break;
        
        case DEFAULT_CONFIG:
            gps_cmd_flag = 1;
            
            dbg_printf("\r\ncfg_option:Revert default cfg from flash \r\n");

            USART_SendString(USART3, (uint8_t*)CFG_DEF_CFG, sizeof(CFG_DEF_CFG)/sizeof(CFG_DEF_CFG[0]));
        break;

        default:
            dbg_printf("\r\nCfg cfg erro!!!\r\n");
        break;
    }
}

/**
 * Send config gnss msg
 * UBX-CFG-GNSS
 *
 * satelite_mode: for GPS, BeiDou, GPSandBeiDou
 * 
 */
void Gps_Cfg_GNSS(uint32_t satellite_Mode)
{
    switch(satellite_Mode)
    {
        case GPS_MODE:
            gps_cmd_flag = 1;
            
            dbg_printf("\r\nsatelite_mode: GPS_MODE\r\n");

            USART_SendString(USART3, (uint8_t*)CFG_GNSS_GPS, sizeof(CFG_GNSS_GPS)/sizeof(CFG_GNSS_GPS[0]));
        break;
        
        case BeiDou_MODE:
            gps_cmd_flag = 1;
            
            dbg_printf("\r\nsatelite_mode: BeiDou_MODE\r\n");
            
            USART_SendString(USART3, (uint8_t*)CFG_GNSS_BeiDou, sizeof(CFG_GNSS_BeiDou)/sizeof(CFG_GNSS_BeiDou[0]));
        break;

        case GPS_BeiDou_MODE:
            gps_cmd_flag = 1;
            
            dbg_printf("\r\nsatelite_mode: GPS_BeiDou_MODE\r\n");

            USART_SendString(USART3, (uint8_t*)CFG_GNSS_GPSandBeiDou, sizeof(CFG_GNSS_GPSandBeiDou)/sizeof(CFG_GNSS_GPSandBeiDou[0]));
        break;

        default:
            dbg_printf("\r\ncfg_gnss_mode erro!!!\r\n");
        break;
    }
}

/**
 * Send config PWR msg
 * UBX-CFG-PWR
 *
 * power_state : PWR RUNNING state; PWR STOPPED state; PWR BACK_UP state;
 * 
 */
void Gps_Cfg_PWR(uint32_t power_state)
{

    switch(power_state)
    {
        case GNSS_RUNNING:
            gps_cmd_flag = 1;
            
            dbg_printf("\r\npower_state:POWER_RUNNING\r\n");

            USART_SendString(USART3, (uint8_t*)CFG_PWR_RUNNING, sizeof(CFG_PWR_RUNNING)/sizeof(CFG_PWR_RUNNING[0]));
        break;
        
        case GNSS_STOPPED:
            gps_cmd_flag = 1;
            
            dbg_printf("\r\npower_state:POWER_STOPPED\r\n");
            
            USART_SendString(USART3, (uint8_t*)CFG_PWR_STOPPED, sizeof(CFG_PWR_STOPPED)/sizeof(CFG_PWR_STOPPED[0]));
        break;

        case SOFT_BACK_UP:
            gps_cmd_flag = 1;
            
            dbg_printf("\r\npower_state:POWER_BACK_UP\r\n");
            
            USART_SendString(USART3, (uint8_t*)CFG_PWR_BACK_UP, sizeof(CFG_PWR_BACK_UP)/sizeof(CFG_PWR_BACK_UP[0]));
        break;

        default:
          dbg_printf("\r\ncfg_pwr_state erro!!!\r\n");
        break;
    }
}

/**
 * Send config RST startup option
 * UBX-CFG-RST
 *
 * startup option;HotStart, WarmStart, ColdStart, 
 * 
 */
void Gps_Cfg_RST(uint32_t start_option)
{

    switch(start_option)
    {
        case HOTSTART:
            gps_cmd_flag = 1;
            
            dbg_printf("\r\nReset start option:HOTSTART\r\n");

            USART_SendString(USART3, (uint8_t*)CFG_RST_HOTSTART, sizeof(CFG_RST_HOTSTART)/sizeof(CFG_RST_HOTSTART[0]));
        break;
        
        case WARMSTART:
            gps_cmd_flag = 1;
            
            dbg_printf("\r\nReset start option:WARMSTART\r\n");
            
            USART_SendString(USART3, (uint8_t*)CFG_RST_WARMSTART, sizeof(CFG_RST_WARMSTART)/sizeof(CFG_RST_WARMSTART[0]));
        break;

        case COLDSTART:
            gps_cmd_flag = 1;

            dbg_printf("\r\nReset start option:COLDSTART\r\n");
            
            USART_SendString(USART3, (uint8_t*)CFG_RST_COLDSTART, sizeof(CFG_RST_COLDSTART)/sizeof(CFG_RST_COLDSTART[0]));
        break;

        default:
          dbg_printf("\r\ncfg_rst_start_option erro!!!\r\n");
        break;
    }
}

/**
 * Write to the Gps
 * 
 * 
 */
int gps_write(const char *buf, const int len)
{
    return USART_SendString(USART3, (uint8_t *)buf, len);
}

/**
 * ubx_write: combine ubx_msg 
 * 
 * 
 */
int ubx_write(unsigned int msg_class, unsigned int msg_id, unsigned char *msg, int data_len)
{
    unsigned char CK_A, CK_B;
    int count;
    int i;
    int ok = 0;
    int msgbuflen;

    msgbuf[0] = 0xb5;
    msgbuf[1] = 0x62;

    CK_A = CK_B = 0;
    msgbuf[2] = msg_class;
    msgbuf[3] = msg_id;
    msgbuf[4] = data_len & 0xff;
    msgbuf[5] = (data_len >> 8) & 0xff;

    if (msg != NULL)
	(void)memcpy(&msgbuf[6], msg, data_len);

    /* calculate CRC */
    for (i = 2; i < 6; i++) {
	CK_A += msgbuf[i];
	CK_B += CK_A;
    }
    if (msg != NULL)
	for (i = 0; i < data_len; i++) {
	    CK_A += msg[i];
	    CK_B += CK_A;
	}

    msgbuf[6 + data_len] = CK_A;
    msgbuf[7 + data_len] = CK_B;
    msgbuflen = data_len + 8;
    
    count = gps_write(msgbuf, msgbuflen);
    ok = (count == (int)msgbuflen);
    
    return (ok);
}

/**
 * ubx_cmd: different cmd and class for send different msg;
 * class for CLASS;
 * cmd for ID;
 */
int ubx_cmd(uint32_t cmd, uint32_t class)
{
    if((class == UBX_CLASS_MON)||(class == UBX_CLASS_NAV))
    {
        switch(cmd)
        {
            case UBX_MON_GNSS:
            case UBX_MON_HW:
            case UBX_MON_VER:
            case UBX_NAV_CLOCK: 
            case UBX_NAV_PVT: 
            case UBX_NAV_SAT: 
            case UBX_NAV_STATUS: 
                sprintf(GpsDebugBuffer, "ubx_write, class: 0x%x, cmd: 0x%x\r\n", class, cmd);
                dbg_printf(GpsDebugBuffer);
                 
                gps_cmd_flag = 1;
                ubx_write(class, cmd, NULL, 0);
            break;
            default:
                dbg_printf("\r\nCLASS MON AND NAV don't support the command\r\n");
            break;
        }
    }
    else if(class = UBX_CLASS_CFG)
    {
        if(cmd == UBX_CFG_PRT)
        {
            gps_cmd_flag = 1;
            USART_SendString(USART3, (uint8_t*)CFG_PRT_UBX, sizeof(CFG_PRT_UBX)/sizeof(CFG_PRT_UBX[0]));
        }
        else
        {
            dbg_printf("\r\nCLASS CFG don't support the command\r\n");
        }
    }

    return 0;  
}


/**
 * Receiver/Software Version
 * UBX-MON-VER
 *
 * sadly more info than fits in session->swtype for now.
 * so squish the data hard, max is maybe 100?
 */
static uint32_t ubx_msg_mon_ver(gps_device_t* session, uint8_t *buf, size_t data_len)
{
    int n = 0;	/* extended info counter */
        
    int i = 0;
    if ( 44 > data_len ) {
	/* incomplete message */
        return 1;
    }

    memcpy(sw_ver, buf, 30);
    sprintf(GpsDebugBuffer, "sw_ver:%s\r\n", sw_ver);
    dbg_printf(GpsDebugBuffer);

    memcpy(hw_ver, buf+30, 10);
    sprintf(GpsDebugBuffer, "hw_ver:%s\r\n", hw_ver);
    dbg_printf(GpsDebugBuffer);
    
    /* get n number of Extended info strings.  what is max n? */
    for ( n = 0; ; n++ ) 
    {
        int start_of_str = UBX_MESSAGE_DATA_OFFSET + 40 + (30 * n);

        if ( (start_of_str + 2 ) > data_len ) 
        {
	        /* last one can be shorter than 30 */
            /* no more data */
            memcpy(ex_ver, buf+40, 40 + (30*n));
            break;
        }
    }
    
    sprintf(GpsDebugBuffer, "ex_ver:%s\r\n", ex_ver);
    dbg_printf(GpsDebugBuffer);

    return 0;

}

/**
 * Receiver Information message major GNSS selection
 * UBX-MON-GNSS
 *
 */
static uint32_t ubx_msg_mon_gnss(gps_device_t *session, uint8_t *buf, size_t data_len)
{
    size_t n = 0;	/* extended info counter */
    uint8_t gnss_enabled;


    if ( 8 !=  data_len ) {
	/* incomplete message */
        return 1;
    }

    gnss_enabled = (uint8_t)getub(buf,3);
        
    sprintf((char *)GpsDebugBuffer, "gnss_enabled: 0x%02x\r\n", gnss_enabled);
    dbg_printf((char *)GpsDebugBuffer);
    
    if(gnss_enabled & (0x01 << 3))
    {
        dbg_printf("\r\nGalileo is enabled!\r\n");
                                     
    }
    
    if(gnss_enabled & (0x01 << 2))
    {
        dbg_printf("\r\nBeiDou is enabled!\r\n");
                                
    }
    
    if(gnss_enabled & (0x01 << 1))
    {
        dbg_printf("\r\nGLONASS is enabled!\r\n");
    }
    
    if(gnss_enabled & (0x01 << 0))
    {
        dbg_printf("\r\nGPS is enabled!\r\n");
    }
    session->gnss_enabled = gnss_enabled;
    return 0;

}

/**
 * Receiver Hardware Status message
 * UBX-MON-HW
 *
 */
static int32_t ubx_msg_mon_hw(gps_device_t *session, uint8_t *buf, size_t data_len)
{
    size_t n = 0;	/* extended info counter */
    uint8_t aStatus, rtccalib, aPower;
    uint16_t noisePerMS;
    uint32_t pinval;
    uint8_t ext1_val;

    if ( 60 !=  data_len ) {
	/* incomplete message */
        return 0;
    }

    noisePerMS = (uint16_t)getleu16(buf, 16);
    session->noisePerMS = noisePerMS;

    aStatus = (uint8_t)getub(buf, 20);
    switch(aStatus)
    {
        case UBX_A_STATUS_INIT:
            session->hw_astatus = 0;
            dbg_printf("Status of the Antenna Supervisor State Machine is INIT!\r\n");
            break;
        
        case UBX_A_STATUS_DONTKNOW:
            session->hw_astatus = 1;
            dbg_printf("Status of the Antenna Supervisor State Machine is DONTKNOW!\r\n");
            break;

        case UBX_A_STATUS_OK:
            session->hw_astatus = 2;
            dbg_printf("Status of the Antenna Supervisor State Machine is OK!\r\n");
            break;

        case UBX_A_STATUS_SHORT:
            session->hw_astatus = 3;
            dbg_printf("Status of the Antenna Supervisor State Machine is SHORT!\r\n");
            break;

        case UBX_A_STATUS_OPEN:
            session->hw_astatus = 4;
            dbg_printf("Status of the Antenna Supervisor State Machine is OPEN!\r\n");
            break;
        
        default:
            break;
    }

    aPower = (uint8_t)getub(buf, 21);
    switch(aPower)
    {
        case UBX_A_POWER_OFF:
            session->hw_apower = 0;
            dbg_printf("Current PowerStatus of Antenna is OFF!\r\n");
            break;
        
        case UBX_A_POWER_ON:
            session->hw_apower = 1;
            dbg_printf("Current PowerStatus of Antenna is ON!\r\n");
            break;

        case UBX_A_POWER_DONTKNOW:
            session->hw_apower= 2;
            dbg_printf("Current PowerStatus of Antenna is DONTKNOW!\r\n");
            break;

        default:
            break;
    }

    rtccalib = (uint8_t)getub(buf, 22);
    session->hw_rtccalib = rtccalib;
    
    pinval = (uint32_t)getleu32(buf, 12);
    ext1_val = (uint8_t)((pinval>>14) & 0x1);
 
    sprintf(GpsDebugBuffer, "hw_rtccalib: %02x\r\n", session->hw_rtccalib);
	dbg_printf(GpsDebugBuffer); 

    sprintf(GpsDebugBuffer, "pinval: %08x, ext1_val:%x \r\n", pinval, ext1_val);
	dbg_printf(GpsDebugBuffer);

    return 0;

}

/**
 * Navigation Position Velocity Time Solution
 * UBX-NAV-PVT
 *
 */
static uint32_t ubx_msg_nav_pvt(gps_device_t *session, uint8_t *buf, size_t data_len)
{
    uint32_t tow;
    uint16_t year;
    uint8_t month, day, hour, min, sec, fix_type, fix_flag, numSV;
    uint32_t tAcc ;
    uint32_t lon, lat, alt, hMSL, hAcc, vAcc, gSpeed;
    if (data_len != 92)
    {
	    return 1;
    }

    tow = (uint32_t)getleu32(buf, 0);
    session->pvt_tow = tow;

    year = (uint16_t)getleu16(buf, 4);
    month = (uint8_t)getub(buf, 6);
    day = (uint8_t)getub(buf, 7);
    hour = (uint8_t)getub(buf, 8);
    min = (uint8_t)getub(buf, 9);
    sec = (uint8_t)getub(buf, 10);

    session->utc_time.year = year;
    session->utc_time.month = month;
    session->utc_time.day = day;
    session->utc_time.hour = hour;
    session->utc_time.min = min;
    session->utc_time.sec = sec;

    tAcc = (uint32_t)getleu32(buf, 12);
    session->tAcc = tAcc;

    fix_type = (uint8_t)getub(buf, 10);
    switch (fix_type) 
    {
        case UBX_MODE_DRONLY:
	    session->fix_type = UBX_MODE_DRONLY;
        break;
        
        case UBX_MODE_2D:	/* FIX-ME: DR-aided GPS may be valid 3D */
	    session->fix_type = UBX_MODE_2D;
	    break;

        case UBX_MODE_3D:
	    session->fix_type = UBX_MODE_3D;
        break;
    
        case UBX_MODE_GNSS_DRCOMBINED:
	    session->fix_type = UBX_MODE_GNSS_DRCOMBINED;
        break;    

        case UBX_MODE_TMONLY:
        session->fix_type = UBX_MODE_TMONLY;
        break;
      
        default:
	    session->fix_type = UBX_MODE_NO_FIX;
    }

    fix_flag = (uint8_t)(getub(buf, 21));
    session->fix_flag = fix_flag;

    numSV = (uint8_t)(getub(buf, 23));
    session->numSV = numSV;
    
    lat= (int32_t)(getles32(buf, 24));
    session->latitude = lat;

    lon = (int32_t)(getles32(buf, 28));
    session->longtitude = lon;

    alt = (int32_t)(getles32(buf, 32));
    session->altitude = alt;

    hMSL = (int32_t)(getles32(buf, 36));
    session->hMSL = hMSL;

    hAcc = (uint32_t)(getles32(buf, 40));
    session->hAcc = hAcc;

    vAcc = (uint32_t)(getles32(buf, 44));
    session->vAcc =vAcc;

    gSpeed = (int32_t)(getles32(buf, 60));
    session->gSpeed = gSpeed;
        

#if 1
    sprintf(GpsDebugBuffer, "tow: %d, 0x%08x, GPS Time Tag:%d \r\n", tow, tow, tow/1000);
	dbg_printf(GpsDebugBuffer);
    
    sprintf(GpsDebugBuffer, "year: %d, 0x%x, \r\n", year, year);
	dbg_printf(GpsDebugBuffer);
    
     sprintf(GpsDebugBuffer, "month: %d, 0x%x, \r\n", month, month);
	dbg_printf(GpsDebugBuffer);

    
    sprintf(GpsDebugBuffer, "tAcc: %d, 0x08%x, \r\n", tAcc, tAcc);
	dbg_printf(GpsDebugBuffer);
        
    sprintf(GpsDebugBuffer, "fix_type: %d, 0x%x, \r\n", fix_type, fix_type);
	dbg_printf(GpsDebugBuffer);

    sprintf(GpsDebugBuffer, "fix_flag: %d, 0x%x, \r\n", fix_flag, fix_flag);
	dbg_printf(GpsDebugBuffer);
    
    sprintf(GpsDebugBuffer, "lat: %d, 0x%08x, \r\n", lat, lat);
	dbg_printf(GpsDebugBuffer);
    
    sprintf(GpsDebugBuffer, "alt: %d, 0x%08x, \r\n", alt, alt);
	dbg_printf(GpsDebugBuffer);
     
    sprintf(GpsDebugBuffer, "hAcc: %d, 0x%08x, \r\n", hAcc, hAcc);
	dbg_printf(GpsDebugBuffer);
    
    sprintf(GpsDebugBuffer, "gSpeed: %d, 0x%08x, \r\n", gSpeed, gSpeed);
	dbg_printf(GpsDebugBuffer);
    
    sprintf(GpsDebugBuffer, "numSV: %d, 0x%08x, \r\n", numSV, numSV);
	dbg_printf(GpsDebugBuffer);
#endif
    return 0;
}

/**
 * Satellite Information
 * UBX-NAV-SAT
 *
 */
static uint32_t ubx_msg_nav_sat(gps_device_t *session, uint8_t *buf, size_t data_len)
{
    uint32_t numSvs, i;
    uint8_t svId, cno;

    if (data_len < 20)
	return 1;
    
    numSvs = (uint32_t)getub(buf, 5);
        
    sprintf(GpsDebugBuffer, "numSvs: %d, 0x%08x, \r\n", numSvs, numSvs);
	dbg_printf(GpsDebugBuffer);

    for (i = 0; i < 10; i++) 
    {
	    uint32_t off = 8 + 12 * i;
    
        svId =(uint8_t)getub(buf, off + 1);
        session->svId[i] = svId;

        cno = (uint8_t)getub(buf, off + 2);
        session->cno[i] = cno;

        sprintf(GpsDebugBuffer, "numSvs[%d]: svId:0x%02x, cno:0x%02x \r\n", i,  session->svId[i], session->cno[i]);
	    dbg_printf(GpsDebugBuffer);
    }
    return 0;

}

/**
 * Receiver Navigation Status
 * UBX-NAV-STATUS
 *
 */
static uint32_t ubx_msg_nav_status(gps_device_t *session, uint8_t *buf, size_t data_len)
{
    uint32_t numSvs, i;
    uint8_t gpsFix, flags;
    uint8_t wknset, towset;
    uint32_t ttff, mass;

    if (data_len != 16)
	return 1;
    
    gpsFix = (uint8_t)getub(buf, 4);
    
    switch (gpsFix) 
    {
        case UBX_MODE_DRONLY:
	    session->gpsFix = UBX_MODE_DRONLY;
        break;
        
        case UBX_MODE_2D:	
	    session->gpsFix = UBX_MODE_2D;
	    break;

        case UBX_MODE_3D:
	    session->gpsFix = UBX_MODE_3D;
        break;
    
        case UBX_MODE_GNSS_DRCOMBINED:
	    session->gpsFix = UBX_MODE_GNSS_DRCOMBINED;
        break;    

        case UBX_MODE_TMONLY:
        session->gpsFix = UBX_MODE_TMONLY;
        break;
      
        default:
	    session->gpsFix = UBX_MODE_NO_FIX;
    }

    flags = getub(buf, 5);
    wknset = (flags &(0x01 << 2));
    session->wknset = wknset;

    towset = (flags &(0x01 << 3));
    session->towset = towset;

    ttff = (uint32_t)getleu32(buf, 8);
    session->ttff = ttff;

    mass = (uint32_t)getleu32(buf, 12);
    session->mass = mass;

    sprintf(GpsDebugBuffer, "gpsFix:0x%x, %d\r\n", gpsFix, gpsFix);
	dbg_printf(GpsDebugBuffer);

    sprintf(GpsDebugBuffer, "flags:0x%x, wknset:0x%x, towset:0x%x\r\n", flags, wknset, towset);
	dbg_printf(GpsDebugBuffer);
    
    sprintf(GpsDebugBuffer, "ttff:0x%08x, %d \r\n", ttff, ttff);
	dbg_printf(GpsDebugBuffer);

    sprintf(GpsDebugBuffer, "mass:0x%08x, %d \r\n", mass, mass);
	dbg_printf(GpsDebugBuffer);

    return 0;

}


/**
 * Clock Solution
 * UBX-NAV-CLOCK
 *
 */
static uint32_t ubx_msg_nav_clock(gps_device_t *session, uint8_t *buf, size_t data_len)
{

    uint32_t clock_tAcc, clock_fAcc;

    if (data_len != 20)
	return 1;

    clock_tAcc = (uint32_t)getleu32(buf, 12);
    session->clock_tAcc = clock_tAcc;
    
    clock_fAcc = (uint32_t)getleu32(buf, 16);
    session->clock_fAcc = clock_fAcc;  
    
    sprintf(GpsDebugBuffer, "clock_tAcc : 0x%08x, %d\r\n", clock_tAcc, clock_tAcc);
	dbg_printf(GpsDebugBuffer); 

    sprintf(GpsDebugBuffer, "clock_fAcc : 0x%08x, %d\r\n", clock_fAcc, clock_fAcc);
	dbg_printf(GpsDebugBuffer); 
    
    return 0; 

}

/**
 * Message Acknowledged
 * UBX-ACK-ACK
 *
 */
static uint32_t ubx_msg_ack_ack(gps_device_t *session, uint8_t *buf, size_t data_len)
{
    uint8_t clsid, msgid;

    if (data_len < 2)
	return 1;
    
    clsid = (uint8_t)getub(buf, 0);
    msgid = (uint8_t)getub(buf, 1);
    
    sprintf(GpsDebugBuffer, "cfg sucess, clsid:%x, msgid:%x\r\n", clsid, msgid);
	dbg_printf(GpsDebugBuffer);

    if(clsid == UBX_CLASS_CFG)
    {
        switch(msgid)
        {
            case UBX_CFG_PRT: 
                    dbg_printf("\r\nCFG PRT for DISABLE NEMA, ONLY UBX PROTOCOL\r\n");
            break;

            case UBX_CFG_GNSS:
                    dbg_printf("\r\nCFG GNSS SUCESS\r\n");
            break;

            case UBX_CFG_PWR:
                    dbg_printf("\r\nCFG PWR SUCESS\r\n");
            break;

            case UBX_CFG_RST:
                    dbg_printf("\r\nCFG RST SUCESS\r\n");
            break;

            case UBX_CFG_CFG:		            
                    dbg_printf("\r\nCFG LOAD SAVE TO FLASH SUCESS\r\n");
            break;

            default:
            break;
        }
    }
    
    return 0;
}

int ubx_parse(gps_device_t * session, uint8_t *buf, size_t len)
{
    size_t data_len;
    uint16_t msgid;

    /* the packet at least contains a head long enough for an empty message */
    if (len < UBX_PREFIX_LEN)
	return 1;

    /* extract message id and length */
    msgid = (buf[2] << 8) | buf[3];
    data_len = (size_t)getles16(buf, 4);
    
    sprintf(GpsDebugBuffer, "msgid: %x\r\n", msgid);
	dbg_printf(GpsDebugBuffer);
    
    sprintf(GpsDebugBuffer, "data_len: %x\r\n", data_len);
	dbg_printf(GpsDebugBuffer);
 
    switch (msgid) 
    {
        case MSGID_MON_GNSS:
	        ubx_msg_mon_gnss(session, &buf[UBX_PREFIX_LEN], data_len);
	    break;

        case MSGID_MON_HW:
            ubx_msg_mon_hw(session, &buf[UBX_PREFIX_LEN], data_len);

	    break;

        case MSGID_MON_VER:
	        ubx_msg_mon_ver(session, &buf[UBX_PREFIX_LEN], data_len);
	    break;
    
        case MSGID_NAV_STATUS:
            ubx_msg_nav_status(session, &buf[UBX_PREFIX_LEN], data_len);
        break;

        case MSGID_NAV_PVT:
	        ubx_msg_nav_pvt(session, &buf[UBX_PREFIX_LEN], data_len);
        break;

        case MSGID_NAV_CLOCK:
	        ubx_msg_nav_clock(session, &buf[UBX_PREFIX_LEN], data_len);
        break;

        case MSGID_NAV_SAT:
	        ubx_msg_nav_sat(session, &buf[UBX_PREFIX_LEN], data_len);
        break;

        case MSGID_ACK_ACK:
            ubx_msg_ack_ack(session, &buf[UBX_PREFIX_LEN], data_len);
        break;
    
        default:
        break;
    }
#if 0
    sprintf(GpsDebugBuffer, "gnss_mode: %x\r\n", session->gnss_enabled);
	dbg_printf(GpsDebugBuffer);
    
    sprintf(GpsDebugBuffer, "noisePerMS: %x\r\n", session->noisePerMS);
	dbg_printf(GpsDebugBuffer);

    sprintf(GpsDebugBuffer, "hw_astatus: %x\r\n", session->hw_astatus);
	dbg_printf(GpsDebugBuffer);

    sprintf(GpsDebugBuffer, "hw_apower: %x\r\n", session->hw_apower);
	dbg_printf(GpsDebugBuffer); 

    sprintf(GpsDebugBuffer, "hw_rtccalib: %x\r\n", session->hw_rtccalib);
	dbg_printf(GpsDebugBuffer);

    sprintf(GpsDebugBuffer, "session->clock_tAcc : %x\r\n", session->clock_tAcc);
	dbg_printf(GpsDebugBuffer); 

    sprintf(GpsDebugBuffer, "session->clock_fAcc : %x\r\n", session->clock_fAcc);
	dbg_printf(GpsDebugBuffer);
#endif 
    return 0;
}

void CM55_Request(CM55_Request_t req)
{
	CM55_Req = req;

#ifdef TASK7_DEBUG   
    sprintf(CM55DebugBuffer, "CM55_Req: 0x%08X\r\n", CM55_Req);
	dbg_printf(CM55DebugBuffer);
#endif
}

void GetCM55cPHDiff(CM55_INFO *CM55)
{
//    NOTE:dbg_printf cann't print float data

    sprintf(CM55DebugBuffer, "CM55 cPHDiff: %d\r\n", CM55 -> cPHDiff);
	dbg_printf(CM55DebugBuffer);

    sprintf(CM55DebugBuffer, "CM55 year: %d\r\n", CM55 -> date.year);
	dbg_printf(CM55DebugBuffer);

    sprintf(CM55DebugBuffer, "CM55 month: %d\r\n", CM55 -> date.month);
	dbg_printf(CM55DebugBuffer);
    
    sprintf(CM55DebugBuffer, "CM55 day: %d\r\n", CM55 -> date.day);
	dbg_printf(CM55DebugBuffer);

}

void Task7(void* p)
{
    gps_device_t gps;
    int i = 0;

    while (1)
    {
	    if (ReceiveState == 1)
	    {
	        USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);
	        USART_ITConfig(USART3, USART_IT_IDLE, DISABLE);

	        memcpy(InputStr, aRxBuffer, ubRxIndex);
	        InputStr[ubRxIndex] = 0;
//	        dbg_printf(InputStr);
            
            if(Fpga_GetMcuUart3Sel() ==  McuUart3Mux_GPS)
            {
                if (gps_cmd_flag == 1)
                { 
                    dbg_printf("\r\nstart..............\n\r");
                    sprintf(GpsDebugBuffer, "ubRXindex: %d\r\n", ubRxIndex);
	                dbg_printf(GpsDebugBuffer);
                    
                    ubx_parse(&gps, InputStr, ubRxIndex);
    	            dbg_printf("\r\nend..............\n\r");
                    gps_cmd_flag = 0;
                
                }
            }
            else if(Fpga_GetMcuUart3Sel() ==  McuUart3Mux_CM55)
            {
                CM55_Parse((char *)InputStr, &CM55);
            }

	        ubRxIndex = 0;
	        ReceiveState = 0;

	        USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	        USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);
	    }
    
        if(CM55_Req == CM55_Request_cPHDiff)
	    {
            CM55_Req = CM55_Request_Idle;
		    GetCM55cPHDiff(&CM55);
	    }
    }

	vTaskDelete(NULL);
}
