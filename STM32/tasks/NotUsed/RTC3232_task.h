#ifndef __RTC3232_task_h__
#define __RTC3232_task_h__

#include "multi-task.h"
#include "stdint.h"
#define DS3232_STACK_SIZE	256
#define DS3232_ADDRESS	(uint8_t)(0x68)
#define I2C2_ITSELF_ADDRESS	(0x5a)
//#define I2C_TIMEOUT_MAX		(1000000)

typedef struct
{
	uint8_t addr;
	uint8_t value;
}DS3232_RegTable_t;

typedef struct
{
	uint16_t year;	//��
	uint8_t month;	//��
	uint8_t week;	//����
	uint8_t date;	//��
	uint8_t hour;	//Сʱ
	uint8_t minute;	//����
	uint8_t second;	//����
}Time_Typedef;
/******************************************************************************
                              �����Ĵ�����ַ�궨��                    
******************************************************************************/

#define Address_second					0x00	//��
#define Address_minute					0x01	//��
#define Address_hour					0x02	//ʱ
#define Address_week					0x03	//����
#define Address_date					0x04	//��
#define Address_month					0x05	//��
#define Address_year					0x06	//��

#define Address_second_Alarm1			0x07	//������
#define Address_minute_Alarm1			0x08	//������
#define Address_hour_Alarm1				0x09	//ʱ����
#define Address_week_Alarm1				0x0a	//�����塢��������

#define Address_minute_Alarm2			0x0b	//������
#define Address_hour_Alarm2				0x0c	//ʱ����
#define Address_week_Alarm2				0x0d	//�����塢��������


#define Address_control					0x0e	//����
#define Address_control_status			0x0f	//���ƺ�״̬��־

#define Address_offset					0x10	//Aging Offset

#define Address_temp_MSB				0x11	//�¶ȸ�8λ
#define Address_temp_LSB				0x12	//�¶ȵ�8λ

#define Shield_secondBit			0x7f
#define Shield_minuteBit			0x7f
#define Shield_hourBit				0x3f
#define Shield_weekBit				0x07
#define Shield_dateBit				0x3f
#define Shield_monthBit				0x1f
#define DS3232_YEARDATA                  (u16)0x2000
//Сʱ�Ĵ���
#define Hour_Mode12					(1<<6)	//12Сʱ��ʽ
#define Hour_Mode24					(0<<6)	//24Сʱ��ʽ

//������Ĵ���1
#define Alarm_second_open			(1<<7)	//�����忪
#define Alarm_second_close			(0<<7)	//�������

//������Ĵ���1&2
#define Alarm_minute_open			(1<<7)	//�����忪
#define Alarm_minute_close			(0<<7)	//�������

//ʱ����Ĵ���1&2��������Ϊ12Сʱģʽ����24Сʱģʽ��
#define Alarm_hour_open12			(3<<6)	//ʱ���忪��12Сʱ��ʽ
#define Alarm_hour_close12			(1<<6)	//ʱ����أ�24Сʱ��ʽ

#define Alarm_hour_open24			(2<<6)	//ʱ���忪��12Сʱ��ʽ
#define Alarm_hour_close24			(0<<6)	//ʱ����أ�24Сʱ��ʽ

//���ڡ�������Ĵ���1&2����ѡ��������������������壩
#define Alarm_week_open				(3<<6)	//�������忪
#define Alarm_week_close			(1<<6)	//���������

#define Alarm_date_open				(2<<6)	//�������忪
#define Alarm_date_close			(0<<6)	//���������


//������ƼĴ���
#define OSC_Enable					(0<<7)	//��������
#define OSC_Disable					(1<<7)	//ֹͣ����

#define SET_BBSQW					(1<<6)	//Vbat pin
#define RESET_BBSQW					(0<<6)	//int/sqw����

#define Temp_CONV_SET				(1<<5)	//ǿ���¶�ת��λ������
#define Temp_CONV_Clear				(0<<5)

#define SQW_OUT1Hz					(0<<3)	//1Hz
#define SQW_OUT1024Hz				(1<<3)	//1.024KHz
#define SQW_OUT4096Hz				(2<<3)	//4.096KHz
#define SQW_OUT8192Hz				(3<<3)	//8.192KHz

#define OUTPUT_INTSQW				(0<<2)	//����������ϵ��λ��1��INTCNλ��

#define A2IE_Enable					(1<<1)	//enable alarm 2
#define A2IE_Disable				(0<<1)	//disable alarm 2

#define A1IE_Enable					(1<<0)	//enable alarm 1
#define A1IE_Disable				(0<<0)	//disable alarm 1

//control and status register
#define Clear_OSF_Flag				(0<<7)	//clear OSF flag

#define Enable_OSC32768				(1<<3)	//EN32KHz EN
#define Disable_OSC32768			(0<<3)	//EN32KHz����

#define Clear_A2IE_Flag				(0<<1)	//�������2�жϱ�־
#define Clear_A1IE_Flag				(0<<0)	//�������2�жϱ�־E
extern int RTCDS3232_Time_Init(Time_Typedef *TimeVAL);
extern uint32_t  RTC3232_Get_Time(Time_Typedef  *TimeVAL);
extern  uint32_t RTC_Check(void);
extern  uint32_t RTC_EOSC_Enable(void);
 

#endif /*__RTC3232_task__*/
