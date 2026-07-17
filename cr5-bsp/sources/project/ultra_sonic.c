#include "ultra_sonic.h"
#include <debug.h>
#include <stdint.h>
#include <gpio.h>
#include <timer.h>
#include <timer_test.h>

static uint32 calculate_distance(uint32 tick) {
    // 클럭 값을 마이크로초로 변환 후 거리 계산
    uint32 us = tick;
    return (us * SONIC_SPEED) / 2.0;  // 왕복 거리이므로 나누기 2
}

uint32 get_distance() {
    timer();
    uint32 start = 0;
    uint32 end = 0;

    SAL_WriteReg(0x00000000UL, 0x1b935180UL);
    SAL_TaskSleep(50);
    // 트리거 신호 (10μs 동안 HIGH)
    SAL_WriteReg(0x00000040UL, 0x1b935180UL);
    us_delay(10);

    SAL_WriteReg(0x00000000UL, 0x1b935180UL);


    while (GPIO_Get(ECHO_PIN) == 0UL);
    start = read_timer();  // 타이머 값 읽기

    // 에코 핀이 LOW가 될 때까지 대기 (end 기록)
    while (GPIO_Get(ECHO_PIN) == 1UL);
    end = read_timer();    // 타이머 값 읽기

    uint32 us = end - start;
    TIMER_Disable(TIMER_CH_2);
    
    // 거리 계산
    return calculate_distance(us);
 //   mcu_printf("distance : %d cm\n", distance);
}

void ultrasonic_init(void)
{
    GPIO_Config(GPIO_GPG(6), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
    // 입력 핀 설정
    GPIO_Config(GPIO_GPG(7), (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN));
}
