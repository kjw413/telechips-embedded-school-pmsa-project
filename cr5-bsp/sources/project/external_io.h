#ifndef EXTERNAL_IO_H
#define EXTERNAL_IO_H

#include "uart/uart.h"

sint32 init_uart(uint8 ch, uint8 portSel);
sint32 init_uart2(uint8 ch, uint8 portSel);
sint32 uart_send_data(uint8 ch, uint8 *data, uint8 size);
sint32 uart_receive_data(uint8 ch, uint8 *data, uint8 size);


#endif // EXTERNAL_IO_H