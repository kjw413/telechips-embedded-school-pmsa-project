#ifndef PROJECT_AUTOHOLD
#define PROJECT_AUTOHOLD

#define CHECK_AUTOHOLD_STK_SIZE               (512UL)
#include <sal_internal.h>

extern volatile int autohold;
void CHECK_AUTOHOLD_Task(void);

#endif

