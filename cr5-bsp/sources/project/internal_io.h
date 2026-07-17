#ifndef INTERNAL_IO_H
#define INTERNAL_IO_H

#include "ipc/ipc.h"

int32 ipc_init(void);

int32 ipc_create_receive_task(void);

int32 ipc_send_on_sound(void);
int32 ipc_send_off_sound(void);

int32 ipc_send_data(uint32 data);

#endif // INTERNAL_IO_H