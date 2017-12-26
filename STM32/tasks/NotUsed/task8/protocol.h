#ifndef __PROTOCOL_H_
#define __PROTOCOL_H_

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
#define UNKNOWN_PACKET		(-4)

int encap_and_send_packet(unsigned short id,unsigned char *data,int length);
int parse_rx_packet(unsigned char* packet, int len);

#endif

