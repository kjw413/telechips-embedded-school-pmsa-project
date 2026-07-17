#include <gpio.h>
#include <debug.h>
#include "project_autohold.h"
#include "project_ipc.h"

static void check_autohold(void *pArg)
{
    // GPIO 핀 초기화
    if (GPIO_Config(GPIO_GPG(6UL), (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN)) == SAL_RET_FAILED)
    {
        mcu_printf("Configuration failed.\n");
        return;
    }
    mcu_printf("Configuration succeeded.\n");


    // 버튼 상태 추적 변수
    uint32 previous_pin_state = 0;  // 이전 상태를 저장 (0 = not pressed, 1 = pressed)

    while (1) 
    {
        // 현재 버튼 상태 읽기
        uint32 current_pin_state = GPIO_Get(GPIO_GPG(6UL));
        //mcu_printf("autohold is now: %d\n", current_pin_state);

        // 버튼이 이전에 눌려있지 않았고, 현재 눌려있다면 상승 에지 감지
        if (current_pin_state == 1 && previous_pin_state == 0) 
        {
            send_data ^= (1);  // autohold 값 반전
            //mcu_printf("autohold is now: %d\n", send_data & 1);
        }
        mcu_printf("autohold is now: %d\n", send_data & 1);


        // 버튼 상태 업데이트
        previous_pin_state = current_pin_state;

        // 0.1s 간격으로 상태 확인
        (void)SAL_TaskSleep(100);
    }
}

////////////////////////////////////TASK Create///////////////////////////////////
void CreateAUTOHOLDTask(void)
{
    static uint32   CHECK_AUTOHOLD_TaskID;
    static uint32   CHECK_AUTOHOLD_Stk[CHECK_AUTOHOLD_STK_SIZE];

    (void)SAL_TaskCreate
    (
        &CHECK_AUTOHOLD_TaskID,
        (const uint8 *)"Autohold Task",
        (SALTaskFunc)&check_autohold,
        &CHECK_AUTOHOLD_Stk[0],
        CHECK_AUTOHOLD_STK_SIZE,
        6,
        NULL
    );
}