#include "project_ipc.h"
#include "ipc/ipc.h"
#include "ipc/common/ipc_typedef.h"

int32 ipc_send_on_sound(void)
{   
    return IPC_SendToMainCtlCmd(SOUND_ON);
}   

int32 ipc_send_data(uint32 data)
{
    return IPC_SendToMainData(data);
}

int32 ipc_receive_camera(void);