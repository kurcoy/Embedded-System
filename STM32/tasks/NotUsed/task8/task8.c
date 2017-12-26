/* this task is the driver for the lmk04031 */

#include "multi-task.h"
#include "FreeRTOS.h"
#include "task8.h"
#include "stm32f4xx_usart.h"
#include "debug.h"
#include "string.h"
#include <stdarg.h>
#include "debug.h"
#include "protocol.h"
#include "slip.h"


StackType_t Task8Stack[TASK8_STACK_SIZE] CCM_RAM;  // Put task stack in CCM
StaticTask_t Task8Buffer CCM_RAM;  // Put TCB in CCM

static char rx_packet[BUFFERSIZE];
unsigned char tx_packet[BUFFERSIZE] = {0,1,2,3,4,5,6,7,8,9};
static int tx_packet_len = 10;

static uint8_t PRINT_BUFFER[128];
static uart_state_machine sm;
static int len;
int has_packet_to_send = 0;
void ISR_UART1(void)
{
	unsigned char ch = 0;
    /* USART in Receiver mode */
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
	{
		/* Receive Transaction data */
		ch = USART_ReceiveData(USART1);
		sprintf((char *)PRINT_BUFFER, "uart1 receive %d\r\n", (int)ch);
		dbg_printf(PRINT_BUFFER);
		switch (ch)
		{
			case END:
			switch (sm)
			{
				case IDLE:
				case START_RX:
					sm =  START_RX;
					len = 1;
					rx_packet[len-1] = ch;
					break;
				case RX_ING:
					sm = RX_END;
					rx_packet[len] = ch;
					len++;
					break;
				default:
					sm = RX_ERROR;
					len = 0;
					break;
			}
			break;
			default:
			switch (sm)
			{
				case IDLE:
					sm =  IDLE;
					len = 0;
					break;
				case START_RX:
					sm = RX_ING;
					rx_packet[len] = ch;
					len++;
					break;
				case RX_ING:
					sm = RX_ING;
					rx_packet[len] = ch;
					len++;
					break;
				default:
					sm = RX_ERROR;
					len =0;
					break;
			}
		}
	}

	
}

int uart_send(unsigned char * ptr, int len) {
	int index;
  	if (!ptr) {
    		return 0;
  	}
  	for (index = 0; index < len; index++) {
    		
		while (!(USART1->SR & (uint16_t)0x0040));
    		USART_SendData(USART1, ptr[index]);
 	}
  	return len;
}

void Task8(void* p)
{
	while (1)
	{

		if(sm == RX_ERROR)
		{
			dbg_printf("rx sm error\r\n");
			sm = IDLE;
		}else if(sm == RX_END)
		{
			dbg_printf("rx one slip frame success\r\n");
			parse_rx_packet(rx_packet,len);
			sm = IDLE;
		}
/*		
		dbg_printf("uart1 send start\r\n");
		uart_send(tx_packet,tx_packet_len);
		dbg_printf("uart1 send end\r\n");
*/
//		dbg_printf("task8 is running\r\n");
		vTaskDelay(10000); /* 10s */
	}

	vTaskDelete(NULL);
}
