#include <debug.h>
#include <sal_api.h>
#include <gpio.h>
#include "project_drive_mode.h"
#include "project_ipc.h"


static void drive_mode(void *pArg)
{
    mcu_printf("Start checking drive mode\n");

    // GPIO 핀 초기화
    if (GPIO_Config(GPIO_GPG(1UL), (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN)) == SAL_RET_FAILED)
    {
        mcu_printf("Drive mode Configuration failed.\n");
        return;
    }
     if (GPIO_Config(GPIO_GPG(2UL), (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN)) == SAL_RET_FAILED)
    {
        mcu_printf("Drive mode Configuration failed.\n");
        return;
    }
    mcu_printf("Drive mode Configuration succeeded.\n");

    //0: P, 
    //1: D, 
    //2: R

    // drive mode 상태 확인 polling
    while (1) 
    {
        // 현재 버튼 상태 읽기
        uint32 D_state = GPIO_Get(GPIO_GPG(1UL));
        uint32 R_state = GPIO_Get(GPIO_GPG(2UL));

        // 버튼이 이전에 눌려있지 않았고, 현재 눌려있다면 상승 에지 감지
        if (D_state) 
        {
            mcu_printf("Drive mode is D\n");
            send_data |= (1 << 8);  // 1로 세팅
            send_data &= ~(1 << 9);
            mcu_printf("send data: %d\n", send_data >> 8);
        }
        else if (R_state)
        {
            mcu_printf("Drive mode is R\n");
            send_data |= (1 << 9);
            send_data &= ~(1 << 8);
            mcu_printf("send data: %d\n", send_data >> 8);
        }
        else
        {
            mcu_printf("Drive mode is P\n");
            send_data &= ~(1 << 9);
            send_data &= ~(1 << 8);
            mcu_printf("send data: %d\n", send_data >> 8);
        }

        // 10ms 간격으로 상태 확인
        (void)SAL_TaskSleep(100);
    }
}

////////////////////////////////////TASK Create///////////////////////////////////
void CreateDRIVEMODETask(void)
{
    static uint32   DRIVE_MODE_TaskID;
    static uint32   DRIVE_MODE_Stk[DRIVE_MODE_STK_SIZE];

    (void)SAL_TaskCreate
    (
        &DRIVE_MODE_TaskID,
        (const uint8 *)"Drive Mode Task",
        (SALTaskFunc)&drive_mode,
        &DRIVE_MODE_Stk[0],
        DRIVE_MODE_STK_SIZE,
        6,
        NULL
    );
}