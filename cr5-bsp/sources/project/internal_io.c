#include "internal_io.h"
#include "ipc/common/ipc_typedef.h"
#include "defs.h"

#if defined(CONFIG_FRONT) && CONFIG_FRONT == 1
int32 ipc_init(void)
{
    return IPC_init(); 
}

int32 ipc_create_receive_task(void)
{
    return IPC_CreateTask();
}
#endif

int32 ipc_send_on_sound(void)
{   
    return IPC_SendToMainCtlCmd(SOUND_ON);
}   

int32 ipc_send_off_sound(void)
{
    return IPC_SendToMainCtlCmd(SOUND_OFF);
}

int32 ipc_send_data(uint32 data)
{
    return IPC_SendToMainData(data);
}