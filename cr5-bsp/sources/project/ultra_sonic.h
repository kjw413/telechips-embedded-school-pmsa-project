#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <sal_api.h>

#define TRIGGER_PIN GPIO_GPG(6)  // 트리거 핀 (예: GPIO_GPG(7))
#define ECHO_PIN    GPIO_GPG(7)  // 에코 핀 (예: GPIO_GPG(6))
#define SONIC_SPEED 0.0343  // 음속 (cm/µs)

uint32 get_distance(void);
void ultrasonic_init(void);

#endif

