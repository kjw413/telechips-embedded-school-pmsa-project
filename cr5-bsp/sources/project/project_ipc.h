#ifndef PROJECT_IPC_H
#define PROJECT_IPC_H

#include <sal_com.h>

int32 ipc_send_on_sound(void);
int32 ipc_send_data(uint32 data);

int32 ipc_receive_camera(void);
void CreateProjectIPCTask(void);

// temporary variable
extern uint32 send_data;

#endif