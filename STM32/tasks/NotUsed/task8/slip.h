#ifndef __SLIP_H_
#define __SLIP_H_

#define END          0300        /*CO indicates end of packet */
#define ESC          0333        /*DB indicates byte stuffing */
#define ESC_END      0334        /*DC DESC ESC_END means END data byte */
#define ESC_ESC      0335        /*DD ESC ESC_ESC means ESC data byte */

int  slipPacket
(
unsigned char *srcbuf ,            /*源buf*/
unsigned char *destbuf,            /*slip打包后的buf*/
unsigned int srclen               /*slip 打包前的长度*/
);

int  slipUnPacketPro
(
unsigned char *srcbuf ,            /*源buf*/
unsigned int srclen               /*slip 打包后的长度*/
);

#endif
