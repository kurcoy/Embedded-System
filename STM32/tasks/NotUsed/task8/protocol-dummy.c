#include "crc-ccitt.h"
#include "slip.h"
#include "task8.h"
#include "debug.h"
#include <string.h>

#define GET_WARNING 	0x0101
#define REPORT_WARNING	0x0201
#define DELETE_WARNING	0x0301
#define GET_MONITOR		0x0401
#define REPORT_MONITOR	0x0501
#define SET_MCU			0x0601
#define ACK				0xff00
#define NACK			0xff01

#define CRC_ERROR 		(-1)
#define LENGTH_ERROR	(-2)
#define ID_ERROR		(-3)
#define OK				0	

static uint8_t PACKET_BUFFER[72];

int report_result(int result)
{	
	unsigned char ack[6];
	unsigned char ack_slip[12];
	unsigned short crc;
	unsigned int slip_len = 0;
	switch(result){
		case 1:
			ack[0] = ACK >> 8;
			ack[1] = ACK;
			ack[2] = 0;
			ack[3] = 0;
			break;
		case 0:
			ack[0] = NACK >> 8;
			ack[1] = NACK;
			ack[2] = 0;
			ack[3] = 0;
			break;
		default:
			dbg_printf("report result error\n");
	}
	crc = crc_ccitt(0,ack,4);
	ack[4] = (crc >> 8) & 0xff;
	ack[5] = crc & 0xff;
	slip_len = slipPacket(ack,ack_slip,6);
	uart_send(ack_slip,slip_len);
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
		case DELETE_WARNING:
		case GET_MONITOR:
		case SET_MCU:
		case ACK:
		case NACK:
		default:
		memcpy(PACKET_BUFFER,packet,data_len);
		PACKET_BUFFER[data_len] = '\r';
		PACKET_BUFFER[data_len + 1] = '\n';
		dbg_printf((char*)PACKET_BUFFER);
		break;
	}
}

