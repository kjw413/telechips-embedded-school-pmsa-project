#include "project_ultra_sonic.h" 
#include <debug.h>
#include <sal_api.h>
#include <main.h>
#include <stdint.h>
#include <gpio.h>
#include <timer.h>
#include <timer_test.h>
#define TRIGGER_PIN GPIO_GPG(6)  // 트리거 핀 (예: GPIO_GPG(7))
#define ECHO_PIN    GPIO_GPG(7)  // 에코 핀 (예: GPIO_GPG(6))
#define SONIC_SPEED 0.0343  // 음속 (cm/µs)

volatile uint32 distance = 0;
static void Ultra_main(void *pArg);
uint32 calculate_distance(uint32 tick);
static void get_distance(void) ;

uint32 calculate_distance(uint32 tick) {
    // 클럭 값을 마이크로초로 변환 후 거리 계산
    uint32 us = tick;
    return (us * SONIC_SPEED) / 2.0;  // 왕복 거리이므로 나누기 2
}

static void get_distance() {
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
    distance = (calculate_distance(us));
    mcu_printf("dist : %d cm\n", distance);
}

static void Ultra_main(void *pArg)
{
    (void)pArg;
    mcu_printf("ultra test start\n");
    
    //ultrasonic_init();
    GPIO_Config(GPIO_GPG(6), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
    // 입력 핀 설정
    GPIO_Config(GPIO_GPG(7), (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN));    
    
    while (1) {        
        get_distance();
        SAL_TaskSleep(1000); 
    }
}


void ULTRA_SONIC_Task(void)
{   
    mcu_printf("Ultra_Sonic start!!!\n");
    
    static uint32   ULTRA_SONIC_TaskID;
    static uint32   ULTRA_SONIC_Stk[ULTRA_SONIC_STK_SIZE];

    (void)SAL_TaskCreate
    (
        &ULTRA_SONIC_TaskID,
        (const uint8 *)"Ultra SONIC Task",
        (SALTaskFunc)&Ultra_main,
        &ULTRA_SONIC_Stk[0],
        ULTRA_SONIC_STK_SIZE,
        SAL_PRIO_TEST,
        NULL
    );
}
