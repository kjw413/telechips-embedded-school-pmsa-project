#include "external_io.h"

sint32 init_uart(uint8 ch, uint8 portSel)
{  
    return UART_Open(ch, 10UL, 115200UL, UART_INTR_MODE, UART_CTSRTS_OFF, portSel,
                    WORD_LEN_8, ENABLE_FIFO, TWO_STOP_BIT_OFF, PARITY_SPACE, (GICIsrFunc)&UART_ISR);

}

sint32 init_uart2(uint8 ch, uint8 portSel)
{  
    return UART_Open(ch, 10UL, 115200UL, UART_INTR_MODE, UART_CTSRTS_OFF, portSel,
                    WORD_LEN_8, ENABLE_FIFO, TWO_STOP_BIT_OFF, PARITY_SPACE, (GICIsrFunc)&UART_ISR2);

}

sint32 uart_send_data(uint8 ch, uint8 *data, uint8 size)
{
    return UART_Write(ch, data, size);
}

sint32 uart_receive_data(uint8 ch, uint8 *data, uint8 size)
{
    return UART_Read(ch, data, size);
}
