#include "crc-ccitt.h"
#include "hw.h"
#include "RTC3232_task.h"
#include "slip.h"
#include "task8.h"
#include "debug.h"
#include "protocol.h"


extern unsigned char tx_packet[BUFFERSIZE];

long power(int x,int y)
{
	long val = 1;
	while(y--)
		val *= x;
	return val;
	return val;
}

/*
BCD 转10进制
*/
unsigned long BCDtoDec(unsigned char* bcd,int length)
{
	int i,tmp;
	unsigned long dec = 0;
	for(i = 0;i < length;i++)
	{
		tmp = ((bcd[i] >> 4)&0x0f)*10 + (bcd[i]&0x0f);
		dec+=tmp * power(100,length - 1 -i);
	}
	return dec;
}
/*
基姆拉尔森公式计算星期
0 -- sunday
1 -- monday
...
6 -- Saturday
*/

int getWeekDay(int y,int m,int d)
{
	int week = -1;
	if(m == 1 || m==2)
	{
		m +=12;
		y--;
	}
	week = (d+1+2*m + 3*(m+1)/5 +y +y/4 - y/100 +y/400)%7;
	return week;
}



int encap_and_send_packet(unsigned short id,unsigned char *data,int length)
{
	int i = 0,slip_len = 0;
	unsigned char tmp[100];
	unsigned short crc;
	tmp[0] = id >>8 & 0xff;
	tmp[1] =id &0xff;
	tmp[2] = length >>8 &0xff;
	tmp[3] = length&0xff;
	for(i = 0;i < length;i++)
		tmp[3+i] = data[i];
	crc = crc_ccitt(0,tmp,length + 4);
	tmp[length + 4] = (crc >> 8) & 0xff;
	tmp[length + 5] = crc & 0xff;
	slip_len = slipPacket(tmp,tx_packet,length + 6);
	uart_send(tx_packet,slip_len);

	return 0;
}

void report_result(int result)
{	
	switch(result){
		case 1:
			encap_and_send_packet(ACK,NULL,0);
			break;
		case 0:
			encap_and_send_packet(NACK,NULL,0);
			break;
		default:
			dbg_printf("report result error\n");
	}
}
int check_packet(unsigned char* packet,int len)
{
	unsigned short crc_data = packet[len - 2] << 8 | packet[len - 1];
	if(crc_ccitt(CRC_INIT,packet,len -2) == crc_data)
		return 1;
	else 
		return 0;
}

int parse_rx_packet(unsigned char* packet, int len)
{
	Time_Typedef start,end;
	unsigned short data_len = 0,unpacket_len = 0,id;
	unpacket_len = slipUnPacketPro(packet,len);
	if(!check_packet(packet,unpacket_len))
	{
		dbg_printf("packet crc error\n");
		return CRC_ERROR;
	}
	data_len = packet[2] << 8 | packet[3];
	if(data_len != unpacket_len -6)
	{
		dbg_printf("packet length error\n");
		return LENGTH_ERROR;
	}
	
	id = packet[0] << 8 | packet[1];
	switch(id){
		case GET_WARNING:
			start.year = BCDtoDec(packet + 5,2);
			start.month = BCDtoDec(packet + 7,1);
			start.date = BCDtoDec(packet + 8,1);
			start.week = getWeekDay(start.year,start.month,start.date);
			end.year = BCDtoDec(packet + 12,2);
			end.month = BCDtoDec(packet +14,1);
			end.date = BCDtoDec(packet + 15,1);		
			end.week = getWeekDay(end.year,end.month,end.date);
			report_warning(start,end);
			break;
		case DELETE_WARNING:
			start.year = BCDtoDec(packet + 5,2);
			start.month = BCDtoDec(packet + 7,1);
			start.date = BCDtoDec(packet + 8,1);
			start.week = getWeekDay(start.year,start.month,start.date);
			end.year = BCDtoDec(packet + 12,2);
			end.month = BCDtoDec(packet +14,1);
			end.date = BCDtoDec(packet + 15,1);		
			end.week = getWeekDay(end.year,end.month,end.date);
			delete_warning(start,end);
			break;
		case GET_MONITOR:
			report_monitor(packet,unpacket_len);
			break;
		case SET_MCU:
			set_mcu(packet,unpacket_len);
			break;
		case ACK:
			break;
		case NACK:
			break;
		default:
			return UNKNOWN_PACKET;
			break;
	}
	return 0;
}
