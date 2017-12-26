#include "multi-task.h"
#include "FreeRTOS.h"
#include "task_socket.h"
#include "main.h"
#include "sockets.h"
#include "task_socket2.h"

StackType_t TaskSocketStack2[TASK_SOCKET_STACK_SIZE2] CCM_RAM;  // Put task stack in CCM
StaticTask_t TaskSocketBuffer2 CCM_RAM;  // Put TCB in CCM

static char SocketBuffer2[32];

extern int PtpdMain(int argc, char **argv);

void Task_socket2(void *p)
{
  int skt = 0;
  int counter=0;
  struct sockaddr_in address;
  int addr_len = sizeof(struct sockaddr_in);

  printf("Task_socket2 is running !\r\n");
  vTaskDelay(7000);/* 300ms */
#if 0
  memset(&address, 0, sizeof(address));
  address.sin_family=AF_INET;
  address.sin_addr.s_addr=inet_addr("192.168.1.111");
  address.sin_port=htons(4009);

  skt = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);


//  bind(skt, (struct sockaddr *)&address, sizeof(address));

  while(skt >= 0)
  {
//	recvfrom(skt, SocketBuffer, sizeof(SocketBuffer), 0, (struct sockaddr *)&address, &addr_len);
	sprintf(SocketBuffer2, "socket UDP testing %d\r\n", counter++);
	sendto(skt, SocketBuffer2, strlen(SocketBuffer2), 0, (struct sockaddr *)&address, sizeof(address));
	vTaskDelay(300);/* 300ms */
  }
#endif

  PtpdMain(0,0);

  vTaskDelete(NULL);
}
