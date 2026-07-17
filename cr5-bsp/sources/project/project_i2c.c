#include "project_i2c.h"
#include "project_ipc.h"

#define I2C_PCF8591_ADDR                ((uint8)0x48)
#define CONTROL_DATA                    ((uint8)0x44)

#define I2C_TASK_STK_SIZE               (512UL)

#define I2C_TEST_CLK_RATE_100           (100)

// accel_data를 0~127로 맞추기 위한 offset
#define ACCEL_OFFSET    (127)
// break_data를 0~127로 맞추기 위한 offset
#define BREAK_OFFSET    (121)
// 최대 입력 값: 127
#define PRESS_MAX       (127)
// 최소 입력 값: 0
#define PRESS_MIN       (0)

// 전/후방으로 보낼 pwm data
uint8 pwm_data = 0;
// 7-segment로 보낼 엑셀 입력 퍼센트
uint16 accel_press = 0;
// 모드에 따른 초기 pwm 값(임시값)
uint8 pwm_mode = 127;

SALRetCode_t I2cRead(uint8 * puiBuff, uint32 uiSize, uint8 uiRegaddr);
SALRetCode_t I2cRead(uint8 * puiBuff, uint32 uiSize, uint8 uiRegaddr)
{
    uint8  module_ctrl_size  = 1;
    uint8  module_ctrl_buff[1] = {0x00};
    uint8  i2c_rx_size  = 0;
    uint8  i2c_asyn  = 0;
    uint8 *i2c_rx_buff  = NULL;
    uint32 i2c_optn  = 0;
    SALRetCode_t ret;

    i2c_rx_buff = puiBuff;
    i2c_rx_size = (uint8)uiSize;
    module_ctrl_buff[0] = uiRegaddr;
    ret = (SALRetCode_t)I2C_Xfer((uint8)1UL, (I2C_PCF8591_ADDR << 1),
                                            module_ctrl_size, module_ctrl_buff,
                                            i2c_rx_size, i2c_rx_buff,
                                            i2c_optn, i2c_asyn);

    return ret;
}


static void I2CTask(void *pArg)
{
    uint8 ucCh = 1;
    SALRetCode_t ret;
    uint8 control[1] = {CONTROL_DATA};
    uint8 input_data[2] = {0};
    int accel_data = 0;
    int break_data = 0;
    
    ret = SAL_RET_SUCCESS;

    // I2C set
    ret = I2C_Open(ucCh, I2C_PCF8591_SCL, I2C_PCF8591_SDA, I2C_TEST_CLK_RATE_100, NULL , NULL);

    if(ret != SAL_RET_SUCCESS){
        mcu_printf("I2C OPEN FAIL!!!!!!!!!\n");
    }

    while(1){
        ret = I2cRead(&input_data[0], (uint32)2, control[0]);
        if(ret != SAL_RET_SUCCESS){
        mcu_printf("I2C READ FAIL!!!!!!!!!\n");
        }
        
		// accle_data 유효값으로 조정
        accel_data = (int)input_data[0] - ACCEL_OFFSET;
		if(accel_data < PRESS_MIN){
            accel_data = PRESS_MIN;
        }
        else if(accel_data > PRESS_MAX){
            accel_data = PRESS_MAX;
        }

		// break_data 유효값으로 조정
        break_data = (int)input_data[1] - BREAK_OFFSET;
		if(break_data < PRESS_MIN){
            break_data = PRESS_MIN;
        }
        else if(break_data > PRESS_MAX){
            break_data = PRESS_MAX;
        }

		// 엑셀 입력 퍼센트 계산
        accel_press = ((float)(accel_data) / (float)PRESS_MAX) * 100;
        send_data = ((uint32)(accel_press << 16) + (send_data & 0x0000ffff));
        //ipc_send_data(test_data);
		// 전/후방으로 보낼 pwm 값 계산
        if((accel_data > 0) && (break_data == 0)){
            pwm_data = pwm_mode + accel_data;
        }
        else{
            pwm_data = pwm_mode - break_data;
        }

        mcu_printf("Accel: %d  Break: %d\n", accel_data, break_data);
        mcu_printf("test_data: %d, Accel press: %d  pwm: %d\n", (send_data >> 16), accel_press, pwm_data);

        (void)SAL_TaskSleep(100); // sleep 0.1s
    }
}


void CreateI2CTask(void)
{
    static uint32   I2CTaskID = 0UL;
    static uint32   I2CTaskStk[I2C_TASK_STK_SIZE];

    (void)SAL_TaskCreate
    (
        &I2CTaskID,
        (const uint8 *)"I2C Task",
        (SALTaskFunc)&I2CTask,
        &I2CTaskStk[0],
        I2C_TASK_STK_SIZE,
        6,
        NULL
    );
}
