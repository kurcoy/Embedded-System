#include "hw.h"
#include "RTC3232_task.h"
#include "task8.h"
#include "protocol.h"
#include "task6.h"

extern void sFLASH_EraseBlock(uint32_t BlockAddr);
//read warning information from flash
int report_warning(Time_Typedef start,Time_Typedef end)
{
	int i =0,j = 0;
	unsigned char warning[16] = {0};
	if(end.week < start.week )
		end.week = end.week + 7;
	for(i = start.week;i<= end.week;i++){
		for(j = (i %7) * 0x10000 ;;j += 16 ){
			sFLASH_ReadBuffer(warning,j,16);
			if(warning[0]== 0xff){
				break;
			}
			else{
				encap_and_send_packet(REPORT_WARNING,warning,13);
			}	
		}	
	}
	encap_and_send_packet(ACK,NULL,0);
	return 0;
}

//delete warning information from flash
int delete_warning(Time_Typedef start,Time_Typedef end)
{
	int i =0;
	if(end.week < start.week )
		end.week = end.week + 7;
	for(i = start.week;i<= end.week;i++){
		sFLASH_EraseBlock(i%7);
	}
	encap_and_send_packet(ACK,NULL,0);
	return 0;
}

int report_monitor(unsigned char* packet,int len)
{
	unsigned short type = packet[4] << 8 | packet[5];
	unsigned char data[64] = {0};
	unsigned short reg,addr;
	int length = 0,i = 0;
	
	switch(type)
	{
		data[0] = packet[4];
		data[1] = packet[5];
		case 1:
			reg = Read_FPGA(0x59);
			data[0] = reg >> 8 && 0xff;
			length = 3;
			break;
		case 2:
			reg = Read_FPGA(0x59);
			data[0] = reg && 0xff;
			length = 3;
			break;
		case 3:
			reg = Read_FPGA(0x60);
			data[0] = reg && 0xff;
			length = 3;
			break;
		case 4:
			reg = Read_FPGA(0x27);
			data[0] = reg && 0xff;
			length = 3;
			break;
		case 5:
			reg = Read_FPGA(2);
			data[2] = reg >>8 & 0xff;
			data[3] = reg &0xff;
			length = 4;
			break;
		case 6:
			reg = Read_FPGA(6);
			data[2] = reg >>8 & 0xff;
			data[3] = reg &0xff;
			length = 4;
			break;
		case 9:
			addr = 0x48;
			for(i = 0;i < 4;i++){
				reg = Read_FPGA(addr);
				data[2 + i*2] = reg >> 8& 0xff;
				data[2 + i*2 +1] = reg &0xff;
				addr += 2;
			}
			length = 10;
			break;
		case 10:
			addr = 0x30;
			for(i = 0;i <16;i++){
				reg = Read_FPGA(addr);
				data[2 + i*2] = reg >> 8& 0xff;
				data[2 + i*2 +1] = reg &0xff;
				addr += 1;
			}
			length = 34;
			break;
		default:
			encap_and_send_packet(NACK,NULL,0);
			return -1;
			break;		
	}
	encap_and_send_packet(REPORT_MONITOR,data,length);
	encap_and_send_packet(ACK,NULL,0);

	return 0;

}

void set_1pps_clock_source(unsigned char source)
{
	Write_FPGA(0x26,source);
}
void set_antenna_power(unsigned char source)
{
	Write_FPGA(0x43,source);
}
void set_fan_speed(unsigned char speed)
{
	Write_FPGA(0x46,speed);
}
void set_cm55_mode(unsigned char mode)
{
	Write_FPGA(0x57,mode);
}

int set_mcu(unsigned char* packet,int unpacket_len)
{
	unsigned short type = packet[4] << 8 | packet[5];
	switch(type)
	{
		case 0:
			set_1pps_clock_source(packet[6]);
			encap_and_send_packet(ACK,NULL,0);
			break;
		case 1:
			set_antenna_power(packet[6]);
			encap_and_send_packet(ACK,NULL,0);
			break;
		case 2:
			set_fan_speed(packet[6]);
			encap_and_send_packet(ACK,NULL,0);
			break;
		case 3:
			set_cm55_mode(packet[6]);
			encap_and_send_packet(ACK,NULL,0);
			break;
		default:
			encap_and_send_packet(NACK,NULL,0);
			break;
	}

	return 0;
}
