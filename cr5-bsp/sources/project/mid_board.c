#include "mid_board.h"
#include "external_io.h"
#include "internal_io.h"
#include "sensor_io.h"
#include "abnormal.h"

/*-------------------receiveFromPedalTask--------------------------------*/
// tcc8050
#define I2C_PCF8591_SCL                 (GPIO_GPC(20UL))
#define I2C_PCF8591_SDA                 (GPIO_GPC(21UL))

#define CONTROL_DATA                    ((uint8)0x44)
#define I2C_CLK_RATE_100                (100)

// accel_data를 0~127로 맞추기 위한 offset
#define ACCEL_OFFSET    (127)
// break_data를 0~127로 맞추기 위한 offset
#define BREAK_OFFSET    (121)
// 최대 입력 값: 127
#define INPUT_MAX       (127)
// 최소 입력 값: 0
#define INTPUT_MIN       (0)
#define IDLE_PWM        (127)

uint32 gRecvFromMidTaskEventId;

static int8 is_abnormal;

static uint32   gIsRedLight;
static uint32   gPressAccel;
static uint32   gPressBreak;
static uint32   gAccelPercent;      // 악셀 페달 값
static int     gPwmDuty;             // PWM duty
/*----------------------receiveFromPedalTask-----------------------------*/
/*------------------------updateAutoholdTask-----------------------------*/
#define AUTOHOLD_PIN (GPIO_GPG(6UL))

static uint8   gAutoholdMode;      // 오토홀드 모드
/*------------------------updateAutoholdTask-----------------------------*/
/*------------------------updateDriveModeTask-----------------------------*/
#define D_MODE_PIN (GPIO_GPG(1UL))
#define R_MODE_PIN (GPIO_GPG(2UL))

static uint8   gDriveMode;         // 주행 모드
/*------------------------updateDriveModeTask-----------------------------*/
/*---------------------------ToBoardTask------------------------------*/
#define FRONT_TX_BUFFER_SIZE (4UL)
#define REAR_TX_BUFFER_SIZE  (4UL)
#define FRONT_RX_BUFFER_SIZE (4UL)
#define REAR_RX_BUFFER_SIZE  (4UL)
#define UART_CH_FOR_FRONT    (2U)
#define UART_CH_FOR_REAR     (1U)
#define UART_PROT_FOR_FRONT  (24U)
#define UART_PROT_FOR_REAR   (22U)
/*-------------------------receiveToBoardTask----------------------------*/

static uint32   gDistance;         // 차량과 장애물 간의 거리

// 생성할 태스크
static SALRetCode_t receiveFromPedalTask(void *pArg);
static SALRetCode_t receiveFromFrontBoardTask(void *pArg);
static SALRetCode_t receiveFromRearBoardTask(void *pArg);
static SALRetCode_t sendToBoardTask(void *pArg);
static SALRetCode_t sendToMainCoreTask(void *pArg);
static SALRetCode_t checkAbnormalActionTask(void *pArg);
static SALRetCode_t updateDriveModeTask(void *pArg);
static SALRetCode_t updateAutoholdTask(void *pArg);


// 중앙 보드에서 실행될 태스크 모두 생성
SALRetCode_t createTask(void)
{
    SALRetCode_t ret = 0;

    // receiveFromPeadalTask
    static uint32   receiveFromPedalTaskID = 0UL;
    static uint32   receiveFromPedalTaskStk[TASK_MEDIUM_STK_SIZE];

    // updateDriveModeTask
    static uint32   updateDriveModeTaskID;
    static uint32   updateDriveModeTaskStk[TASK_NORMAL_STK_SIZE];

    // updateAutoholdTask
    static uint32   updateAutoholdTaskID;
    static uint32   updateAutoholdTaskStk[TASK_NORMAL_STK_SIZE];

    // ProjectIPCTask
    static uint32   ProjectIPCTaskID;
    static uint32   ProjectIPCTaskStk[TASK_MEDIUM_STK_SIZE];
    
    // checkAbnormalAction
    static uint32 checkAbnormalActionTaskID;
    static uint32 checkAbnormalActionTaskStk[TASK_MEDIUM_STK_SIZE];

    // sendToBoardTask
    static uint32   sendToBoardTaskID;
    static uint32   sendToBoardTaskStk[TASK_MEDIUM_STK_SIZE];

    // receiveToRearBoardTask
    static uint32   receiveFromFrontBoardTaskID;
    static uint32   receiveFromFrontBoardTaskStk[TASK_MEDIUM_STK_SIZE];

    // receiveToRearBoardTask
    static uint32   receiveFromRearBoardTaskID;
    static uint32   receiveFromRearBoardTaskStk[TASK_MEDIUM_STK_SIZE];
    
    IPC_Create();

    ret = SAL_EventCreate((uint32 *)&gRecvFromMidTaskEventId, \
                          (const uint8*)"receive signal event created", \
                          0);
    if(ret != SAL_RET_SUCCESS){
        mcu_printf("error for SAL_EventCreate!\n");
        return ret;
    }

    ret = SAL_TaskCreate
    (
        &checkAbnormalActionTaskID,
        (const uint8 *)"Check Abnormal Action Task",
        (SALTaskFunc)&checkAbnormalActionTask,
        &checkAbnormalActionTaskStk[0],
        TASK_MEDIUM_STK_SIZE,
        6,
        NULL
    );
    if (ret != SAL_RET_SUCCESS) {
        mcu_printf("Task creation failed with error code: %d\n", ret);
        return ret;
    }

    ret = SAL_TaskCreate
    (
        &receiveFromPedalTaskID,
        (const uint8 *)"Receive From Pedal Task",
        (SALTaskFunc)&receiveFromPedalTask,
        &receiveFromPedalTaskStk[0],
        TASK_MEDIUM_STK_SIZE,
        6,
        NULL
    );
    if(ret != SAL_RET_SUCCESS){
        mcu_printf("[DEBUG] Receive From Pedal Task Create Fail!!!!!!!!!\n");
        return ret;
    }

    ret = SAL_TaskCreate
    (
        &updateDriveModeTaskID,
        (const uint8 *)"Update Drive Mode Task",
        (SALTaskFunc)&updateDriveModeTask,
        &updateDriveModeTaskStk[0],
        TASK_NORMAL_STK_SIZE,
        6,
        NULL
    );
    if (ret != SAL_RET_SUCCESS) {
        mcu_printf("Task creation failed with error code: %d\n", ret);
        return ret;
    }

    ret = SAL_TaskCreate
    (
        &updateAutoholdTaskID,
        (const uint8 *)"Update Autohold Task",
        (SALTaskFunc)&updateAutoholdTask,
        &updateAutoholdTaskStk[0],
        TASK_NORMAL_STK_SIZE,
        6,
        NULL
    );
    if (ret != SAL_RET_SUCCESS) {
        mcu_printf("Task creation failed with error code: %d\n", ret);
        return ret;
    }

    init_uart(UART_CH_FOR_FRONT, UART_PROT_FOR_FRONT); //전방으로 보내는 uart 초기화
    init_uart2(UART_CH_FOR_REAR, UART_PROT_FOR_REAR);   //후방으로 보내는 uart 초기화

    ret = SAL_TaskCreate
    (
        &sendToBoardTaskID,
        (const uint8 *)"Send To Board Task",
        (SALTaskFunc)&sendToBoardTask,
        &sendToBoardTaskStk[0],
        TASK_MEDIUM_STK_SIZE,
        6,
        NULL
    );
    if (ret != SAL_RET_SUCCESS) {
        mcu_printf("Task creation failed with error code: %d\n", ret);
        return ret;
    }

    ret = SAL_TaskCreate
    (
        &receiveFromFrontBoardTaskID,
        (const uint8 *)"receive From Front Board Task",
        (SALTaskFunc)&receiveFromFrontBoardTask,
        &receiveFromFrontBoardTaskStk[0],
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
        &receiveFromRearBoardTaskID,
        (const uint8 *)"Receive From Rear Board Task",
        (SALTaskFunc)&receiveFromRearBoardTask,
        &receiveFromRearBoardTaskStk[0],
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
        &ProjectIPCTaskID,
        (const uint8 *)"Send To Main Core Task",
        (SALTaskFunc)&sendToMainCoreTask,
        &ProjectIPCTaskStk[0],
        TASK_MEDIUM_STK_SIZE,
        6,
        NULL
    );
    if (ret != SAL_RET_SUCCESS) {
        mcu_printf("Task creation failed with error code: %d\n", ret);
        return ret;
    }

    return ret;
}

static SALRetCode_t receiveFromPedalTask(void *pArg)
{
    (void)pArg;

    // int pressBreak = 0;        
    //uint32 startTick, endTick; // time test          

    uint8 ucCh = 1;
    SALRetCode_t ret;
    uint8 control[1] = {CONTROL_DATA};
    uint8 input_data[2] = {0};
    uint8 isAutohold = 0;

    ret = SAL_RET_SUCCESS;

    // I2C set
    ret = I2C_Open(ucCh, I2C_PCF8591_SCL, I2C_PCF8591_SDA, I2C_CLK_RATE_100, NULL , NULL);

    if(ret != SAL_RET_SUCCESS){
        mcu_printf("[DEBUG] I2C OPEN FAIL!!!!!!!!!\n");
        return ret;
    }

    while(1){
        ret = PCF8591Read(&input_data[0], (uint32)2, control[0]);
        if(ret != SAL_RET_SUCCESS){
            mcu_printf("[DEBUG] PCF8591 READ FAIL!!!!!!!!!\n");
            return ret;
        }
        
      // accle_data 유효값으로 조정
        gPressAccel = (int)input_data[0] - ACCEL_OFFSET;
      if(gPressAccel < 2){
            gPressAccel = INTPUT_MIN;
        }
        else if(gPressAccel > INPUT_MAX){
            gPressAccel = INPUT_MAX;
        }

      // break_press 유효값으로 조정
        gPressBreak = (int)input_data[1] - BREAK_OFFSET;
      if(gPressBreak < 2){
            gPressBreak = INTPUT_MIN;
        }
        else if(gPressBreak > INPUT_MAX){
            gPressBreak = INPUT_MAX;
        }

      // 엑셀 입력 퍼센트 계산
        gAccelPercent = ((float)(gPressAccel) / (float)INPUT_MAX) * 100;
      // 전/후방으로 보낼 pwm 값 계산
        gPwmDuty = IDLE_PWM;
        if(gPressBreak == 0){
            gPwmDuty += gPressAccel;
        }
        gPwmDuty -= gPressBreak;
        if(gPwmDuty < INTPUT_MIN){
            gPwmDuty = 0;
        }
        
        if(gAutoholdMode){
            if(gPwmDuty == 0){
                isAutohold = 1;
            }
        }
        else{
            isAutohold = 0;
        }

        if(isAutohold){
            if((gPressAccel != 0)){
                isAutohold = 0;
                gPwmDuty += gPressAccel;
            }
            else{
                gPwmDuty = 0;
            }
        }

        // mcu_printf("gAccelPercent: %d, gPressBreak: %d, gPwmDuty: %d\n", gAccelPercent, gPressBreak, gPwmDuty);
        // mcu_printf("gAccelPercent: %d\n", gAccelPercent);

        ret = SAL_TaskSleep(10); // sleep 0.1s
        if(ret != SAL_RET_SUCCESS){
            mcu_printf("[DEBUG] Pedal Task Sleep Fail!!!!!!!!!\n");
            return ret;
        }
    }

    return ret;
}

static SALRetCode_t sendToBoardTask(void *pArg)
{
    uint8   ch_front = UART_CH_FOR_FRONT; //전방 채널
    uint8   ch_rear = UART_CH_FOR_REAR;  // 후방 채널
    uint8   send_to_front_data[FRONT_TX_BUFFER_SIZE];
    uint8   send_to_rear_data[REAR_TX_BUFFER_SIZE];
    SALRetCode_t ret;
    ret = SAL_RET_SUCCESS;

    (void)SAL_MemSet(send_to_front_data, 0, FRONT_TX_BUFFER_SIZE);
    (void)SAL_MemSet(send_to_rear_data, 0, REAR_TX_BUFFER_SIZE);

    while(1)
    {
        send_to_front_data[0] = send_to_rear_data[0] = is_abnormal;
        send_to_front_data[1] = send_to_rear_data[1] = (uint8)gPwmDuty;
        send_to_front_data[2] = send_to_rear_data[2] = gDriveMode;
        send_to_front_data[3] = send_to_rear_data[3] = gAutoholdMode;

        (void)uart_send_data(ch_front, send_to_front_data, FRONT_TX_BUFFER_SIZE); //전방에 전달
        (void)uart_send_data(ch_rear, send_to_rear_data, REAR_TX_BUFFER_SIZE);  //후방에 전달, 버퍼 사이즈 전방과 동일하게..
        mcu_printf("[send] is_abnormal: %d, pwm: %d, drive mode: %d, autohold: %d!!\n",send_to_front_data[0], send_to_front_data[1], send_to_front_data[2], send_to_front_data[3]);
   
        SAL_TaskSleep(500);
    }

    return ret;
}

static SALRetCode_t receiveFromFrontBoardTask(void *pArg)
{
    uint8   receive_data[FRONT_RX_BUFFER_SIZE];
    SALRetCode_t ret = SAL_RET_SUCCESS;
    uint32 flag = 0;

    (void)pArg;

    // ret = SAL_EventCreate((uint32 *)&gRecvFromMidTaskEventId, 
    //                       (const uint8*)"receive signal event created", 
    //                       0);
    // if(ret != SAL_RET_SUCCESS){
    //     mcu_printf("error for SAL_EventCreate!\n");
    //     return ret;
    // }

    (void)SAL_MemSet(receive_data, 0, FRONT_RX_BUFFER_SIZE);

    while(1)
    {
        (void) SAL_EventGet(gRecvFromMidTaskEventId, 0x00000001UL, 0UL, (uint32)SAL_EVENT_OPT_CONSUME, &flag);

        if ((flag & 0x00000001UL) != 0) // 비트가 설정되었는지 확인
        {
            uart_receive_data(UART_CH_FOR_FRONT, receive_data, FRONT_RX_BUFFER_SIZE);

            if(receive_data[0] == ID_ULTRA_SONIC){
                mcu_printf("[front] ID_ULTRA_SONIC");
                gDistance = receive_data[1];
            }
            else if(receive_data[0] == ID_RED){
                mcu_printf("[front] ID_RED");
                gIsRedLight = receive_data[1];
            }
            else{
                mcu_printf("invaild ID\n");
            }

        }

        mcu_printf("[front] Distance: %d, Red Light Status: %d\n", gDistance, gIsRedLight);
    }

    return ret;
}

static SALRetCode_t receiveFromRearBoardTask(void *pArg)
{
        uint8   receive_data[FRONT_RX_BUFFER_SIZE];
    SALRetCode_t ret = SAL_RET_SUCCESS;
    uint32 flag = 0;

    (void)pArg;

    // ret = SAL_EventCreate((uint32 *)&gRecvFromMidTaskEventId, 
    //                       (const uint8*)"receive signal event created", 
    //                       0);
    // if(ret != SAL_RET_SUCCESS){
    //     mcu_printf("error for SAL_EventCreate!\n");
    //     return ret;
    // }

    (void)SAL_MemSet(receive_data, 0, FRONT_RX_BUFFER_SIZE);

    while(1)
    {
        (void) SAL_EventGet(gRecvFromMidTaskEventId, 0x00000002UL, 0UL, (uint32)SAL_EVENT_OPT_CONSUME, &flag);

        if ((flag & 0x00000002UL) != 0) // 비트가 설정되었는지 확인
        {
            uart_receive_data(UART_CH_FOR_REAR, receive_data, FRONT_RX_BUFFER_SIZE);

            if(receive_data[0] == ID_ULTRA_SONIC){
                gDistance = receive_data[1];
            }
            else{
                mcu_printf("invaild ID\n");
            }

        }

        mcu_printf("[rear] Distance: %d\n", gDistance);
    }

    return ret;
}

static SALRetCode_t sendToMainCoreTask(void *pArg)
{  
    (void)pArg;
    uint32 txIpcData = 0;
    SALRetCode_t ret;
    ret = SAL_RET_SUCCESS;


    while(1){
        txIpcData = (gAccelPercent << 16) + (gDriveMode << 8) + (gAutoholdMode);
        ipc_send_data(txIpcData);
        
        // (void)SAL_GetTickCount(&endTick);
        // mcu_printf("[%d] start: %d, end: %d\n", startTick - endTick, startTick, endTick);

        ret = SAL_TaskSleep(100); // sleep 0.1s
        if(ret != SAL_RET_SUCCESS){
            mcu_printf("[DEBUG] Send to Main Core Task Sleep Fail!!!!!!!!!\n");
            return ret;
        }
    }

    return ret;
}

static SALRetCode_t checkAbnormalActionTask(void *pArg)
{
    (void)pArg;

    SALRetCode_t ret;
    // uint32 startTick, endTick; // time test  

    // -------------------------------
    uint8   send_data[FRONT_TX_BUFFER_SIZE];
    (void)SAL_MemSet(send_data, 0, FRONT_TX_BUFFER_SIZE);
    // --------------------------------

    ret = SAL_RET_SUCCESS;

    init_queue();

    while(1){
        // (void)SAL_GetTickCount(&startTick);

        // not P
        if(gDriveMode != 0){
            // abnormal
            if(is_abnormal){
                add_queue(0);

                if(check_finish_abnormal(gPressBreak)){
                    ipc_send_off_sound();
                    is_abnormal = ID_DATA;
                }
            }
            // noraml
            else{
                add_queue(gPwmDuty);

                if(check_start_abnormal(
                    gPressAccel, 
                    gPwmDuty, 
                    gIsRedLight, 
                    gAutoholdMode, 
                    gDriveMode,
                    gDistance)
                )
                {
                    send_data[0] = ID_ABNORMAL;
                    uart_send_data(UART_CH_FOR_FRONT, send_data, FRONT_TX_BUFFER_SIZE);
                    uart_send_data(UART_CH_FOR_REAR, send_data, FRONT_RX_BUFFER_SIZE);

                    ipc_send_on_sound();
                    
                    is_abnormal = ID_ABNORMAL;
                    gPwmDuty = 0;
                }
                
            }
        }
        else{
            add_queue(0);
        }

        // 50ms 간격으로 확인
        ret = SAL_TaskSleep(ABNORMAL_DELAY);
        if(ret != SAL_RET_SUCCESS){
            mcu_printf("[DEBUG] check Abnormal Action Task Fail!!!!!!!!!\n");
            return ret;
        }
        // (void)SAL_GetTickCount(&endTick);
        // mcu_printf("[checkAb] starttick: %d, endtick: %d, diff: %d\n", startTick, endTick, endTick - startTick);
    }

    return ret;
}

static SALRetCode_t updateDriveModeTask(void *pArg)
{
    (void)pArg;
    SALRetCode_t ret;
    ret = SAL_RET_SUCCESS;

    mcu_printf("Start checking drive mode\n");

    // GPIO 핀 초기화
    ret = GPIO_Config(D_MODE_PIN, (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN));
    if (ret != SAL_RET_SUCCESS)
    {
        mcu_printf("Drive mode Configuration failed.\n");
        return ret;
    }
    
    ret = GPIO_Config(R_MODE_PIN, (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN));
    if (ret != SAL_RET_SUCCESS)
    {
        mcu_printf("Drive mode Configuration failed.\n");
        return ret;
    }
    
    mcu_printf("Drive mode Configuration succeeded.\n");

    //0: P, 
    //1: D, 
    //2: R

    // drive mode 상태 확인 polling
    while (1) 
    {
        // 현재 버튼 상태 읽기
        uint32 D_state = GPIO_Get(D_MODE_PIN);
        uint32 R_state = GPIO_Get(R_MODE_PIN);

        // 버튼이 이전에 눌려있지 않았고, 현재 눌려있다면 상승 에지 감지
        if (D_state) 
        {
            gDriveMode = 1;  // 1로 세팅
        }
        else if (R_state)
        {
            gDriveMode = 2;
        }
        else
        {
            gDriveMode = 0;            
        }
        // mcu_printf("gDriveMode: %d (0: P mode, 1: D mode, 2: R mode)\n", gDriveMode);

        // 10ms 간격으로 상태 확인
        ret = SAL_TaskSleep(500);
        if(ret != SAL_RET_SUCCESS){
            mcu_printf("[DEBUG] Update Drive Mode Task Sleep Fail!!!!!!!!!\n");
            return ret;
        }
    }

    return ret;
}

static SALRetCode_t updateAutoholdTask(void *pArg)
{
    (void)pArg;
    SALRetCode_t ret;
    ret = SAL_RET_SUCCESS;

    ret = GPIO_Config(AUTOHOLD_PIN, (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN));

    // Autohold GPIO 핀 초기화
    if (ret != SAL_RET_SUCCESS)
    {
        mcu_printf("Autohold GPIO configuration failed.\n");
        return ret;
    }
    mcu_printf("Autohold GPIO configuration succeeded.\n");


    // 버튼 상태 추적 변수
    uint32 previous_pin_state = 0;  // 이전 상태를 저장 (0 = not pressed, 1 = pressed)

    while (1) 
    {
        // 현재 버튼 상태 읽기
        uint32 current_pin_state = GPIO_Get(AUTOHOLD_PIN);
        //mcu_printf("autohold is now: %d\n", current_pin_state);

        // 버튼이 이전에 눌려있지 않았고, 현재 눌려있다면 상승 에지 감지
        if (current_pin_state == 1 && previous_pin_state == 0) 
        {
            gAutoholdMode ^= (1);  // autohold 값 반전
        }

        // 버튼 상태 업데이트
        previous_pin_state = current_pin_state;
        // mcu_printf("Autoholde: %d\n", gAutoholdMode);

        ret = SAL_TaskSleep(500); // sleep 0.1s
        if(ret != SAL_RET_SUCCESS){
            mcu_printf("[DEBUG] Update Autohold Task Sleep Fail!!!!!!!!!\n");
            return ret;
        }
    }
}
