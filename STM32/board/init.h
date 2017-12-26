#ifndef __init_h__
#define __init_h__

void Init_CPU  		   (void);
void Init_BSPHardWare(void);
void Init_EXTHardWare(void);
/* CPU clock is 167MHz, to delay 1us need 167 CPU cycle */
void udelay(int useconds);


#endif/*__init_h__*/
