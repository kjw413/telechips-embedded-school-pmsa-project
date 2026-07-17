MCU_BSP_PROJECT_PATH := $(MCU_BSP_BUILD_CURDIR)

# Paths
VPATH += $(MCU_BSP_PROJECT_PATH)

# Includes
INCLUDES += -I$(MCU_BSP_PROJECT_PATH)
INCLUDES += -I$(MCU_BSP_PROJECT_PATH)../app.drivers/
INCLUDES += -I$(MCU_BSP_PROJECT_PATH)../dev.drivers/

# ----------------------- 보드에 맞게 설정 ------------------------
# 보드(전방(CONFIG_FRONT) | 중앙(CONFIG_MID) | 후방(CONFIG_REAR)) = 1
CONFIG_FRONT = 1
# -----------------------------------------------------------------

# Sources
ifdef CONFIG_FRONT
SRCS += front_board.c \
				control.c \
				external_io.c \
				internal_io.c \
				ultra_sonic.c
endif

ifdef CONFIG_MID
SRCS += mid_board.c \
				abnormal.c \
				external_io.c \
				internal_io.c \
				sensor_io.c
endif

ifdef CONFIG_REAR
SRCS += rear_board.c \
        control.c \
        external_io.c \
        ultra_sonic.c
endif