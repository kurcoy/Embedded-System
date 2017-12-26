#ifndef __CRC_CCITT_H_
#define __CRC_CCITT_H_

#define CRC_INIT		0xFFFF

unsigned short crc_ccitt(unsigned short crc, unsigned char const *buffer, int len);

#endif
