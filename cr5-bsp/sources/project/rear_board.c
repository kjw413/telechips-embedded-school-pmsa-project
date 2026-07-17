#include "rear_board.h"
#include "external_io.h"
#include "ultra_sonic.h"
#include "control.h"

#define MID_TX_BUFFER_SIZE (4UL)
#define MID_RX_BUFFER_SIZE (4UL)
#define UART_CH_FOR_MID    (1U)
#define UART_PROT_FOR_MID  (22U)

static uint32	gDistance;			// 차량과 장애물 간의 거리
static uint32 	gPwmDuty;			// PWM duty
static uint8	gDriveMode;			// 주행 모드

// 생성할 태스크
// static SALRetCode_t transferMidBoardTask(void *pArg);
// static SALRetCode_t sendToMidBoardTask(void *pArg);
static SALRetCode_t receiveFromMidBoardTask(void *pArg);
static void receiveFromTrasonicTask(void *pArg);
static void controlMotorTask(void *pArg);

PDMModeConfig_t ModeConfigInfo;
uint32 gRecvFromMidTaskEventId;

// 중앙 보드에서 실행될 태스크 모두 생성
SALRetCode_t createTask(void)
{
    uint32 ret = 0;
    static uint32   ULTRA_SONIC_TaskID;
    static uint32   ULTRA_SONIC_Stk[TASK_HIGH_STK_SIZE];

    static uint32   MOTOR_TaskID;
    static uint32   MOTOR_Task_Stk[TASK_HIGH_STK_SIZE];

    static uint32 receiveFromMidBoardTaskID;
    static uint32 receiveFromMidBoardTaskStk[TASK_MEDIUM_STK_SIZE];

    init_uart(UART_CH_FOR_MID, UART_PROT_FOR_MID); //중앙으로 보내는 uart 초기화

    ret = SAL_TaskCreate
    (
        &receiveFromMidBoardTaskID,
        (const uint8 *)"receive From Mid Board Task",
        (SALTaskFunc)&receiveFromMidBoardTask,
        &receiveFromMidBoardTaskStk[0],
        TASK_MEDIUM_STK_SIZE,
        3,
        NULL
    );
    
    if (ret != SAL_RET_SUCCESS) {
        mcu_printf("Task creation failed with error code: %d\n", ret);
        return ret;
    }

    ret = SAL_TaskCreate
    (
        &ULTRA_SONIC_TaskID,
        (const uint8 *)"Receive From Trasonic Task",
        (SALTaskFunc)&receiveFromTrasonicTask,
        &ULTRA_SONIC_Stk[0],
        TASK_MEDIUM_STK_SIZE,
        6,
        NULL
    );
    if(ret != SAL_RET_SUCCESS){
        mcu_printf("[DEBUG] Receive From UltraSonic Create Fail!!!!!!!!!\n");
        return ret;
    }

    ret = SAL_TaskCreate
    (
        &MOTOR_TaskID,
        (const uint8 *)"Start Motor Task",
        (SALTaskFunc)&controlMotorTask,
        &MOTOR_Task_Stk[0],
        TASK_MEDIUM_STK_SIZE,
        6,
        NULL
    );
    if (ret != SAL_RET_SUCCESS) {
        mcu_printf("Motor Task failed!!!!!!!!!!!!!!!!!!", ret);
        return ret;
    }

    return ret;
}

// static SALRetCode_t transferMidBoardTask(void *pArg){
//     SALRetCode_t ret;
//     uint8   ch = 1U;
//     uint8   portSel = 22U;
//     uint8   send_data[MID_TX_BUFFER_SIZE];
//     uint8   receive_data[MID_RX_BUFFER_SIZE];

//     init_uart(ch, portSel);

//     while(1){
//         (void)SAL_MemSet(send_data, 0, MID_TX_BUFFER_SIZE);
//         send_data[0] = (uint8)gDistance;
        
//         uart_send_data(ch, send_data, MID_TX_BUFFER_SIZE);

//         (void)SAL_MemSet(receive_data, 0, MID_RX_BUFFER_SIZE);
//         uart_receive_data(ch, receive_data, MID_RX_BUFFER_SIZE);

//         gPwmDuty = receive_data[0];
//         gDriveMode = receive_data[1];
//         gAutoholdMode = receive_data[2];

//         ret = SAL_TaskSleep(50); // sleep 0.1s
//         if(ret != SAL_RET_SUCCESS){
//             mcu_printf("transferMidBoardTask Fail!!!!!!!!!\n");
//             return ret;
//         }

//     }
// }

static SALRetCode_t receiveFromMidBoardTask(void *pArg)
{
    uint8   ch = UART_CH_FOR_MID;
    uint8   receive_data[MID_RX_BUFFER_SIZE];
    SALRetCode_t ret = SAL_RET_SUCCESS;
    uint32 flag = 0;

    uint32 startTick, midTick, endTick;

    (void)pArg;

    ret = SAL_EventCreate((uint32 *)&gRecvFromMidTaskEventId, \
                          (const uint8*)"receive signal event created", \
                          0);
    if(ret != SAL_RET_SUCCESS){
        mcu_printf("error for SAL_EventCreate!\n");
        return ret;
    }
    
    (void)SAL_MemSet(receive_data, 0, MID_RX_BUFFER_SIZE);

    while(1){
        (void)SAL_GetTickCount(&startTick);

        (void) SAL_EventGet(gRecvFromMidTaskEventId, 0x00000001UL, 0UL, (uint32)SAL_EVENT_OPT_CONSUME, &flag);

        (void)SAL_GetTickCount(&midTick);

        if ((flag & 0x00000001UL) != 0) // 비트가 설정되었는지 확인
        {
            uart_receive_data(ch, receive_data, MID_RX_BUFFER_SIZE);

            if(receive_data[0] == ID_ABNORMAL){
                gPwmDuty = 0;
                Motor_UpdateDuty(&ModeConfigInfo, gPwmDuty);
                GPIO_Set(IN_1_3, 0UL);
                GPIO_Set(IN_2_4, 0UL);
            }
            else if(receive_data[0] == ID_DATA){
                gPwmDuty = receive_data[1];
                gDriveMode = receive_data[2];
            }
            else{
                mcu_printf("invaild ID\n");
            }
        }

        (void)SAL_GetTickCount(&endTick);

        mcu_printf("[receiveFromMidBoardTask][ID: %d] Duty: %d, drive_mode: %d\n", receive_data[0], gPwmDuty, receive_data[2]);
        mcu_printf("- startTick: %d\n", startTick);
        mcu_printf("- midTick: %d\n", midTick);
        mcu_printf("- endTick: %d\n", endTick);
    }

    return ret;
}

static void receiveFromTrasonicTask(void *pArg)
{
    uint8   ch = UART_CH_FOR_MID; //중앙으로 향하는 채널
    uint8   send_data[MID_TX_BUFFER_SIZE];
    uint8   is_danger = 0;

    uint32 startTick, endTick;

    (void)pArg;

    ultrasonic_init();

    (void)SAL_MemSet(send_data, 0, MID_TX_BUFFER_SIZE);
    
    send_data[0] = ID_ULTRA_SONIC;

    while (1) {
        if(gDriveMode == R){
            (void)SAL_GetTickCount(&startTick);

            gDistance = get_distance();

            if(is_danger == IS_DANGER){
                if(gDistance >= DANGER_DISTANCE){
                    is_danger = IS_NOT_DANGER;
                    send_data[1] = is_danger;

                    (void)uart_send_data(ch, send_data, MID_TX_BUFFER_SIZE);
                }
            }
            else{
                if(gDistance < DANGER_DISTANCE){
                    is_danger = IS_DANGER;
                    send_data[1] = is_danger;

                    (void)uart_send_data(ch, send_data, MID_TX_BUFFER_SIZE);   
                }            
            }

            (void)SAL_GetTickCount(&endTick);

            mcu_printf("[rear][TrasonicTask] gDistance: %d\n", gDistance);
            mcu_printf("- startTick: %d\n", startTick);
            mcu_printf("- endTick: %d\n", endTick);
        }

        SAL_TaskSleep(1000);
    }
}

static void controlMotorTask(void *pArg)
{
    uint32 startTick, endTick;

    (void)pArg;

    // 모터 초기화
    Motor_Init(&ModeConfigInfo);

    while (1) {
        (void)SAL_GetTickCount(&startTick);

        // 상태 및 속도 적용
        Motor_Option(gDriveMode,&gPwmDuty);

        // PDM 듀티 업데이트
        Motor_UpdateDuty(&ModeConfigInfo, gPwmDuty);

        // PDM 설정 적용
        Motor_ConfigurePDM(&ModeConfigInfo);

        // 디버깅 메시지 출력

        // GPIO 상태 출력

        (void)SAL_GetTickCount(&endTick);

        mcu_printf("[rear][controlMotorTask] gDriveMode: %d, gPwmDuty: %d\n", gDriveMode, gPwmDuty);
        mcu_printf("- startTick: %d\n", startTick);
        mcu_printf("- endTick: %d\n", endTick);
        
        SAL_TaskSleep(1000);
    }
}


/*
static void controlMotorTask(void *pArg)
{
    (void)pArg;
    mcu_printf("Motor Task Start!!!\n");

    uint32 x = 0;            // 초기 속도
    gDriveMode = 1;    // 초기 DriveMode (정회전)
    uint8 increasing = 1;    // 속도 증가/감소 상태 플래그 (1: 증가, 0: 감소)
    PDMModeConfig_t ModeConfigInfo;

    // 모터 초기화
    Motor_Init(&ModeConfigInfo);

    while (1) {
        // 속도 업데이트
        if (increasing) {
            x += 4;  // 속도 증가
            if (x >= 255) {
                x = 255;     // 속도 상한 제한
                increasing = 0;  // 감소 상태로 전환
            }
        } else {
            x -= 4;  // 속도 감소
            if (x == 0) {
                increasing = 1;  // 증가 상태로 전환
                
                // gDriveMode 상태 순환
                if (gDriveMode == 0) {
                    gDriveMode = 1;  // 정지 → 정회전
                } else if (gDriveMode == 1) {
                    gDriveMode = 2;  // 정회전 → 역회전
                } else {
                    gDriveMode = 0;  // 역회전 → 정지
                }

                mcu_printf("Direction changed: gDriveMode = %d\n", gDriveMode);
            }
        }

        // 상태 및 속도 적용
        Motor_Option(gDriveMode, &x);

        // PDM 듀티 업데이트
        Motor_UpdateDuty(&ModeConfigInfo, x);

        // PDM 설정 적용
        Motor_ConfigurePDM(&ModeConfigInfo);

        // 디버깅 메시지 출력
        mcu_printf("Speed (x): %d, gDriveMode: %d, Increasing: %d\n", x, gDriveMode, increasing);

        // 300ms 대기
        SAL_TaskSleep(300);
    }
}
*/

/*
static void controlMotorTask(void *pArg)
{
    (void)pArg;
    mcu_printf("Motor Start!!!\n");
    uint32 x = 0;  // 초기값
    gDriveMode = 2;
    PDMModeConfig_t ModeConfigInfo;

    // 초기화
    Motor_Init(&ModeConfigInfo);

    while (1) {

        // 듀티 및 출력 업데이트
        Motor_UpdateDuty(&ModeConfigInfo, x);

        // PDM 설정
        Motor_ConfigurePDM(&ModeConfigInfo);

        // x 값 업데이트
	x += 4;
        // 상태 변화 처리
        Motor_Option(gDriveMode, &x);

        SAL_TaskSleep(300);  // 주기적으로 Sleep
    }
   
}
*/