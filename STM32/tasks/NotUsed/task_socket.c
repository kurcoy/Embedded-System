#include "multi-task.h"
#include "FreeRTOS.h"
#include "task_socket.h"
#include "main.h"
#include "sockets.h"

StackType_t TaskSocketStack[TASK_SOCKET_STACK_SIZE] CCM_RAM;  // Put task stack in CCM
StaticTask_t TaskSocketBuffer CCM_RAM;  // Put TCB in CCM

static char SocketBuffer[32];

void Task_socket(void *p)
{
  int skt = 0;
  int counter=0;
  struct sockaddr_in address;
  int addr_len = sizeof(struct sockaddr_in);

  vTaskDelay(7000);/* 300ms */
  memset(&address, 0, sizeof(address));
  address.sin_family=AF_INET;
  address.sin_addr.s_addr=htonl(INADDR_ANY);//inet_addr("192.168.1.107");
  address.sin_port=htons(4008);

  skt = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

  bind(skt, (struct sockaddr *)&address, sizeof(address));

  while(skt >= 0)
  {
	recvfrom(skt, SocketBuffer, sizeof(SocketBuffer), 0, (struct sockaddr *)&address, &addr_len);
	sprintf(SocketBuffer, "socket UDP testing %d\r\n", counter++);
	sendto(skt, SocketBuffer, strlen(SocketBuffer), 0, (struct sockaddr *)&address, sizeof(address));
	vTaskDelay(300);/* 300ms */
  }


  vTaskDelete(NULL);
}
