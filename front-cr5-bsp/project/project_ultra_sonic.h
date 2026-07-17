
#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#define ULTRA_SONIC_STK_SIZE               (512UL)
#include <sal_internal.h>

extern volatile uint32 distanc;
void ULTRA_SONIC_Task(void);
#endif


