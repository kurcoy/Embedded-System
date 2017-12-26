#ifndef __bsp_LED_h__
#define __bsp_LED_h__

#include "stm32f10x.h"

void Init_LED (void);

void Led1Set	(int level);
void Led2Set	(int level);

#endif/*__bsp_LED__*/
