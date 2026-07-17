#include <debug.h>
#include <sal_api.h>
#include <main.h>
#include <stdint.h>
#include <gpio.h>
#include "project_autohold.h"

volatile int autohold = 0;

static void check_autohold(void *pArg)
{

    (void)pArg;
    mcu_printf("Start checking autohold\n");

    // GPIO 핀 초기화
    if (GPIO_Config(GPIO_GPG(10UL), (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN)) == SAL_RET_FAILED)
    {
        mcu_printf("Configuration failed.\n");
        return;
    }
    mcu_printf("Configuration succeeded.\n");


    // 버튼 상태 추적 변수
    uint32_t previous_pin_state = 0;  // 이전 상태를 저장 (0 = not pressed, 1 = pressed)

    while (1)
    {
        // 현재 버튼 상태 읽기
        uint32_t current_pin_state = GPIO_Get(GPIO_GPG(10UL));

        // 버튼이 이전에 눌려있지 않았고, 현재 눌려있다면 상승 에지 감지
        if (previous_pin_state == 0 && current_pin_state == 1)
        {
            mcu_printf("Button pressed.\n");
            autohold = !autohold;  // autohold 값 반전
            mcu_printf("autohold is now: %d\n", autohold);
        }

        // 버튼 상태 업데이트
        previous_pin_state = current_pin_state;

        // 10ms 간격으로 상태 확인
        (void)SAL_TaskSleep(110);
    }
}

////////////////////////////////////TASK Create///////////////////////////////////
void CHECK_AUTOHOLD_Task(void)
{
    mcu_printf("!!@#Check Autohold task test !!!@#@\n");

    static uint32   CHECK_AUTOHOLD_TaskID;
    static uint32   CHECK_AUTOHOLD_Stk[CHECK_AUTOHOLD_STK_SIZE];

    (void)SAL_TaskCreate
    (
        &CHECK_AUTOHOLD_TaskID,
        (const uint8 *)"Chechk Autohold Task",
        (SALTaskFunc)&check_autohold,
        &CHECK_AUTOHOLD_Stk[0],
        CHECK_AUTOHOLD_STK_SIZE,
        SAL_PRIO_AUTOHOLD, //우선순위는 알아서
        NULL
    );
}

