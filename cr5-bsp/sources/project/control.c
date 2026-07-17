#include "control.h"

//////////////////////////////////// <Function> //////////////////////////////////////////////////////

static void Motor_SetGpio(void)
{
   (void)GPIO_Config(ENA_PWM , (GPIO_FUNC(10UL) | GPIO_OUTPUT | GPIO_CD(0x3UL)));
   (void)GPIO_Config(IN_1_3 , (GPIO_FUNC(0UL) | GPIO_OUTPUT));
   (void)GPIO_Config(IN_2_4 , (GPIO_FUNC(0UL) | GPIO_OUTPUT));
   (void)GPIO_Config(Drive_State, (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN));
   (void)GPIO_Config(Rear_State , (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN));
   (void)GPIO_Set(IN_1_3 , 1UL);
   (void)GPIO_Set(IN_2_4 , 0UL);
}

void Motor_Init(PDMModeConfig_t *ModeConfigInfo) { // PDM 설정
    Motor_SetGpio(); // GPIO 설정
    (void)PDM_Init(); // PDM 초기화

    ModeConfigInfo->mcPortNumber = 65UL; // 65번 포트 PDM 활성화
    ModeConfigInfo->mcOperationMode = PDM_OUTPUT_MODE_PHASE_1; // PDM Mode 1로 설정
    ModeConfigInfo->mcClockDivide = 0UL;
    ModeConfigInfo->mcLoopCount = 0UL;
    ModeConfigInfo->mcInversedSignal = 0UL;

    ModeConfigInfo->mcPosition1 = 0UL;
    ModeConfigInfo->mcPosition2 = 0UL;
    ModeConfigInfo->mcPosition3 = 0UL;
    ModeConfigInfo->mcPosition4 = 0UL;

    ModeConfigInfo->mcOutPattern1 = 0UL;
    ModeConfigInfo->mcOutPattern2 = 0UL;
    ModeConfigInfo->mcOutPattern3 = 0UL;
    ModeConfigInfo->mcOutPattern4 = 0UL;
    ModeConfigInfo->mcMaxCount = 0UL;
}

void Motor_UpdateDuty(PDMModeConfig_t *ModeConfigInfo, uint32 speed) { // DutyCycle, Period 설정
    // uint32 duty_percentage;

    ModeConfigInfo->mcDutyNanoSec1 =  (speed * (1000UL * 1000UL)) / 255; // x값 0 ~ 255 입력에 따라 PDM값 조절
    ModeConfigInfo->mcPeriodNanoSec1 = 1000UL * 1000UL;

    // duty_percentage = (ModeConfigInfo->mcDutyNanoSec1 * 100UL) / ModeConfigInfo->mcPeriodNanoSec1;
    
    // mcu_printf("\n == Duty : %d%%\n", duty_percentage);  
}
void Motor_Option(uint8 gDriveMode, uint32 *speed) {
    static uint8 prev_mode = 0;  // 내부적으로 이전 상태를 관리

    // 방향 전환 감지 및 속도 초기화
    if (gDriveMode != prev_mode) {
        *speed = 0;  // 방향 전환 시 속도 초기화
    }

    // 상태에 따른 GPIO 설정
    switch (gDriveMode) {
        case 0:  // 정지 상태 (Parking)
            GPIO_Set(IN_2_4, 0UL);
            GPIO_Set(IN_1_3, 0UL);
            *speed = 0;  // 정지 시 속도 고정
            break;

        case 1:  // 정회전 (Drive)
            GPIO_Set(IN_2_4, 1UL);
            GPIO_Set(IN_1_3, 0UL);
            break;

        case 2:  // 역회전 (Reverse)
            GPIO_Set(IN_2_4, 0UL);
            GPIO_Set(IN_1_3, 1UL);
            break;

        default:  // 잘못된 입력 처리
            break;
    }

    // 이전 모드 업데이트
    prev_mode = gDriveMode;
}


void Motor_ConfigurePDM(PDMModeConfig_t *ModeConfigInfo) { // PDM 활성화
    (void)PDM_Disable((uint32)PDM_CH_0, PMM_ON);
    (void)PDM_SetConfig((uint32)PDM_CH_0, (PDMModeConfig_t *)ModeConfigInfo);
    (void)PDM_Enable((uint32)PDM_CH_0, PMM_ON);
}


