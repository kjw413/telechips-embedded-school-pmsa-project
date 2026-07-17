#include "front_board.h"
#include "external_io.h"
#include "internal_io.h"
#include "ultra_sonic.h"
#include "control.h"

#define MID_TX_BUFFER_SIZE (4UL)
#define MID_RX_BUFFER_SIZE (4UL)
#define UART_CH_FOR_MID    (2U)
#define UART_PROT_FOR_MID  (22U)


uint32   gAutoholdMode;

static uint32	gDistance;			// 차량과 장애물 간의 거리
static uint32 	gPwmDuty;			// PWM duty
static uint8	gDriveMode;			// 주행 모드

// 생성할 태스크
static SALRetCode_t receiveFromMidBoardTask(void *pArg);
static void receiveFromTrasonicTask(void *pArg);
static SALRetCode_t receiveFromMainCoreTask(void);
static void controlMotorTask(void *pArg);

// uint32 gIsLedLight = 0;
PDMModeConfig_t ModeConfigInfo;
uint32 gRecvFromMidTaskEventId;

// 중앙 보드에서 실행될 태스크 모두 생성
SALRetCode_t createTask(void)
{
    SALRetCode_t ret = 0;

    static uint32   receiveFromMidBoard_TaskID;
    static uint32   receiveFromMidBoard_Stk[TASK_MEDIUM_STK_SIZE];

    // static uint32   sendToMidBoardTask_TaskID;
    // static uint32   sendToMidBoardTask_Stk[TASK_MEDIUM_STK_SIZE];

    static uint32   ULTRA_SONIC_TaskID;
    static uint32   ULTRA_SONIC_Stk[TASK_HIGH_STK_SIZE];

    static uint32   MOTOR_TaskID;
    static uint32   MOTOR_Task_Stk[TASK_HIGH_STK_SIZE];

    // ipc
    ret = receiveFromMainCoreTask();
    if(ret != SAL_RET_SUCCESS) {
        mcu_printf("error for receiveFromMainCoreTask()\n");
        return ret;
    }

    init_uart(UART_CH_FOR_MID, UART_PROT_FOR_MID); //중앙으로 보내는 uart 초기화

    // receive From Mid Board Task
    ret = SAL_TaskCreate
    (
        &receiveFromMidBoard_TaskID,
        (const uint8 *)"receive From Mid Board Task",
        (SALTaskFunc)&receiveFromMidBoardTask,
        &receiveFromMidBoard_Stk[0],
        TASK_MEDIUM_STK_SIZE,
        3,
        NULL
    );
    if(ret != SAL_RET_SUCCESS) {
        mcu_printf("error for receiveFromMidBoardTask()\n");
        return ret;
    }

    // ultra_sonic
    ret = SAL_TaskCreate
    (
        &ULTRA_SONIC_TaskID,
        (const uint8 *)"Ultra SONIC Task",
        (SALTaskFunc)&receiveFromTrasonicTask,
        &ULTRA_SONIC_Stk[0],
        TASK_HIGH_STK_SIZE,
        6,
        NULL
    );
    if(ret != SAL_RET_SUCCESS) {
        mcu_printf("error for receiveFromTrasonicTask()\n");
        return ret;
    }

    // Mortor Control
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
                gAutoholdMode = receive_data[3];
            }
            else{
                mcu_printf("invaild ID\n");
            }
        }

        (void)SAL_GetTickCount(&endTick);

        mcu_printf("[receiveFromMidBoardTask][ID: %d] Duty: %d, drive_mode: %d, autohold: %d\n", receive_data[0], gPwmDuty, receive_data[2], receive_data[3]);
        mcu_printf("- startTick: %d\n", startTick);
        mcu_printf("- midTick: %d\n", midTick);
        mcu_printf("- endTick: %d\n", endTick);
    }


    return ret;
}

// static SALRetCode_t sendToMidBoardTask(void *pArg)
// {
//     uint8   ch = UART_CH_FOR_MID; //중앙으로 향하는 채널
//     uint8   send_data[MID_TX_BUFFER_SIZE];
//     SALRetCode_t ret;
//     ret = SAL_RET_SUCCESS;

//     (void)SAL_MemSet(send_data, 0, MID_TX_BUFFER_SIZE);
//     while(1)
//     {
//         send_data[0] = gDistance;
//         send_data[1] = gIsLedLight;

//         (void)uart_send_data(ch, send_data, MID_TX_BUFFER_SIZE); //전방에 전달
//         SAL_TaskSleep(100);
//     }

//     return ret;
// }

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
        if(gDriveMode == D){
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

            mcu_printf("[TrasonicTask] is_danger: %d\n", is_danger);
            mcu_printf("- startTick: %d\n", startTick);
            mcu_printf("- endTick: %d\n", endTick);
        }

        SAL_TaskSleep(1000);
    }
}

static SALRetCode_t receiveFromMainCoreTask(void)
{
    if(ipc_init() < 0){
        mcu_printf("error for ipc_init()!\n");
        return SAL_RET_FAILED;
    }

    return (SALRetCode_t)ipc_create_receive_task();
}

static void controlMotorTask(void *pArg)
{
    uint32 startTick, endTick;

    (void)pArg;
    //mcu_printf("Motor Start!!!\n");
    //uint32 x = 127;  // 초기값
    gDriveMode = 255;

    // 초기화
    Motor_Init(&ModeConfigInfo);

    while (1) {
        (void)SAL_GetTickCount(&startTick);

       // GPIO_Set_Switch();  // 상태 확인

        // 듀티 및 출력 업데이트
        Motor_UpdateDuty(&ModeConfigInfo, gPwmDuty);

        // PDM 설정
        Motor_ConfigurePDM(&ModeConfigInfo);

        // x 값 업데이트

        // 상태 변화 처리
        Motor_Option(gDriveMode, &gPwmDuty);

        (void)SAL_GetTickCount(&endTick);

        // mcu_printf("[front][controlMotorTask] gDriveMode: %d, gPwmDuty: %d\n", gDriveMode, gPwmDuty);
        // mcu_printf("- startTick: %d\n", startTick);
        // mcu_printf("- endTick: %d\n", endTick);
        
        SAL_TaskSleep(1000);
    }
   
}
