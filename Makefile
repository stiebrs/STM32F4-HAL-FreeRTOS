######################################
# target
######################################
TARGET = HAL-freeRTOS-test1


######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og


#######################################
# paths
#######################################
# STM Cube library path
ifeq ($(OS),Windows_NT)
GCC_PATH = C:\tools\arm-gcc\7-2017-q4-major\bin
CUBE_PATH = C:\tools\STM32Cube\Repository\STM32Cube_FW_F4_V1.25.2
else
GCC_PATH = ~/tools/gcc-arm-none-eabi-8-2019-q3-update/bin
CUBE_PATH = /home/kaspars/STM32Cube/Repository/STM32Cube_FW_F4_V1.25.2
endif

# Build path
BUILD_DIR = build
DRIVERS_PATH = $(CUBE_PATH)/Drivers
HAL_PATH = $(DRIVERS_PATH)/STM32F4xx_HAL_Driver
MIDDLEWARES_PATH = $(CUBE_PATH)/Middlewares

# firmware library path
PERIFLIB_PATH = 

# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
C_SOURCES =  \
$(HAL_PATH)/Src/stm32f4xx_hal_gpio.c \
$(HAL_PATH)/Src/stm32f4xx_hal_crc.c \
$(HAL_PATH)/Src/stm32f4xx_hal_pwr.c \
$(HAL_PATH)/Src/stm32f4xx_hal_flash_ramfunc.c \
$(HAL_PATH)/Src/stm32f4xx_hal_dma2d.c \
$(HAL_PATH)/Src/stm32f4xx_hal_flash_ex.c \
$(HAL_PATH)/Src/stm32f4xx_hal_rcc.c \
$(HAL_PATH)/Src/stm32f4xx_hal.c \
$(HAL_PATH)/Src/stm32f4xx_hal_tim.c \
$(HAL_PATH)/Src/stm32f4xx_hal_pwr_ex.c \
$(HAL_PATH)/Src/stm32f4xx_hal_uart.c \
$(HAL_PATH)/Src/stm32f4xx_hal_dma.c \
$(HAL_PATH)/Src/stm32f4xx_hal_dma_ex.c \
$(HAL_PATH)/Src/stm32f4xx_hal_rcc_ex.c \
$(HAL_PATH)/Src/stm32f4xx_hal_spi.c \
$(HAL_PATH)/Src/stm32f4xx_hal_flash.c \
$(HAL_PATH)/Src/stm32f4xx_hal_tim_ex.c \
$(HAL_PATH)/Src/stm32f4xx_hal_cortex.c \
$(MIDDLEWARES_PATH)/Third_Party/FreeRTOS/Source/croutine.c \
$(MIDDLEWARES_PATH)/Third_Party/FreeRTOS/Source/queue.c \
$(MIDDLEWARES_PATH)/Third_Party/FreeRTOS/Source/event_groups.c \
$(MIDDLEWARES_PATH)/Third_Party/FreeRTOS/Source/timers.c \
$(MIDDLEWARES_PATH)/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c \
$(MIDDLEWARES_PATH)/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c \
$(MIDDLEWARES_PATH)/Third_Party/FreeRTOS/Source/tasks.c \
$(MIDDLEWARES_PATH)/Third_Party/FreeRTOS/Source/list.c \
$(MIDDLEWARES_PATH)/Third_Party/FreeRTOS/Source/CMSIS_RTOS/cmsis_os.c \
Src/stm32f4xx_hal_timebase_TIM.c \
Src/gpio.c \
Src/stm32f4xx_hal_msp.c \
Src/spi.c \
Src/usb_otg.c \
/Src/system_stm32f4xx.c \
Src/usart.c \
Src/printf_retarget.c \
Src/tim.c \
Src/stm32f4xx_it.c \
Src/crc.c \
Src/freertos.c \
Src/main.c 

# ASM sources
ASM_SOURCES =  \
startup_stm32f429xx.s


######################################
# firmware library
######################################
PERIFLIB_SOURCES = 


#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifeq ($(OS),Windows_NT)
RM := rmdir /Q /S
CPY := copy
else
RM := rm -rf
CPY := cp
endif

ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4

# fpu
FPU = -mfpu=fpv4-sp-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32F429xx \
-DUSE_FULL_ASSERT


# AS includes
AS_INCLUDES =  \
-I/Inc

# C includes
C_INCLUDES =  \
-IInc \
-I$(HAL_PATH)/Inc \
-I$(HAL_PATH)/Inc/Legacy \
-I$(MIDDLEWARES_PATH)/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F \
-I$(DRIVERS_PATH)/CMSIS/Device/ST/STM32F4xx/Include \
-I$(MIDDLEWARES_PATH)/Third_Party/FreeRTOS/Source/include \
-I$(MIDDLEWARES_PATH)/Third_Party/FreeRTOS/Source/CMSIS_RTOS \
-I$(DRIVERS_PATH)/CMSIS/Include


# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = STM32F429ZITx_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	
$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	$(RM) .dep $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

# *** EOF ***