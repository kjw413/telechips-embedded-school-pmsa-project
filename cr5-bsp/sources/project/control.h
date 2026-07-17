#ifndef CONTROL_H
#define CONTROL_H

#include "pdm.h"
#include "gpio.h"
#include "debug.h"
#include "sal_internal.h"
#include "bsp.h"
#include <sal_api.h>
#include <main.h>
#include <stdint.h>
#include "front_board.h"

static void Motor_SetGpio(void);
void GPIO_Set_Switch(void);
void Motor_Init(PDMModeConfig_t *ModeConfigInfo);
void Motor_UpdateDuty(PDMModeConfig_t *ModeConfigInfo, uint32 speed);
void Motor_Option(uint8 gDriveMode, uint32 *speed);
void Motor_ConfigurePDM(PDMModeConfig_t *ModeConfigInfo);

#define PDM_CH_0                        (0UL)
#define Drive_State                     GPIO_GPG(1UL)
#define Rear_State                      GPIO_GPG(2UL)
#define ENA_PWM                         GPIO_GPC(29UL)
#define IN_1_3                          GPIO_GPC(5UL)
#define IN_2_4                          GPIO_GPC(4UL)

#endif // CONTROL_H

