#ifndef SENSOR_H
#define SENSOR_H

#include <i2c/tcc805x/i2c_reg.h>

SALRetCode_t PCF8591Read(uint8 * puiBuff, uint32 uiSize, uint8 uiRegaddr);

#endif // SENSOR_H