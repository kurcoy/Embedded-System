#ifndef __SLIP_H_
#define __SLIP_H_

#define END          0300        /*CO indicates end of packet */
#define ESC          0333        /*DB indicates byte stuffing */
#define ESC_END      0334        /*DC DESC ESC_END means END data byte */
#define ESC_ESC      0335        /*DD ESC ESC_ESC means ESC data byte */

int  slipPacket
(
unsigned char *srcbuf ,            /*Դbuf*/
unsigned char *destbuf,            /*slip������buf*/
unsigned int srclen               /*slip ���ǰ�ĳ���*/
);

int  slipUnPacketPro
(
unsigned char *srcbuf ,            /*Դbuf*/
unsigned int srclen               /*slip �����ĳ���*/
);

#endif
