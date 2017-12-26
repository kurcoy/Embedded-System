#ifndef __task8_h__
#define __task8_h__
#include <stdio.h>
#include "multi-task.h"

#define TASK8_STACK_SIZE	256
#define TASK8_PERIOD	20 /* 20Mms*/

#define BUFFERSIZE      128

extern StackType_t Task8Stack[TASK8_STACK_SIZE] CCM_RAM;  // Put task stack in CCM
extern StaticTask_t Task8Buffer CCM_RAM;  // Put TCB in CCM

void Task8(void* p);

typedef enum {
	IDLE =0,		//空闲状态
	START_RX,		//收到slip开始信号
	START_TX,		//收到slip开始信号
	RX_ING,			//接收slip帧状态
	TX_ING,			//传输slip帧状态
	RX_END,			//接收slip帧完成状态
	TX_END,			//传输slip帧完成状态
	RX_ERROR,		//接收slip帧出错
	TX_ERROR		//传输slip帧出错
}uart_state_machine;

int uart_send(unsigned char * ptr, int len);

#endif/*__taskt_h__*/
