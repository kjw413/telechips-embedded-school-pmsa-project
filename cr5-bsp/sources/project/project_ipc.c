#include "project_ipc.h"
#include "ipc/ipc.h"
#include "ipc/common/ipc_typedef.h"
#include <debug.h>

#define ProjectIPC_TASK_STK_SIZE    (512UL)

uint32 send_data = 0;


int32 ipc_send_on_sound(void)
{   
    return IPC_SendToMainCtlCmd(SOUND_ON);
}   

int32 ipc_send_data(uint32 data)
{
    return IPC_SendToMainData(data);
}

int32 ipc_receive_camera(void);

static void ProjectIPCTask(void *pArg)
{
    while(1){
        mcu_printf("IPCIPCICPICICIPCICP!!!!!!!!\n");
        ipc_send_data(send_data);
        (void)SAL_TaskSleep(100); // sleep 0.1s
    } 

}


void CreateProjectIPCTask(void)
{
    static uint32   ProjectIPCTaskID;
    static uint32   ProjectIPCTaskStk[ProjectIPC_TASK_STK_SIZE];

    SALRetCode_t ret = SAL_TaskCreate
    (
        &ProjectIPCTaskID,
        (const uint8 *)"ProjectIPC Task",
        (SALTaskFunc)&ProjectIPCTask,
        &ProjectIPCTaskStk[0],
        ProjectIPC_TASK_STK_SIZE,
        6,
        NULL
    );

    if (ret != SAL_RET_SUCCESS) {
        mcu_printf("Task creation failed with error code: %d\n", ret);
    }      
}
