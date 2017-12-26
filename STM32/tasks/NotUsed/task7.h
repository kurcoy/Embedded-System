#ifndef __task7_h__
#define __task7_h__

#include "multi-task.h"

#define TASK7_STACK_SIZE	256
#define TASK7_PERIOD	20 /* 20Mms*/

extern StackType_t Task7Stack[TASK7_STACK_SIZE] CCM_RAM;  // Put task stack in CCM
extern StaticTask_t Task7Buffer CCM_RAM;  // Put TCB in CCM

void Task7(void* p);

typedef enum
{
    McuUart3Mux_None = 0,
    McuUart3Mux_GPS,
    McuUart3Mux_TOD,
    McuUart3Mux_CM55,
    McuUart3Mux_ACS9522
} McuUart3Mux_t;


typedef struct{
	int year;  
	int month; 
	int  day;
	int hour;
	int minute;
	int second;
}DATE_TIME;

typedef struct{
    short   Msg_NO;
    int     TxRxFlag;
    unsigned char   CStatus;
    unsigned char   TrackStatus;
    int         cPHDiff;
    double          cPWM1;
    double          cPWM2;
    int             SYNCNT;
    int             HCNT;
    double          HPAVG;
    double          VCH1;
    double          HPMOD;
    double          VCM10;
    double          inT;
    int             TcPHDiff;
    float           Version;
	DATE_TIME date;
    int             def_val;
}CM55_INFO;

typedef enum
{
    CM55_Request_Idle = 0,
	CM55_Request_cPHDiff,
}CM55_Request_t;

extern McuUart3Mux_t Fpga_CacheUart3Sel(McuUart3Mux_t route);

int CM55_Parse(char *line, CM55_INFO *CM55);

void Int_To_Str(int x,char *Str);
void CM55_Request(CM55_Request_t req);

typedef struct{
	uint16_t    year;  
	uint8_t     month; 
	uint8_t     day;
	uint8_t     hour;
	uint8_t     min;
	uint8_t     sec;
}UTC_TIME;


typedef struct{
    uint8_t subtype[128];
    uint8_t gnss_enabled;
    uint16_t noisePerMS;
    uint8_t hw_astatus;
    uint8_t hw_apower;
    uint8_t hw_rtccalib;
    uint8_t ext1_val;
    uint32_t pvt_tow;
    uint32_t tAcc;
    uint8_t fix_type; 
    uint8_t fix_flag; 
    uint8_t numSV;
    int32_t latitude;
    int32_t longtitude;
    int32_t altitude;
    int32_t hMSL;
    uint32_t hAcc;
    uint32_t vAcc;
    int32_t gSpeed;
    uint8_t svId[10];
    uint8_t cno[10];
    uint8_t gpsFix;
    uint8_t wknset;
    uint8_t towset;
    uint32_t ttff;
    uint32_t mass;
    uint32_t clock_tAcc;
    uint32_t clock_fAcc;
    UTC_TIME utc_time;
 
}gps_device_t;


#define UBX_PREFIX_LEN		6
#define UBX_CLASS_OFFSET	2
#define UBX_TYPE_OFFSET		3

#define UBX_MODE_NO_FIX                 0x00
#define UBX_MODE_DRONLY                 0x01
#define UBX_MODE_2D                     0x02
#define UBX_MODE_3D                     0x03
#define UBX_MODE_GNSS_DRCOMBINED        0x04
#define UBX_MODE_TMONLY                 0x05


#define UBX_MESSAGE_BASE_SIZE 6
#define UBX_MESSAGE_DATA_OFFSET UBX_MESSAGE_BASE_SIZE

#define UBX_A_STATUS_INIT               0x00
#define UBX_A_STATUS_DONTKNOW           0x01
#define UBX_A_STATUS_OK                 0x02
#define UBX_A_STATUS_SHORT              0x03
#define UBX_A_STATUS_OPEN               0x04

#define UBX_A_POWER_OFF                 0x00
#define UBX_A_POWER_ON                  0x01
#define UBX_A_POWER_DONTKNOW            0x02


#define UBX_CLASS_NAV   0x01         /**< Navigation */
#define UBX_CLASS_CFG   0x06         /**< Configuration requests */
#define UBX_CLASS_MON   0x0a         /**< System monitoring */
#define UBX_CLASS_ACK   0x05         /**< ACK */


#define UBX_ACK_ACK     0x01         /**< ACK */


#define UBX_NAV_STATUS                      0x03
#define UBX_NAV_PVT                         0x07
#define UBX_NAV_CLOCK                       0x22
#define UBX_NAV_SAT                         0x35

#define UBX_CFG_PRT		                    0x00
#define UBX_CFG_GNSS	                    0x3E
#define UBX_CFG_PWR		                    0x57
#define UBX_CFG_RST		                    0x04
#define UBX_CFG_MSG		                    0x01
#define UBX_CFG_CFG		                    0x09

#define UBX_MON_VER		                    0x04
#define UBX_MON_HW		                    0x09
#define UBX_MON_GNSS	                    0x28

#define GPS_MODE                    0X00
#define BeiDou_MODE                 0X01
#define GPS_BeiDou_MODE             0X02

#define GNSS_RUNNING                0x00
#define GNSS_STOPPED                0x01
#define SOFT_BACK_UP                0x02

#define HOTSTART                    0x00
#define WARMSTART                   0x01
#define COLDSTART                   0x02

#define CURRENT_CONFIG              0X00
#define DEFAULT_CONFIG              0X01

#define INPUT_ERROR                 0x04
#define UBX_MSGID(cls_, id_) (((cls_)<<8)|(id_))

typedef enum {
    MSGID_NAV_STATUS	= UBX_MSGID(UBX_CLASS_NAV, UBX_NAV_STATUS),
    MSGID_NAV_PVT	    = UBX_MSGID(UBX_CLASS_NAV, UBX_NAV_PVT),
    MSGID_NAV_CLOCK	    = UBX_MSGID(UBX_CLASS_NAV, UBX_NAV_CLOCK),
    MSGID_NAV_SAT	    = UBX_MSGID(UBX_CLASS_NAV, UBX_NAV_SAT),
 
    MSGID_MON_VER		= UBX_MSGID(UBX_CLASS_MON, UBX_MON_VER),
    MSGID_MON_HW		= UBX_MSGID(UBX_CLASS_MON, UBX_MON_HW),
    MSGID_MON_GNSS      = UBX_MSGID(UBX_CLASS_MON, UBX_MON_GNSS),
    
    MSGID_ACK_ACK      = UBX_MSGID(UBX_CLASS_ACK, UBX_ACK_ACK),

} ubx_message_t;


/* these are independent of byte order */
#define getsb(buf, off)	((int8_t)buf[off])
#define getub(buf, off)	((uint8_t)buf[off])
#define putbyte(buf,off,b) do {buf[off] = (unsigned char)(b);} while (0)

/* little-endian access */
#define getles16(buf, off)	((int16_t)(((uint16_t)getub((buf),   (off)+1) << 8) | (uint16_t)getub((buf), (off))))
#define getleu16(buf, off)	((uint16_t)(((uint16_t)getub((buf), (off)+1) << 8) | (uint16_t)getub((buf), (off))))
#define getles32(buf, off)	((int32_t)(((uint16_t)getleu16((buf),  (off)+2) << 16) | (uint16_t)getleu16((buf), (off))))
#define getleu32(buf, off)	((uint32_t)(((uint16_t)getleu16((buf),(off)+2) << 16) | (uint16_t)getleu16((buf), (off))))
#define getles64(buf, off)	((int64_t)(((uint64_t)getleu32(buf, (off)+4) << 32) | getleu32(buf, (off))))
#define getleu64(buf, off)	((uint64_t)(((uint64_t)getleu32(buf, (off)+4) << 32) | getleu32(buf, (off))))
extern float getlef32(const char *, int);
extern double getled64(const char *, int);

#define putle16(buf, off, w) do {putbyte(buf, (off)+1, (uint)(w) >> 8); putbyte(buf, (off), (w));} while (0)
#define putle32(buf, off, l) do {putle16(buf, (off)+2, (uint)(l) >> 16); putle16(buf, (off), (l));} while (0)

/* big-endian access */
#define getbes16(buf, off)	((int16_t)(((uint16_t)getub(buf, (off)) << 8) | (uint16_t)getub(buf, (off)+1)))
#define getbeu16(buf, off)	((uint16_t)(((uint16_t)getub(buf, (off)) << 8) | (uint16_t)getub(buf, (off)+1)))
#define getbes32(buf, off)	((int32_t)(((uint16_t)getbeu16(buf, (off)) << 16) | getbeu16(buf, (off)+2)))
#define getbeu32(buf, off)	((uint32_t)(((uint16_t)getbeu16(buf, (off)) << 16) | getbeu16(buf, (off)+2)))
#define getbes64(buf, off)	((int64_t)(((uint64_t)getbeu32(buf, (off)) << 32) | getbeu32(buf, (off)+4)))
#define getbeu64(buf, off)	((uint64_t)(((uint64_t)getbeu32(buf, (off)) << 32) | getbeu32(buf, (off)+4)))
extern float getbef32(const char *, int);
extern double getbed64(const char *, int);

#define putbe16(buf,off,w) do {putbyte(buf, (off), (w) >> 8); putbyte(buf, (off)+1, (w));} while (0)
#define putbe32(buf,off,l) do {putbe16(buf, (off), (l) >> 16); putbe16(buf, (off)+2, (l));} while (0)

extern void putbef32(char *, int, float);
extern void putbed64(char *, int, double);

extern void shiftleft(unsigned char *, int, unsigned short);

extern int ubx_cmd(uint32_t cmd, uint32_t class);



#endif/*__taskt_h__*/
