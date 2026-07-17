#include "sensor_io.h"

#define I2C_PCF8591_ADDR                ((uint8)0x48)

SALRetCode_t PCF8591Read(uint8 * puiBuff, uint32 uiSize, uint8 uiRegaddr)
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
    ret = I2C_Xfer((uint8)1UL, (I2C_PCF8591_ADDR << 1),
                                            module_ctrl_size, module_ctrl_buff,
                                            i2c_rx_size, i2c_rx_buff,
                                            i2c_optn, i2c_asyn);

    return ret;
}
