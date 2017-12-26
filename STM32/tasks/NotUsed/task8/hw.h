#ifndef __HW_H_
#define __HW_H_

#include "RTC3232_task.h"

int report_warning(Time_Typedef start,Time_Typedef end);
int delete_warning(Time_Typedef start,Time_Typedef end);
int report_monitor(unsigned char* packet,int len);
int set_mcu(unsigned char* packet,int unpacket_len);

#endif
