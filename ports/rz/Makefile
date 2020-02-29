include ../../py/mkenv.mk

CROSS ?= 0
BOARD ?= GR_MANGO

ifeq ($(wildcard boards/$(BOARD)/.),)
$(error Invalid BOARD specified)
endif

# If the build directory is not given, make it reflect the board name.
BUILD ?= build-$(BOARD)

BOARD_DIR ?= boards/$(BOARD)
ifeq ($(wildcard $(BOARD_DIR)/.),)
$(error Invalid BOARD specified: $(BOARD_DIR))
endif

ifeq ($(BOARD),GR_MANGO)
CROSS = 1
TARGET = TARGET_RZ_A2XX
TARGET_LD = TARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/TOOLCHAIN_GCC_ARM/RZA2M.ld
STACK_USAGE = 16384
else
ifeq ($(CROSS), 1)
TARGET = stm32f405
TARGET_LD = $(TARGET).ld
endif
endif

# qstr definitions (must come before including py.mk)
QSTR_DEFS = qstrdefsport.h

# include py core make definitions
include $(TOP)/py/py.mk

ifeq ($(CROSS), 1)
CROSS_COMPILE ?= arm-none-eabi-
endif

INC += -I.
INC += -I$(TOP)
INC += -I$(BUILD)

ifeq ($(TARGET), TARGET_RZ_A2XX)
INC += -ITARGET_RENESAS
INC += -ITARGET_RENESAS/$(TARGET)
INC += -ITARGET_RENESAS/$(TARGET)/common
INC += -ITARGET_RENESAS/$(TARGET)/common/r_cache
INC += -ITARGET_RENESAS/$(TARGET)/common/r_cache/inc
INC += -ITARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)
INC += -ITARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device
INC += -ITARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/inc
INC += -ITARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/inc/iodefine
INC += -ITARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/inc/iodefine/iobitmasks
INC += -ITARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/inc/iodefine/iodefines
INC += -I$(TOP)/lib/mbed-os
INC += -I$(TOP)/lib/mbed-os/drivers
INC += -I$(TOP)/lib/mbed-os/events
INC += -I$(TOP)/lib/mbed-os/hal
INC += -I$(TOP)/lib/mbed-os/cmsis
INC += -I$(TOP)/lib/mbed-os/cmsis/TARGET_CORTEX_A
INC += -I$(TOP)/lib/mbed-os/platform
INC += -I$(TOP)/lib/mbed-os/rtos
INC += -I$(TOP)/lib/mbed-os/rtos/TARGET_CORTEX
INC += -I$(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/rtx4/
INC += -I$(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/rtx5/Include
INC += -I$(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Config
INC += -I$(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Include
INC += -I$(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source
INC += -I$(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/TOOLCHAIN_GCC_ARM
INC += -I$(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Include
INC += -I$(TOP)/lib/mbed-os/usb/device/hal
INC += -I$(TOP)/lib/mbed-os/usb/device/targets/TARGET_RENESAS/$(TARGET)
INC += -I$(TOP)/lib/mbed-os/usb/device/USBAudio
INC += -I$(TOP)/lib/mbed-os/usb/device/USBDevice
INC += -I$(TOP)/lib/mbed-os/usb/device/USBHID
INC += -I$(TOP)/lib/mbed-os/usb/device/USBMIDI
INC += -I$(TOP)/lib/mbed-os/usb/device/USBMSD
INC += -I$(TOP)/lib/mbed-os/usb/device/USBPhy
INC += -I$(TOP)/lib/mbed-os/usb/device/USBSerial
INC += -I$(TOP)/lib/mbed-os/usb/device/utilities
INC += -I$(TOP)/lib/mbed-os/usb/device/utilities/events
endif

ifeq ($(CROSS), 1)
ifeq ($(TARGET), TARGET_RZ_A2XX)
LD = gcc
#-Wstack-usage=$(STACK_USAGE)
CFLAGS_COMMON += $(INC) -DAPPLICATION_ADDR=0x50000000 -DAPPLICATION_SIZE=0x1000000 -DMBED_ROM_START=0x50000000 -DMBED_ROM_SIZE=0x1000000 -DMBED_RAM_START=0x400000 -DMBED_RAM_SIZE=0x2000000 -DARM_MATH_CA9 -D__FPU_PRESENT -DDEVICE_EMAC=1 -D__MBED__=1 -DDEVICE_USBDEVICE=1 -DTARGET_LIKE_MBED -DDEVICE_PORTINOUT=1 -DMBED_BUILD_TIMESTAMP=1582947353.33 -DCOMPONENT_FLASHIAP=1 -DMBEDTLS_NO_DEFAULT_ENTROPY_SOURCES -DCOMPONENT_PSA_SRV_EMUL=1 -DDEVICE_SERIAL_ASYNCH=1 -D__CMSIS_RTOS -DMBEDTLS_USER_CONFIG_FILE="mbedtls_entropy_config.h" -D__EVAL -DDEVICE_SPISLAVE=1 -DTOOLCHAIN_GCC -DTARGET_CORTEX_A -DDEVICE_I2C_ASYNCH=1 -DTARGET_DEBUG -DDEVICE_RTC=1 -D__MBED_CMSIS_RTOS_CA9 -DCOMPONENT_PSA_SRV_IMPL=1 -DTARGET_LIKE_CORTEX_A9 -DDEVICE_PWMOUT=1 -DDEVICE_SPI_ASYNCH=1 -DDEVICE_INTERRUPTIN=1 -DTARGET_CORTEX -DDEVICE_I2C=1 -DDEVICE_PORTOUT=1 -DDEVICE_I2CSLAVE=1 -DDEVICE_USTICKER=1 -DDEVICE_STDIO_MESSAGES=1 -DTARGET_MBRZA2M -DTARGET_RENESAS -DTARGET_RZA2M -DTARGET_NAME=GR_MANGO -DCOMPONENT_NSPE=1 -DOVERRIDE_CONSOLE_USBSERIAL -DDEVICE_SERIAL_FC=1 -DMBEDTLS_TEST_NULL_ENTROPY -DTARGET_A9 -DTARGET_RZ_A2XX -D__CORTEX_A9 -DDEVICE_PORTIN=1 -DDEVICE_SLEEP=1 -DTOOLCHAIN_GCC_ARM -DDEVICE_SPI=1 -DTARGET_GR_MANGO -DDEVICE_ANALOGIN=1 -DDEVICE_SERIAL=1 -DDEVICE_FLASH=1 -DMBED_CONF_APP_MAIN_STACK_SIZE=8192 -DTARGET_RZ_A2_EMAC
CFLAGS_COMMON += -DMBED_CONF_PLATFORM_CTHUNK_COUNT_MAX=1
CFLAGS_COMMON += -DMBED_CONF_PLATFORM_DEFAULT_SERIAL_BAUD_RATE=115200
CFLAGS_COMMON += -DMBED_CONF_PLATFORM_STDIO_BAUD_RATE=115200
CFLAGS_COMMON += -DMBED_CONF_RTOS_THREAD_STACK_SIZE=4096
#CFLAGS_COMMON += -MMD
#CFLAGS_COMMON += -MF $(@:%.o=%.d)
CFLAGS = -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -fmessage-length=0 -fno-exceptions -fno-builtin -ffunction-sections -fdata-sections -funsigned-char -fno-delete-null-pointer-checks -fomit-frame-pointer -O0 -g3 -DMBED_DEBUG -DMBED_TRAP_ERRORS_ENABLED=1 -mcpu=cortex-a9 -mthumb-interwork -marm -march=armv7-a -mfpu=vfpv3 -mfloat-abi=hard -mno-unaligned-access $(CFLAGS_COMMON)
CPPFLAGS = -std=gnu++98 -fno-rtti -Wvla -c -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -fmessage-length=0 -fno-exceptions -fno-builtin -ffunction-sections -fdata-sections -funsigned-char -fno-delete-null-pointer-checks -fomit-frame-pointer -O0 -g3 -mcpu=cortex-a9 -mthumb-interwork -marm -march=armv7-a -mfpu=vfpv3 -mfloat-abi=hard -mno-unaligned-access $(CFLAGS_COMMON)
LD = arm-none-eabi-g++
#LDFLAGS = -nostdlib -Xlinker -T $(TARGET_LD) -Map=$@.map --cref --gc-sections
LDFLAGS = -DMBED_APP_START=0x50000000 -DMBED_APP_SIZE=0x1000000 -DMBED_ROM_START=0x50000000 -DMBED_ROM_SIZE=0x1000000 -DMBED_RAM_START=0x400000 -DMBED_RAM_SIZE=0x2000000 -DMBED_BOOT_STACK_SIZE=1024 -T $(TARGET_LD) -Wl,-Map=$@.map -Wl,--gc-sections -Wl,--wrap,main -Wl,--wrap,_malloc_r -Wl,--wrap,_free_r -Wl,--wrap,_realloc_r -Wl,--wrap,_memalign_r -Wl,--wrap,_calloc_r -Wl,--wrap,exit -Wl,--wrap,atexit -Wl,-n -mcpu=cortex-a9 -mthumb-interwork -marm -march=armv7-a -mfpu=vfpv3 -mfloat-abi=hard -mno-unaligned-access -specs=nosys.specs -Wl,--start-group -lstdc++ -lsupc++ -lm -lc -lgcc -lnosys -Wl,--end-group
else
DFU = $(TOP)/tools/dfu.py
PYDFU = $(TOP)/tools/pydfu.py
CFLAGS_CORTEX_M4 = -mthumb -mtune=cortex-m4 -mcpu=cortex-m4 -msoft-float -fsingle-precision-constant -Wdouble-promotion
CFLAGS = $(INC) -Wall -Werror -std=c99 -nostdlib $(CFLAGS_CORTEX_M4) $(COPT)
LDFLAGS = -nostdlib -T stm32f405.ld -Map=$@.map --cref --gc-sections
endif
else
LD = gcc
CFLAGS = -m32 $(INC) -Wall -Werror -std=c99 $(COPT)
LDFLAGS = -m32 -Wl,-Map=$@.map,--cref -Wl,--gc-sections
endif

CSUPEROPT = -Os # save some code space

# Tune for Debugging or Optimization
ifeq ($(DEBUG), 1)
CFLAGS += -O0 -ggdb
else
CFLAGS += -Os -DNDEBUG
CFLAGS += -fdata-sections -ffunction-sections
endif

LIBS =

SRC_C = \
	main.c \
	uart_core.c \
	lib/utils/printf.c \
	lib/utils/stdout_helpers.c \
	lib/utils/pyexec.c \
	lib/libc/string0.c \
	lib/mp-readline/readline.c \
	$(BUILD)/_frozen_mpy.c \

ifeq ($(BOARD), GR_MANGO)
SRC_C += TARGET_RENESAS/$(TARGET)/analogin_api.c
SRC_C += TARGET_RENESAS/$(TARGET)/flash_api.c
SRC_C += TARGET_RENESAS/$(TARGET)/gpio_api.c
SRC_C += TARGET_RENESAS/$(TARGET)/gpio_irq_api.c
SRC_C += TARGET_RENESAS/$(TARGET)/i2c_api.c
SRC_C += TARGET_RENESAS/$(TARGET)/pinmap.c
SRC_C += TARGET_RENESAS/$(TARGET)/port_api.c
SRC_C += TARGET_RENESAS/$(TARGET)/pwmout_api.c
SRC_C += TARGET_RENESAS/$(TARGET)/rtc_api.c
SRC_C += TARGET_RENESAS/$(TARGET)/serial_api.c
SRC_C += TARGET_RENESAS/$(TARGET)/sleep.c
SRC_C += TARGET_RENESAS/$(TARGET)/spi_api.c
SRC_C += TARGET_RENESAS/$(TARGET)/us_ticker.c
SRC_C += TARGET_RENESAS/$(TARGET)/common/rza_io_regrw.c
SRC_C += TARGET_RENESAS/$(TARGET)/common/r_cache/src/lld/r_cache_lld_rza2m.c
SRC_C += TARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/PeripheralPins.c
SRC_C += TARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/cmsis_nvic.c
SRC_C += TARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/mbed_sf_boot.c
SRC_C += TARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/mmu_RZ_A2M.c
SRC_C += TARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/nvic_wrapper.c
SRC_C += TARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/octaram_init.c
SRC_C += TARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/os_tick_ostm.c
SRC_C += TARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/RZ_A2_Init.c
SRC_C += TARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/system_RZ_A2M.c

SRC_C += $(TOP)/lib/mbed-os/cmsis/TARGET_CORTEX_A/irq_ctrl_gic.c
SRC_C += $(TOP)/lib/mbed-os/hal/mbed_critical_section_api.c
SRC_C += $(TOP)/lib/mbed-os/hal/mbed_flash_api.c
SRC_C += $(TOP)/lib/mbed-os/hal/mbed_gpio.c
SRC_C += $(TOP)/lib/mbed-os/hal/mbed_itm_api.c
SRC_C += $(TOP)/lib/mbed-os/hal/mbed_lp_ticker_api.c
SRC_C += $(TOP)/lib/mbed-os/hal/mbed_pinmap_common.c
SRC_C += $(TOP)/lib/mbed-os/hal/mbed_pinmap_default.c
SRC_C += $(TOP)/lib/mbed-os/hal/mbed_ticker_api.c
SRC_C += $(TOP)/lib/mbed-os/hal/mbed_us_ticker_api.c
SRC_C += $(TOP)/lib/mbed-os/hal/mpu/mbed_mpu_v7m.c
SRC_C += $(TOP)/lib/mbed-os/hal/mpu/mbed_mpu_v8m.c
SRC_C += $(TOP)/lib/mbed-os/platform/mbed_application.c
SRC_C += $(TOP)/lib/mbed-os/platform/mbed_assert.c
SRC_C += $(TOP)/lib/mbed-os/platform/mbed_board.c
SRC_C += $(TOP)/lib/mbed-os/platform/mbed_critical.c
SRC_C += $(TOP)/lib/mbed-os/platform/mbed_error.c
SRC_C += $(TOP)/lib/mbed-os/platform/mbed_error_hist.c
SRC_C += $(TOP)/lib/mbed-os/platform/mbed_interface.c
SRC_C += $(TOP)/lib/mbed-os/platform/mbed_mktime.c
SRC_C += $(TOP)/lib/mbed-os/platform/mbed_mpu_mgmt.c
SRC_C += $(TOP)/lib/mbed-os/platform/mbed_sdk_boot.c
SRC_C += $(TOP)/lib/mbed-os/platform/mbed_semihost_api.c
SRC_C += $(TOP)/lib/mbed-os/platform/mbed_sleep_manager.c
SRC_C += $(TOP)/lib/mbed-os/platform/mbed_wait_api_no_rtos.c
SRC_C += $(TOP)/lib/mbed-os/platform/mbed_stats.c
SRC_C += $(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/mbed_boot.c
SRC_C += $(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/mbed_rtos_rtx.c
SRC_C += $(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/mbed_rtx_handlers.c
SRC_C += $(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/rtx4/cmsis_os1.c
SRC_C += $(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/rtx5/Source/os_systick.c
SRC_C += $(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Config/TARGET_CORTEX_A/handlers.c
SRC_C += $(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Config/RTX_Config.c
SRC_C += $(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/rtx5/Source/os_tick_ptim.c
SRC_C += $(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source/rtx_delay.c
SRC_C += $(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source/rtx_evflags.c
SRC_C += $(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source/rtx_evr.c
SRC_C += $(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source/rtx_kernel.c
SRC_C += $(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source/rtx_lib.c
SRC_C += $(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source/rtx_memory.c
SRC_C += $(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source/rtx_mempool.c
SRC_C += $(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source/rtx_msgqueue.c
SRC_C += $(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source/rtx_mutex.c
SRC_C += $(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source/rtx_semaphore.c
SRC_C += $(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source/rtx_system.c
SRC_C += $(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source/rtx_thread.c
SRC_C += $(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source/rtx_timer.c
SRC_S += $(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source/TOOLCHAIN_GCC/TARGET_CORTEX_A/irq_ca.S

SRC_S += TARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/TOOLCHAIN_GCC_ARM/startup_RZA2M.S
SRC_S += TARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/TOOLCHAIN_GCC_ARM/weak_handler.S

SRC_CPP += $(TOP)/lib/mbed-os/drivers/AnalogIn.cpp
SRC_CPP += $(TOP)/lib/mbed-os/drivers/BusIn.cpp
SRC_CPP += $(TOP)/lib/mbed-os/drivers/BusInOut.cpp
SRC_CPP += $(TOP)/lib/mbed-os/drivers/BusOut.cpp
SRC_CPP += $(TOP)/lib/mbed-os/drivers/CAN.cpp
SRC_CPP += $(TOP)/lib/mbed-os/drivers/Ethernet.cpp
SRC_CPP += $(TOP)/lib/mbed-os/drivers/FlashIAP.cpp
SRC_CPP += $(TOP)/lib/mbed-os/drivers/I2C.cpp
SRC_CPP += $(TOP)/lib/mbed-os/drivers/I2CSlave.cpp
SRC_CPP += $(TOP)/lib/mbed-os/drivers/InterruptIn.cpp
SRC_CPP += $(TOP)/lib/mbed-os/drivers/InterruptManager.cpp
SRC_CPP += $(TOP)/lib/mbed-os/drivers/MbedCRC.cpp
SRC_CPP += $(TOP)/lib/mbed-os/drivers/QSPI.cpp
SRC_CPP += $(TOP)/lib/mbed-os/drivers/RawSerial.cpp
SRC_CPP += $(TOP)/lib/mbed-os/drivers/Serial.cpp
SRC_CPP += $(TOP)/lib/mbed-os/drivers/SerialBase.cpp
SRC_CPP += $(TOP)/lib/mbed-os/drivers/SPI.cpp
SRC_CPP += $(TOP)/lib/mbed-os/drivers/SPISlave.cpp
SRC_CPP += $(TOP)/lib/mbed-os/drivers/TableCRC.cpp
SRC_CPP += $(TOP)/lib/mbed-os/drivers/Ticker.cpp
SRC_CPP += $(TOP)/lib/mbed-os/drivers/Timeout.cpp
SRC_CPP += $(TOP)/lib/mbed-os/drivers/Timer.cpp
SRC_CPP += $(TOP)/lib/mbed-os/drivers/TimerEvent.cpp
SRC_CPP += $(TOP)/lib/mbed-os/drivers/UARTSerial.cpp
SRC_CPP += $(TOP)/lib/mbed-os/hal/LowPowerTickerWrapper.cpp
SRC_CPP += $(TOP)/lib/mbed-os/hal/mbed_lp_ticker_wrapper.cpp
SRC_CPP += $(TOP)/lib/mbed-os/platform/ATCmdParser.cpp
SRC_CPP += $(TOP)/lib/mbed-os/platform/CallChain.cpp
SRC_CPP += $(TOP)/lib/mbed-os/platform/CThunkBase.cpp
SRC_CPP += $(TOP)/lib/mbed-os/platform/FileBase.cpp
SRC_CPP += $(TOP)/lib/mbed-os/platform/FileHandle.cpp
SRC_CPP += $(TOP)/lib/mbed-os/platform/FilePath.cpp
SRC_CPP += $(TOP)/lib/mbed-os/platform/FileSystemHandle.cpp
SRC_CPP += $(TOP)/lib/mbed-os/platform/LocalFileSystem.cpp
SRC_CPP += $(TOP)/lib/mbed-os/platform/mbed_alloc_wrappers.cpp
SRC_CPP += $(TOP)/lib/mbed-os/platform/mbed_mem_trace.cpp
SRC_CPP += $(TOP)/lib/mbed-os/platform/mbed_retarget.cpp
SRC_CPP += $(TOP)/lib/mbed-os/platform/mbed_rtc_time.cpp
SRC_CPP += $(TOP)/lib/mbed-os/platform/mbed_wait_api_rtos.cpp
SRC_CPP += $(TOP)/lib/mbed-os/platform/Stream.cpp
SRC_CPP += $(TOP)/lib/mbed-os/rtos/ConditionVariable.cpp
SRC_CPP += $(TOP)/lib/mbed-os/rtos/EventFlags.cpp
SRC_CPP += $(TOP)/lib/mbed-os/rtos/Kernel.cpp
SRC_CPP += $(TOP)/lib/mbed-os/rtos/Mutex.cpp
SRC_CPP += $(TOP)/lib/mbed-os/rtos/RtosTimer.cpp
SRC_CPP += $(TOP)/lib/mbed-os/rtos/Semaphore.cpp
SRC_CPP += $(TOP)/lib/mbed-os/rtos/ThisThread.cpp
SRC_CPP += $(TOP)/lib/mbed-os/rtos/Thread.cpp
SRC_CPP += $(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/mbed_rtx_idle.cpp
SRC_CPP += $(TOP)/lib/mbed-os/rtos/TARGET_CORTEX/SysTimer.cpp
SRC_CPP += $(TOP)/lib/mbed-os/usb/device/hal/mbed_usb_phy.cpp
SRC_CPP += $(TOP)/lib/mbed-os/usb/device/targets/TARGET_RENESAS/$(TARGET)/USBPhy_RZ_A2.cpp
SRC_CPP += $(TOP)/lib/mbed-os/usb/device/USBAudio/USBAudio.cpp
SRC_CPP += $(TOP)/lib/mbed-os/usb/device/USBDevice/EndpointResolver.cpp
SRC_CPP += $(TOP)/lib/mbed-os/usb/device/USBDevice/USBDevice.cpp
SRC_CPP += $(TOP)/lib/mbed-os/usb/device/USBHID/USBHID.cpp
SRC_CPP += $(TOP)/lib/mbed-os/usb/device/USBHID/USBKeyboard.cpp
SRC_CPP += $(TOP)/lib/mbed-os/usb/device/USBHID/USBMouse.cpp
SRC_CPP += $(TOP)/lib/mbed-os/usb/device/USBHID/USBMouseKeyboard.cpp
SRC_CPP += $(TOP)/lib/mbed-os/usb/device/USBMIDI/USBMIDI.cpp
#SRC_CPP += $(TOP)/lib/mbed-os/usb/device/USBMSD/USBMSD.cpp
SRC_CPP += $(TOP)/lib/mbed-os/usb/device/USBSerial/USBSerial.cpp
SRC_CPP += $(TOP)/lib/mbed-os/usb/device/USBSerial/USBCDC.cpp
SRC_CPP += $(TOP)/lib/mbed-os/usb/device/utilities/AsyncOp.cpp
SRC_CPP += $(TOP)/lib/mbed-os/usb/device/utilities/ByteBuffer.cpp
SRC_CPP += $(TOP)/lib/mbed-os/usb/device/utilities/LinkedListBase.cpp
SRC_CPP += $(TOP)/lib/mbed-os/usb/device/utilities/OperationListBase.cpp
SRC_CPP += $(TOP)/lib/mbed-os/usb/device/utilities/events/PolledQueue.cpp
SRC_CPP += $(TOP)/lib/mbed-os/usb/device/utilities/events/TaskBase.cpp

SRC_C += rz_main.c
SRC_CPP += wrapper.cpp
SRC_CPP += TARGET_RENESAS/$(TARGET)/common/target_override_console.cpp

endif

OBJ = $(PY_CORE_O)
OBJ += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_S:.S=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_CPP:.cpp=.o))
#DEPS += $(addprefix $(BUILD)/, $(SRC_C:.c=.d))
#DEPS += $(addprefix $(BUILD)/, $(SRC_S:.S=.d))
#DEPS += $(addprefix $(BUILD)/, $(SRC_CPP:.cpp=.d))
#-include $(DEPS)

ifeq ($(CROSS), 1)
all: $(BUILD)/firmware.dfu
else
all: $(BUILD)/firmware.elf
endif

$(BUILD)/_frozen_mpy.c: frozentest.mpy $(BUILD)/genhdr/qstrdefs.generated.h
	$(ECHO) "MISC freezing bytecode"
	$(Q)$(TOP)/tools/mpy-tool.py -f -q $(BUILD)/genhdr/qstrdefs.preprocessed.h -mlongint-impl=none $< > $@

$(BUILD)/firmware.elf: $(OBJ)
	$(ECHO) "LINK $@"
	$(Q)$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)
	$(Q)$(SIZE) $@

$(BUILD)/firmware.bin: $(BUILD)/firmware.elf
	$(Q)$(OBJCOPY) -O binary -j .isr_vector -j .text -j .data $^ $(BUILD)/firmware.bin

$(BUILD)/firmware.dfu: $(BUILD)/firmware.bin
	$(ECHO) "Create $@"
#	$(Q)$(PYTHON) $(DFU) -b 0x08000000:$(BUILD)/firmware.bin $@

deploy: $(BUILD)/firmware.dfu
	$(ECHO) "Writing $< to the board"
#	$(Q)$(PYTHON) $(PYDFU) -u $<

# Run emulation build on a POSIX system with suitable terminal settings
run:
	stty raw opost -echo
	build/firmware.elf
	@echo Resetting terminal...
# This sleep is useful to spot segfaults
	sleep 1
	reset

test: $(BUILD)/firmware.elf
	$(Q)/bin/echo -e "print('hello world!', list(x+1 for x in range(10)), end='eol\\\\n')\\r\\n\\x04" | $(BUILD)/firmware.elf | tail -n2 | grep "^hello world! \\[1, 2, 3, 4, 5, 6, 7, 8, 9, 10\\]eol"

include $(TOP)/py/mkrules.mk
