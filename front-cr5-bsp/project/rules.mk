MCU_BSP_PROJECT_PATH := $(MCU_BSP_BUILD_CURDIR)

# Paths
VPATH += $(MCU_BSP_PROJECT_PATH)

# Includes
INCLUDES += -I$(MCU_BSP_PROJECT_PATH)
INCLUDES += -I/home/junghyun/topst/cr5-bsp/sources/app.drivers/
INCLUDES += -I/home/junghyun/topst/cr5-bsp/sources/dev.drivers/

# Sources
SRCS += project_ipc.c
SRCS += project_i2c.c