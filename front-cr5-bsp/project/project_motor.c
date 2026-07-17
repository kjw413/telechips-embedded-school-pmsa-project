#include "pdm.h"
#include "motor.h"
#include "gpio.h"
#include "debug.h"
#include "sal_internal.h"
#include "bsp.h"
#include <sal_api.h>
#include <main.h>
#include <ultra_sonic.h>

///////////////////////////////////// <Declaration> /////////////////////////////////////////////////

static void Motor(void *pArg);
static void Motor_SetGpio(void);
void GPIO_Set_Switch(void);
static void PWM_Setting(void);
void Motor_Init(PDMModeConfig_t *ModeConfigInfo);
void Motor_UpdateDuty(PDMModeConfig_t *ModeConfigInfo, uint32 x);
void Motor_HandleStateChange(uint8 *prev_state, uint32 *x);
void Motor_ConfigurePDM(PDMModeConfig_t *ModeConfigInfo);

extern volatile uint32 distance;
volatile uint8 motor_state=1;

#define PDM_CH_0                        (0UL)
#define Drive_State			GPIO_GPG(1UL)
#define Rear_State			GPIO_GPG(2UL)
#define ENA_PWM				GPIO_GPC(29UL)
#define IN_1_3				GPIO_GPC(5UL)
#define IN_2_4				GPIO_GPC(4UL)

//////////////////////////////////// <Function> //////////////////////////////////////////////////////

void GPIO_Set_Switch(void) {
    uint8 drive_state = GPIO_Get(Drive_State); 
    uint8 rear_state = GPIO_Get(Rear_State); 

    if (drive_state == 1 && rear_state == 0) {
        motor_state = 1;
    } else if (rear_state == 1 && drive_state == 0) {
        motor_state = 2;
    } else if (drive_state == 0 && rear_state == 0) {
        motor_state = 0;
    }
}


static void Motor_SetGpio
(
    void
)
{
   (void)GPIO_Config(ENA_PWM , (GPIO_FUNC(10UL) | GPIO_OUTPUT | GPIO_CD(0x3UL)));
   (void)GPIO_Config(IN_1_3 , (GPIO_FUNC(0UL) | GPIO_OUTPUT));
   (void)GPIO_Config(IN_2_4 , (GPIO_FUNC(0UL) | GPIO_OUTPUT));
   (void)GPIO_Config(Drive_State, (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN));
   (void)GPIO_Config(Rear_State , (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN));
   (void)GPIO_Set(IN_1_3 , 1UL);
   (void)GPIO_Set(IN_2_4 , 0UL);
}
void Motor_Init(PDMModeConfig_t *ModeConfigInfo);
void Motor_UpdateDuty(PDMModeConfig_t *ModeConfigInfo, uint32 x);
void Motor_HandleStateChange(uint8 *prev_state, uint32 *x);
void Motor_ConfigurePDM(PDMModeConfig_t *ModeConfigInfo);

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

void Motor_UpdateDuty(PDMModeConfig_t *ModeConfigInfo, uint32 x) { // DutyCycle, Period 설정 
    ModeConfigInfo->mcDutyNanoSec1 =  (x * (1000UL * 1000UL)) / 255; // x값 0 ~ 255 입력에 따라 PDM값 조절
    ModeConfigInfo->mcPeriodNanoSec1 = 1000UL * 1000UL;

    uint32 duty_percentage = (ModeConfigInfo->mcDutyNanoSec1 * 100UL) / ModeConfigInfo->mcPeriodNanoSec1;
    mcu_printf("\n == Duty : %d%% , Distance : %d\n", duty_percentage, distance);
}

void Motor_HandleStateChange(uint8 *prev_state, uint32 *x) {
    uint8 local_motor_state = motor_state;  // 전역 변수 motor_state를 직접 참조

    if (*prev_state != local_motor_state) {
        if ((*prev_state == 0 && local_motor_state == 1) || 
            (*prev_state == 0 && local_motor_state == 2)) {
            *x = 127;  // 상태 변화 시 x 초기화
            mcu_printf("x has been reset to 127 due to state change: %d -> %d\n", *prev_state, local_motor_state);
        }
        if (local_motor_state == 1) { // motor state가 1일 때 Drive
            GPIO_Set(IN_2_4, 1UL);
            GPIO_Set(IN_1_3, 0UL);
            mcu_printf("state : %d -> %d (Drive)\n", *prev_state, local_motor_state);
        } else if (local_motor_state == 2) { // motor state 가 2일 때 Rear
            GPIO_Set(IN_2_4, 0UL);
            GPIO_Set(IN_1_3, 1UL);
            mcu_printf("state : %d -> %d (Reverse)\n", *prev_state, local_motor_state);
        } else { // Parking
            GPIO_Set(IN_2_4, 0UL);
            GPIO_Set(IN_1_3, 0UL);
            mcu_printf("state : %d -> %d (Parking)\n", *prev_state, local_motor_state);
        }

        *prev_state = local_motor_state;  // 상태 업데이트
    }
    if(*x == 255){
	    *x = 127;
    }
}

void Motor_ConfigurePDM(PDMModeConfig_t *ModeConfigInfo) { // PDM 활성화
    (void)PDM_Disable((uint32)PDM_CH_0, PMM_ON);
    (void)PDM_SetConfig((uint32)PDM_CH_0, (PDMModeConfig_t *)ModeConfigInfo);
    (void)PDM_Enable((uint32)PDM_CH_0, PMM_ON);
}

static void Motor(void *pArg) {
    (void)pArg;

    uint32 x = 127;  // 초기값
    uint8 prev_state = 255;
    PDMModeConfig_t ModeConfigInfo;

    // 초기화
    Motor_Init(&ModeConfigInfo);

    while (1) {
        GPIO_Set_Switch();  // 상태 확인

        // 듀티 및 출력 업데이트
        Motor_UpdateDuty(&ModeConfigInfo, x);

        // PDM 설정
        Motor_ConfigurePDM(&ModeConfigInfo);

        // x 값 업데이트
        x += 4;

        // 상태 변화 처리
        Motor_HandleStateChange(&prev_state, &x);

        SAL_TaskSleep(500);  // 주기적으로 Sleep
    }
}

void Motor_Task(void){   
    mcu_printf("Motor start!!!\n");
    
    static uint32   Motor_TaskID;
    static uint32   Motor_Stk[Motor_STK_SIZE];

    (void)SAL_TaskCreate
    (
        &Motor_TaskID,
        (const uint8 *)"Motor Task",
        (SALTaskFunc)&Motor,
        &Motor_Stk[0],
        Motor_STK_SIZE,
        SAL_PRIO_TEST1,
        NULL
    );
}


