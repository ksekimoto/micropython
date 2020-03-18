# Select the board to build for: if not given on the command line,
# then default to GR_MANGO
BOARD ?= GR_MANGO

CROSS ?= 0

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
TARGET_LD = boards/$(BOARD)/$(BOARD).ld
STACK_USAGE = 16384
LVGL_ENABLE = 0
MBED_GR_LIBS = 0
MBED_HTTP = 0
MBED_CONSOLE = 1
OVERRIDE_CONSOLE_USBSERIAL = 0
endif

include ../../py/mkenv.mk
-include mpconfigport.mk
include $(BOARD_DIR)/mpconfigboard.mk

# qstr definitions (must come before including py.mk)
QSTR_DEFS = qstrdefsport.h $(BUILD)/pins_qstr.h 
QSTR_GLOBAL_DEPENDENCIES = mpconfigboard_common.h $(BOARD_DIR)/mpconfigboard.h

# File containing description of content to be frozen into firmware.
FROZEN_MANIFEST ?= boards/manifest.py
FROZEN_DIR =
FROZEN_MPY_DIR =
# include py core make definitions
include $(TOP)/py/py.mk

#GIT_SUBMODULES = lib/lwip lib/mbedtls lib/mynewt-nimble

#######################################################################
# MCU configuration
#######################################################################

# Select the cross compile prefix
CROSS_COMPILE ?= arm-none-eabi-

# MCU_SERIERS is defined in mpconfigboard.mk file under board folder
ifeq ($(MCU_SERIES),RZA2M)
MCU_SERIES_LOW = rza2m
MCU_SERIES_CAP = RZA2M
endif

#######################################################################
# C include
#######################################################################
INC += -I.
INC += -I$(TOP)
INC += -I$(BUILD)

INC += -I$(MCU_SERIES_LOW)
INC += -Iboards/$(BOARD)
INC += -Imbed
INC += -Ilv

ifeq ($(TARGET), TARGET_RZ_A2XX)
INC_MBED += -ITARGET_RENESAS
INC_MBED += -ITARGET_RENESAS/$(TARGET)
INC_MBED += -ITARGET_RENESAS/$(TARGET)/common
INC_MBED += -ITARGET_RENESAS/$(TARGET)/common/r_cache
INC_MBED += -ITARGET_RENESAS/$(TARGET)/common/r_cache/inc
INC_MBED += -ITARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)
INC_MBED += -ITARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device
INC_MBED += -ITARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/inc
INC_MBED += -ITARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/inc/iodefine
INC_MBED += -ITARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/inc/iodefine/iobitmasks
INC_MBED += -ITARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/inc/iodefine/iodefines

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
INC += -Ilib/mbed-os
INC += -Ilib/mbed-os/hal
INC += -Ilib/mbed-os/drivers
INC += -Ilib/mbed-os/platform
INC += -Ilib/mbed-os/cmsis/TARGET_CORTEX_A
INC += -Ilib/mbed-gr-libs/EasyAttach_CameraAndLCD
INC += -Ilib/mbed-gr-libs/GR-PEACH_video

INC += -Ilib/lv_bindings/lvgl

endif

#######################################################################
# C flags.
#######################################################################

ifeq ($(LVGL_ENABLE),1)
CFLAGS_COMMON += -DLVGL_ENABLE=1
endif

ifeq ($(MBED_GR_LIBS), 1)
CFLAGS_COMMON += -DMBED_GR_LIBS=1
endif

ifeq ($(TARGET), TARGET_RZ_A2XX)
LIB_MBED = libmbed.a
LD = gcc
#-Wstack-usage=$(STACK_USAGE)
CFLAGS_MBED_COMMON += $(INC_MBED) -DAPPLICATION_ADDR=0x50000000 -DAPPLICATION_SIZE=0x1000000 -DMBED_ROM_START=0x50000000 -DMBED_ROM_SIZE=0x1000000 -DMBED_RAM_START=0x400000 -DMBED_RAM_SIZE=0x2000000 -DARM_MATH_CA9 -D__FPU_PRESENT -DDEVICE_EMAC=1 -D__MBED__=1 -DDEVICE_USBDEVICE=1 -DTARGET_LIKE_MBED -DDEVICE_PORTINOUT=1 -DMBED_BUILD_TIMESTAMP=1582947353.33 -DCOMPONENT_FLASHIAP=1 -DMBEDTLS_NO_DEFAULT_ENTROPY_SOURCES -DCOMPONENT_PSA_SRV_EMUL=1 -DDEVICE_SERIAL_ASYNCH=1 -D__CMSIS_RTOS -DMBEDTLS_USER_CONFIG_FILE="mbedtls_entropy_config.h" -D__EVAL -DDEVICE_SPISLAVE=1 -DTOOLCHAIN_GCC -DTARGET_CORTEX_A -DDEVICE_I2C_ASYNCH=1 -DTARGET_DEBUG -DDEVICE_RTC=1 -D__MBED_CMSIS_RTOS_CA9 -DCOMPONENT_PSA_SRV_IMPL=1 -DTARGET_LIKE_CORTEX_A9 -DDEVICE_PWMOUT=1 -DDEVICE_SPI_ASYNCH=1 -DDEVICE_INTERRUPTIN=1 -DTARGET_CORTEX -DDEVICE_I2C=1 -DDEVICE_PORTOUT=1 -DDEVICE_I2CSLAVE=1 -DDEVICE_USTICKER=1 -DDEVICE_STDIO_MESSAGES=1 -DTARGET_MBRZA2M -DTARGET_RENESAS -DTARGET_RZA2M -DTARGET_NAME=GR_MANGO -DCOMPONENT_NSPE=1 -DDEVICE_SERIAL_FC=1 -DMBEDTLS_TEST_NULL_ENTROPY -DTARGET_A9 -DTARGET_RZ_A2XX -D__CORTEX_A9 -DDEVICE_PORTIN=1 -DDEVICE_SLEEP=1 -DTOOLCHAIN_GCC_ARM -DDEVICE_SPI=1 -DTARGET_GR_MANGO -DDEVICE_ANALOGIN=1 -DDEVICE_SERIAL=1 -DDEVICE_FLASH=1 -DMBED_CONF_APP_MAIN_STACK_SIZE=8192 -DTARGET_RZ_A2_EMAC
CFLAGS_MBED_COMMON += -DMBED_CONF_PLATFORM_CTHUNK_COUNT_MAX=1
CFLAGS_MBED_COMMON += -DMBED_CONF_PLATFORM_DEFAULT_SERIAL_BAUD_RATE=115200
CFLAGS_MBED_COMMON += -DMBED_CONF_PLATFORM_STDIO_BAUD_RATE=115200
CFLAGS_MBED_COMMON += -DMBED_CONF_RTOS_THREAD_STACK_SIZE=4096
#CFLAGS_COMMON += -include lib/mbed-os/mbed_config.h
#ifeq ($(MBED_OS_USE), 1)
CFLAGS_MBED_COMMON += @./mbed-os-include.txt
#endif
ifeq ($(MBED_GR_LIBS), 1)
CFLAGS_MBED_COMMON += @./mbed-gr-libs-include.txt
ifeq ($(OVERRIDE_CONSOLE_USBSERIAL), 1)
endif
CFLAGS_MBED_COMMON += -DOVERRIDE_CONSOLE_USBSERIAL
endif
ifeq ($(MBED_CONSOLE), 1)
CFLAGS_MBED_COMMON += -DMBED_CONSOLE
endif
CFLAGS_MBED_COMMON += -DMBED_CONF_APP_LCD=1
CFLAGS_MBED_COMMON += -DMBED_CONF_APP_CAMERA=1
CFLAGS_MBED_COMMON += -DCAMERA_MODULE=MODULE_VDC
CFLAGS_MBED_COMMON += -DMBED_CONF_APP_CAMERA_TYPE=CAMERA_OV7725
CFLAGS_MBED_COMMON += -DMBED_CONF_APP_LCD_TYPE=RGB_TO_HDMI
#CFLAGS_COMMON += -DDEVICE_SERIAL=1 -DDEVICE_SERIAL_ASYNCH=1 -DMBED_CONF_PLATFORM_CTHUNK_COUNT_MAX=1 -DMBED_CONF_PLATFORM_DEFAULT_SERIAL_BAUD_RATE=115200 
CFLAGS_COMMON += $(INC)
CFLAGS_COMMON += $(CFLAGS_MBED_COMMON)
#CFLAGS_COMMON += -MMD
#CFLAGS_COMMON += -MF $(@:%.o=%.d)
CFLAGS += -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -fmessage-length=0 -fno-exceptions -fno-builtin -ffunction-sections -fdata-sections -funsigned-char -fno-delete-null-pointer-checks -fomit-frame-pointer -O0 -g3 -DMBED_DEBUG -DMBED_TRAP_ERRORS_ENABLED=1 -mcpu=cortex-a9 -mthumb-interwork -marm -march=armv7-a -mfpu=vfpv3 -mfloat-abi=hard -mno-unaligned-access $(CFLAGS_COMMON)
CPPFLAGS += -std=gnu++98 -fno-rtti -Wvla -c -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -fmessage-length=0 -fno-exceptions -fno-builtin -ffunction-sections -fdata-sections -funsigned-char -fno-delete-null-pointer-checks -fomit-frame-pointer -O0 -g3 -mcpu=cortex-a9 -mthumb-interwork -marm -march=armv7-a -mfpu=vfpv3 -mfloat-abi=hard -mno-unaligned-access $(CFLAGS_COMMON)
LD = arm-none-eabi-g++
#LDFLAGS = -nostdlib -Xlinker -T $(TARGET_LD) -Map=$@.map --cref --gc-sections
LDFLAGS += -DMBED_APP_START=0x50000000 -DMBED_APP_SIZE=0x1000000 -DMBED_ROM_START=0x50000000 -DMBED_ROM_SIZE=0x1000000 -DMBED_RAM_START=0x400000 -DMBED_RAM_SIZE=0x2000000 -DMBED_BOOT_STACK_SIZE=1024 -T $(TARGET_LD) -Wl,-Map=$@.map -Wl,--gc-sections -Wl,--wrap,main -Wl,--wrap,_memalign_r -Wl,--wrap,_calloc_r -Wl,--wrap,exit -Wl,--wrap,atexit -Wl,-n -mcpu=cortex-a9 -mthumb-interwork -marm -march=armv7-a -mfpu=vfpv3 -mfloat-abi=hard -mno-unaligned-access -specs=nosys.specs -Wl,--start-group -lstdc++ -lsupc++ -lm -lc -lgcc -lnosys -Wl,--end-group
#LDFLAGS += -Wl,--wrap,_malloc_r -Wl,--wrap,_free_r -Wl,--wrap,_realloc_r 
#LDFLAGS += -lmbed -Llibs

#CFLAGS += -D$(MCU_SERIES_CAP)
#CFLAGS += -DCONFIG_$(MCU_SERIES_CAP)
ifeq ($(BOARD),GR_MANGO)
CFLAGS += -DGRMANGO
endif
CFLAGS += -DFFCONF_H=\"lib/oofatfs/ffconf.h\"

CSUPEROPT = -Os # save some code space

# Tune for Debugging or Optimization
ifeq ($(DEBUG), 1)
CFLAGS += -O0 -ggdb
else
CFLAGS += -Os -DNDEBUG
CFLAGS += -fdata-sections -ffunction-sections
endif

endif

#######################################################################
# source files
#######################################################################

SRC_C = \
	main.c \
	lib/utils/interrupt_char.c \
	lib/utils/printf.c \
	lib/utils/pyexec.c \
	lib/libc/string0.c \
	lib/mp-readline/readline.c \
#	$(BUILD)/_frozen_mpy.c \

SRC_MBED_C += TARGET_RENESAS/$(TARGET)/analogin_api.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/flash_api.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/gpio_api.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/gpio_irq_api.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/i2c_api.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/pinmap.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/port_api.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/pwmout_api.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/rtc_api.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/serial_api.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/sleep.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/spi_api.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/us_ticker.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/common/rza_io_regrw.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/common/r_cache/src/lld/r_cache_lld_rza2m.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/PeripheralPins.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/cmsis_nvic.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/mbed_sf_boot.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/mmu_RZ_A2M.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/nvic_wrapper.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/octaram_init.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/os_tick_ostm.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/RZ_A2_Init.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/system_RZ_A2M.c

SRC_MBED_C += lib/mbed-os/cmsis/TARGET_CORTEX_A/irq_ctrl_gic.c
SRC_MBED_C += lib/mbed-os/hal/mbed_critical_section_api.c
SRC_MBED_C += lib/mbed-os/hal/mbed_flash_api.c
SRC_MBED_C += lib/mbed-os/hal/mbed_gpio.c
SRC_MBED_C += lib/mbed-os/hal/mbed_itm_api.c
SRC_MBED_C += lib/mbed-os/hal/mbed_lp_ticker_api.c
SRC_MBED_C += lib/mbed-os/hal/mbed_pinmap_common.c
SRC_MBED_C += lib/mbed-os/hal/mbed_pinmap_default.c
SRC_MBED_C += lib/mbed-os/hal/mbed_ticker_api.c
SRC_MBED_C += lib/mbed-os/hal/mbed_us_ticker_api.c
SRC_MBED_C += lib/mbed-os/hal/mpu/mbed_mpu_v7m.c
SRC_MBED_C += lib/mbed-os/hal/mpu/mbed_mpu_v8m.c
SRC_MBED_C += lib/mbed-os/platform/mbed_application.c
SRC_MBED_C += lib/mbed-os/platform/mbed_assert.c
SRC_MBED_C += lib/mbed-os/platform/mbed_board.c
SRC_MBED_C += lib/mbed-os/platform/mbed_critical.c
SRC_MBED_C += lib/mbed-os/platform/mbed_error.c
SRC_MBED_C += lib/mbed-os/platform/mbed_error_hist.c
SRC_MBED_C += lib/mbed-os/platform/mbed_interface.c
SRC_MBED_C += lib/mbed-os/platform/mbed_mktime.c
SRC_MBED_C += lib/mbed-os/platform/mbed_mpu_mgmt.c
SRC_MBED_C += lib/mbed-os/platform/mbed_sdk_boot.c
SRC_MBED_C += lib/mbed-os/platform/mbed_semihost_api.c
SRC_MBED_C += lib/mbed-os/platform/mbed_sleep_manager.c
SRC_MBED_C += lib/mbed-os/platform/mbed_wait_api_no_rtos.c
SRC_MBED_C += lib/mbed-os/platform/mbed_stats.c
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/mbed_boot.c
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/mbed_rtos_rtx.c
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/mbed_rtx_handlers.c
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/rtx4/cmsis_os1.c
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/rtx5/Source/os_systick.c
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Config/TARGET_CORTEX_A/handlers.c
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Config/RTX_Config.c
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/rtx5/Source/os_tick_ptim.c
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source/rtx_delay.c
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source/rtx_evflags.c
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source/rtx_evr.c
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source/rtx_kernel.c
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source/rtx_lib.c
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source/rtx_memory.c
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source/rtx_mempool.c
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source/rtx_msgqueue.c
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source/rtx_mutex.c
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source/rtx_semaphore.c
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source/rtx_system.c
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source/rtx_thread.c
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source/rtx_timer.c

SRC_MBED_S += lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Source/TOOLCHAIN_GCC/TARGET_CORTEX_A/irq_ca.S
SRC_MBED_S += TARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/TOOLCHAIN_GCC_ARM/startup_RZA2M.S
SRC_MBED_S += TARGET_RENESAS/$(TARGET)/TARGET_$(BOARD)/device/TOOLCHAIN_GCC_ARM/weak_handler.S

SRC_MBED_CPP += lib/mbed-os/drivers/AnalogIn.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/BusIn.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/BusInOut.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/BusOut.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/CAN.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/Ethernet.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/FlashIAP.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/I2C.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/I2CSlave.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/InterruptIn.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/InterruptManager.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/MbedCRC.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/QSPI.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/RawSerial.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/Serial.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/SerialBase.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/SPI.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/SPISlave.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/TableCRC.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/Ticker.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/Timeout.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/Timer.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/TimerEvent.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/UARTSerial.cpp
SRC_MBED_CPP += lib/mbed-os/hal/LowPowerTickerWrapper.cpp
SRC_MBED_CPP += lib/mbed-os/hal/mbed_lp_ticker_wrapper.cpp
SRC_MBED_CPP += lib/mbed-os/platform/ATCmdParser.cpp
SRC_MBED_CPP += lib/mbed-os/platform/CallChain.cpp
SRC_MBED_CPP += lib/mbed-os/platform/CThunkBase.cpp
SRC_MBED_CPP += lib/mbed-os/platform/FileBase.cpp
SRC_MBED_CPP += lib/mbed-os/platform/FileHandle.cpp
SRC_MBED_CPP += lib/mbed-os/platform/FilePath.cpp
SRC_MBED_CPP += lib/mbed-os/platform/FileSystemHandle.cpp
SRC_MBED_CPP += lib/mbed-os/platform/LocalFileSystem.cpp
SRC_MBED_CPP += lib/mbed-os/platform/mbed_alloc_wrappers.cpp
SRC_MBED_CPP += lib/mbed-os/platform/mbed_mem_trace.cpp
SRC_MBED_CPP += lib/mbed-os/platform/mbed_retarget.cpp
SRC_MBED_CPP += lib/mbed-os/platform/mbed_rtc_time.cpp
SRC_MBED_CPP += lib/mbed-os/platform/mbed_wait_api_rtos.cpp
SRC_MBED_CPP += lib/mbed-os/platform/Stream.cpp
SRC_MBED_CPP += lib/mbed-os/rtos/ConditionVariable.cpp
SRC_MBED_CPP += lib/mbed-os/rtos/EventFlags.cpp
SRC_MBED_CPP += lib/mbed-os/rtos/Kernel.cpp
SRC_MBED_CPP += lib/mbed-os/rtos/Mutex.cpp
SRC_MBED_CPP += lib/mbed-os/rtos/RtosTimer.cpp
SRC_MBED_CPP += lib/mbed-os/rtos/Semaphore.cpp
SRC_MBED_CPP += lib/mbed-os/rtos/ThisThread.cpp
SRC_MBED_CPP += lib/mbed-os/rtos/Thread.cpp
SRC_MBED_CPP += lib/mbed-os/rtos/TARGET_CORTEX/mbed_rtx_idle.cpp
SRC_MBED_CPP += lib/mbed-os/rtos/TARGET_CORTEX/SysTimer.cpp
SRC_MBED_CPP += lib/mbed-os/usb/device/hal/mbed_usb_phy.cpp
SRC_MBED_CPP += lib/mbed-os/usb/device/targets/TARGET_RENESAS/$(TARGET)/USBPhy_RZ_A2.cpp
SRC_MBED_CPP += lib/mbed-os/usb/device/USBAudio/USBAudio.cpp
SRC_MBED_CPP += lib/mbed-os/usb/device/USBDevice/EndpointResolver.cpp
SRC_MBED_CPP += lib/mbed-os/usb/device/USBDevice/USBDevice.cpp
SRC_MBED_CPP += lib/mbed-os/usb/device/USBHID/USBHID.cpp
SRC_MBED_CPP += lib/mbed-os/usb/device/USBHID/USBKeyboard.cpp
SRC_MBED_CPP += lib/mbed-os/usb/device/USBHID/USBMouse.cpp
SRC_MBED_CPP += lib/mbed-os/usb/device/USBHID/USBMouseKeyboard.cpp
SRC_MBED_CPP += lib/mbed-os/usb/device/USBMIDI/USBMIDI.cpp
#SRC_MBED_CPP += lib/mbed-os/usb/device/USBMSD/USBMSD.cpp
SRC_MBED_CPP += lib/mbed-os/usb/device/USBSerial/USBSerial.cpp
SRC_MBED_CPP += lib/mbed-os/usb/device/USBSerial/USBCDC.cpp
SRC_MBED_CPP += lib/mbed-os/usb/device/utilities/AsyncOp.cpp
SRC_MBED_CPP += lib/mbed-os/usb/device/utilities/ByteBuffer.cpp
SRC_MBED_CPP += lib/mbed-os/usb/device/utilities/LinkedListBase.cpp
SRC_MBED_CPP += lib/mbed-os/usb/device/utilities/OperationListBase.cpp
SRC_MBED_CPP += lib/mbed-os/usb/device/utilities/events/PolledQueue.cpp
SRC_MBED_CPP += lib/mbed-os/usb/device/utilities/events/TaskBase.cpp

ifeq ($(MBED_GR_LIBS), 1)
#SRC_MBED_CPP += lib/mbed-gr-libs/AlarmTimer/$(TARGET)/AlarmTimer.cpp
SRC_MBED_CPP += lib/mbed-gr-libs/components/LCD/Display_shield_config/LcdCfg_Display_shield.cpp
SRC_MBED_CPP += lib/mbed-gr-libs/components/LCD/LCD_config_40pin/LcdCfg_40pin_4_3inch.cpp
SRC_MBED_CPP += lib/mbed-gr-libs/components/LCD/LCD_config_lvds_to_hdmi/LcdCfg_lvds_to_hdmi.cpp
SRC_MBED_CPP += lib/mbed-gr-libs/components/LCD/LCD_config_rgb_to_hdmi/LcdCfg_rgb_to_hdmi.cpp
SRC_MBED_CPP += lib/mbed-gr-libs/components/LCD/LCD_config_RSK_TFT/LcdCfg_RSK_TFT.cpp
#SRC_MBED_CPP += lib/mbed-gr-libs/components/LCD/LCD_shield_config/LcdCfg_LCD_shield/LcdCfg_4_3inch.cpp
#SRC_MBED_CPP += lib/mbed-gr-libs/components/LCD/LCD_shield_config/LcdCfg_LCD_shield/LcdCfg_7_1inch.cpp
#SRC_MBED_CPP += lib/mbed-gr-libs/components/LCD/LCD_shield_config/TouchKey_LCD_shield/TouchKey_4_3inch.cpp
#SRC_MBED_CPP += lib/mbed-gr-libs/components/LCD/LCD_shield_config/TouchKey_LCD_shield/TouchKey_7_1inch.cpp
#SRC_MBED_CPP += lib/mbed-gr-libs/components/LCD/LCD_shield_config/TouchKey_LCD_shield/TouchKey_RSK_TFT.cpp
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/src/r_dk2_core.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/src/r_dk2_if.c
SRC_MBED_CPP += lib/mbed-gr-libs/EasyAttach_CameraAndLCD/EasyAttach_CameraAndLCD.cpp
SRC_MBED_C += lib/mbed-gr-libs/GraphicsFramework/jcu/jcu_driver/converter_wrapper.c
SRC_MBED_C += lib/mbed-gr-libs/GraphicsFramework/jcu/jcu_driver/converter_wrapper.c
SRC_MBED_C += lib/mbed-gr-libs/GraphicsFramework/jcu/jcu_driver/jcu_api.c
SRC_MBED_C += lib/mbed-gr-libs/GraphicsFramework/jcu/jcu_driver/jcu_para.c
SRC_MBED_CPP += lib/mbed-gr-libs/GraphicsFramework/jcu/jcu_driver/JPEG_Coverter.cpp
SRC_MBED_C += lib/mbed-gr-libs/GraphicsFramework/jcu/jcu_driver/$(TARGET)/jcu_reg.c
SRC_MBED_CPP += lib/mbed-gr-libs/GR-PEACH_video/DisplayBase.cpp
SRC_MBED_C += lib/mbed-gr-libs/GR-PEACH_video/targets/$(TARGET)/drivers/ceu/src/r_ceu_driver.c
SRC_MBED_C += lib/mbed-gr-libs/GR-PEACH_video/targets/$(TARGET)/drivers/ceu/userdef/ceu_userdef.c
SRC_MBED_C += lib/mbed-gr-libs/GR-PEACH_video/targets/$(TARGET)/drivers/r_mipi/src/r_mipi_api.c
SRC_MBED_C += lib/mbed-gr-libs/GR-PEACH_video/targets/$(TARGET)/drivers/r_mipi/userdef/r_mipi_userdef_api.c
SRC_MBED_C += lib/mbed-gr-libs/GR-PEACH_video/targets/$(TARGET)/TARGET_RZA2M/rz_a2m_evb_vdc.c
SRC_MBED_C += lib/mbed-gr-libs/GR-PEACH_video/targets/$(TARGET)/TARGET_RZA2M/lvds/lvds_pll_data.c
SRC_MBED_C += lib/mbed-gr-libs/GR-PEACH_video/targets/$(TARGET)/TARGET_RZA2M/lvds/lvds_pll_main.c
SRC_MBED_C += lib/mbed-gr-libs/GR-PEACH_video/targets/$(TARGET)/TARGET_RZA2M/drivers/vdc/src/r_spea.c
SRC_MBED_C += lib/mbed-gr-libs/GR-PEACH_video/targets/$(TARGET)/TARGET_RZA2M/drivers/vdc/src/r_spea_check_parameter.c
SRC_MBED_C += lib/mbed-gr-libs/GR-PEACH_video/targets/$(TARGET)/TARGET_RZA2M/drivers/vdc/src/r_spea_register.c
SRC_MBED_C += lib/mbed-gr-libs/GR-PEACH_video/targets/$(TARGET)/TARGET_RZA2M/drivers/vdc/src/r_spea_register_address.c
SRC_MBED_C += lib/mbed-gr-libs/GR-PEACH_video/targets/$(TARGET)/TARGET_RZA2M/drivers/vdc/src/r_vdc.c
SRC_MBED_C += lib/mbed-gr-libs/GR-PEACH_video/targets/$(TARGET)/TARGET_RZA2M/drivers/vdc/src/r_vdc_check_parameter.c
SRC_MBED_C += lib/mbed-gr-libs/GR-PEACH_video/targets/$(TARGET)/TARGET_RZA2M/drivers/vdc/src/r_vdc_interrupt.c
SRC_MBED_C += lib/mbed-gr-libs/GR-PEACH_video/targets/$(TARGET)/TARGET_RZA2M/drivers/vdc/src/r_vdc_register.c
SRC_MBED_C += lib/mbed-gr-libs/GR-PEACH_video/targets/$(TARGET)/TARGET_RZA2M/drivers/vdc/src/r_vdc_register_address.c
SRC_MBED_C += lib/mbed-gr-libs/GR-PEACH_video/targets/$(TARGET)/TARGET_RZA2M/drivers/vdc/src/r_vdc_shared_param.c
SRC_MBED_C += lib/mbed-gr-libs/GR-PEACH_video/targets/$(TARGET)/TARGET_RZA2M/drivers/vdc/userdef/spea_userdef.c
SRC_MBED_C += lib/mbed-gr-libs/GR-PEACH_video/targets/$(TARGET)/TARGET_RZA2M/drivers/vdc/userdef/vdc_userdef.c
SRC_MBED_C += lib/mbed-gr-libs/utility/mmu/r_mmu_lld.c
endif

ifeq ($(OVERRIDE_CONSOLE_USBSERIAL), 1)
SRC_MBED_CPP += TARGET_RENESAS/$(TARGET)/common/target_override_console.cpp
endif

ifeq ($(MBED_HTTP), 1)
endif

SRC_C += help.c
SRC_C += gccollect.c
SRC_S += gchelper_a9.S
SRC_C += i2c.c
SRC_C += machine_uart.c
SRC_C += machine_timer.c
SRC_C += modmachine.c
SRC_C += modpyb.c
SRC_C += mphalport.c
SRC_C += led.c
SRC_C += pendsv.c
SRC_C += pin.c
SRC_C += pin_defs_rz.c
SRC_C += pin_named_pins.c
SRC_C += softtimer.c
SRC_C += systick.c
SRC_C += uart.c
SRC_C += $(MCU_SERIES_LOW)/rza2m_gpio.c
SRC_C += $(MCU_SERIES_LOW)/rza2m_ostm2.c
SRC_C += $(MCU_SERIES_LOW)/rza2m_sci.c
SRC_C += $(MCU_SERIES_LOW)/rza2m_utils.c
SRC_CPP += mbed/mbed_timer.cpp
SRC_CPP += mbed/mbed_uart.cpp
ifeq ($(MBED_GR_LIBS), 1)
SRC_C += camera_lcd.c
SRC_CPP += mbed/mbed_camera_lcd.cpp
endif
ifeq ($(LVGL_ENABLE) ,1)
SRC_C += lv/modlvrz.c
endif

#######################################################################
# object files
#######################################################################

OBJ =
OBJ += $(PY_O)
OBJ += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_S:.S=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_CPP:.cpp=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_MOD:.c=.o))
OBJ += $(BUILD)/pins_$(BOARD).o

OBJ_MBED_C += $(addprefix $(BUILD)/, $(SRC_MBED_C:.c=.o))
OBJ_MBED_S += $(addprefix $(BUILD)/, $(SRC_MBED_S:.S=.o))
OBJ_MBED_CPP += $(addprefix $(BUILD)/, $(SRC_MBED_CPP:.cpp=.o))
OBJ_MBED += $(OBJ_MBED_C) 
OBJ_MBED += $(OBJ_MBED_S) 
OBJ_MBED += $(OBJ_MBED_CPP) 

all: $(BUILD)/firmware.elf

#$(OBJ_MBED_S: $(SRC_MBED_S)
#	$(ECHO) "CC $<"
#	$(Q)$(CC) $(CFLAGS) $(CFLAGS_MBED_COMMON) -c -o $@ $<

ifeq ($(EXEC), 1)
vpath %.S $(SRC_MBED_S)
$(BUILD)/%.o: %S
	$(ECHO) "CC $<"
	dirname $@ | xargs mkdir -p
	$(Q)$(CC) $(CFLAGS) $(CFLAGS_MBED_COMMON) -c -o $@ $<

vpath %.c $(SRC_MBED_C)
$(BUILD)/%.o: %c
	$(ECHO) "CC $<"
	dirname $@ | xargs mkdir -p
	$(Q)$(CC) $(CFLAGS) $(CFLAGS_MBED_COMMON) -c -MD -o $@ $<
	@# The following fixes the dependency file.
	@# See http://make.paulandlesley.org/autodep.html for details.
	@# Regex adjusted from the above to play better with Windows paths, etc.
	@$(CP) $(@:.o=.d) $(@:.o=.P); \
  	$(SED) -e 's/#.*//' -e 's/^.*:  *//' -e 's/ *\\$$//' \
      -e '/^$$/ d' -e 's/$$/ :/' < $(@:.o=.d) >> $(@:.o=.P); \
  	$(RM) -f $(@:.o=.d)

vpath %.cpp $(SRC_MBED_CPP)
$(BUILD)/%.o: %cpp
	$(ECHO) "CC $<"
	dirname $@ | xargs mkdir -p
	$(Q)$(CXX) $(CPPFLAGS) $(CFLAGS_MBED_COMMON) -c -MD -o $@ $<
	@# The following fixes the dependency file.
	@# See http://make.paulandlesley.org/autodep.html for details.
	@# Regex adjusted from the above to play better with Windows paths, etc.
	@$(CP) $(@:.o=.d) $(@:.o=.P); \
  	$(SED) -e 's/#.*//' -e 's/^.*:  *//' -e 's/ *\\$$//' \
      -e '/^$$/ d' -e 's/$$/ :/' < $(@:.o=.d) >> $(@:.o=.P); \
  	$(RM) -f $(@:.o=.d)

$(OBJ_MBED): $(OBJ_MBED_S) $(OBJ_MBED_C) $(OBJ_MBED_CPP)

endif

#######################################################################
# build rules
#######################################################################

AR_MBED_FLAGS = rcs

libs/$(LIB_MBED): $(OBJ_MBED)
	$(ECHO) "LIB $@"
	$(MKDIR) -p libs
	$(Q)$(AR) $(AR_MBED_FLAGS) libs/$(LIB_MBED) $^

#$(BUILD)/_frozen_mpy.c: frozentest.mpy #$(BUILD)/genhdr/qstrdefs.generated.h
#	$(ECHO) "MISC freezing bytecode"
#	$(Q)$(TOP)/tools/mpy-tool.py -f -q #$(BUILD)/genhdr/qstrdefs.preprocessed.h -mlongint-impl=none $< > $@

ifneq ($(FROZEN_MANIFEST)$(FROZEN_DIR),)
# To use frozen source modules, put your .py files in a subdirectory (eg scripts/)
# and then invoke make with FROZEN_DIR=scripts (be sure to build from scratch).
CFLAGS += -DMICROPY_MODULE_FROZEN_STR
endif

ifneq ($(FROZEN_MANIFEST)$(FROZEN_MPY_DIR),)
# To use frozen bytecode, put your .py files in a subdirectory (eg frozen/) and
# then invoke make with FROZEN_MPY_DIR=frozen (be sure to build from scratch).
CFLAGS += -DMICROPY_QSTR_EXTRA_POOL=mp_qstr_frozen_const_pool
CFLAGS += -DMICROPY_MODULE_FROZEN_MPY
endif

#$(BUILD)/_frozen_mpy.c: frozentest.mpy $(BUILD)/genhdr/qstrdefs.generated.h
#	$(ECHO) "MISC freezing bytecode"
#	$(Q)$(TOP)/tools/mpy-tool.py -f -q $(BUILD)/genhdr/qstrdefs.preprocessed.h -mlongint-impl=none $< > $@

$(BUILD)/firmware.elf: $(OBJ) libs/$(LIB_MBED)
	$(ECHO) "LINK $@"
	$(Q)$(LD) $(LDFLAGS) -o $@ $^
	$(Q)$(OBJCOPY) -O srec $(BUILD)/firmware.elf  $(BUILD)/firmware.mot	
	$(Q)$(OBJDUMP) -S -D $(BUILD)/firmware.elf > $(BUILD)/firmware.s	
	$(Q)$(SIZE) $@

$(BUILD)/firmware.bin: $(BUILD)/firmware.elf
	$(Q)$(OBJCOPY) -O binary -j .isr_vector -j .text -j .data $^ $(BUILD)/firmware.bin

#$(BUILD)/firmware.dfu: $(BUILD)/firmware.bin
#	$(ECHO) "Create $@"
#	$(Q)$(PYTHON) $(DFU) -b 0x08000000:$(BUILD)/firmware.bin $@

#deploy: $(BUILD)/firmware.dfu
#	$(ECHO) "Writing $< to the board"
#	$(Q)$(PYTHON) $(PYDFU) -u $<

# Run emulation build on a POSIX system with suitable terminal settings
#run:
#	stty raw opost -echo
#	build/firmware.elf
#	@echo Resetting terminal...
# This sleep is useful to spot segfaults
#	sleep 1
#	reset

#test: $(BUILD)/firmware.elf
#	$(Q)/bin/echo -e "print('hello world!', list(x+1 for x in range(10)), end='eol\\\\n')\\r\\n\\x04" | #$(BUILD)/firmware.elf | tail -n2 | grep "^hello world! \\[1, 2, 3, 4, 5, 6, 7, 8, 9, 10\\]eol"

#test: $(BUILD)/firmware.elf
#	$(Q)/bin/echo -e "print('hello world!', list(x+1 for x in range(10)), end='eol\\\\n')\\r\\n\\x04" | $(BUILD)/firmware.elf | tail -n2 | grep "^hello world! \\[1, 2, 3, 4, 5, 6, 7, 8, 9, 10\\]eol"
MAKE_PINS = boards/make-pins.py
BOARD_PINS = $(BOARD_DIR)/pins.csv
AF_FILE = boards/$(MCU_SERIES_LOW)_af.csv
PREFIX_FILE = boards/$(MCU_SERIES_LOW)_prefix.c
GEN_PINS_SRC = $(BUILD)/pins_$(BOARD).c
GEN_PINS_HDR = $(HEADER_BUILD)/pins.h
GEN_PINS_QSTR = $(BUILD)/pins_qstr.h
GEN_PINS_AD_CONST = $(HEADER_BUILD)/pins_ad_const.h
GEN_PINS_AF_CONST = $(HEADER_BUILD)/pins_af_const.h
#GEN_PINS_AF_DEFS = $(HEADER_BUILD)/pins_af_defs.h
GEN_PINS_AF_PY = $(BUILD)/pins_af.py

INSERT_USB_IDS = $(TOP)/tools/insert-usb-ids.py
FILE2H = $(TOP)/tools/file2h.py

USB_IDS_FILE = usb.h
CDCINF_TEMPLATE = pybcdc.inf_template
GEN_CDCINF_FILE = $(HEADER_BUILD)/pybcdc.inf
GEN_CDCINF_HEADER = $(HEADER_BUILD)/pybcdc_inf.h

# List of sources for qstr extraction
SRC_QSTR += $(SRC_C) $(SRC_MOD) $(SRC_LIB) $(EXTMOD_SRC_C)
# Append any auto-generated sources that are needed by sources listed in
# SRC_QSTR
SRC_QSTR_AUTO_DEPS += $(GEN_CDCINF_HEADER)

# Making OBJ use an order-only depenedency on the generated pins.h file
# has the side effect of making the pins.h file before we actually compile
# any of the objects. The normal dependency generation will deal with the
# case when pins.h is modified. But when it doesn't exist, we don't know
# which source files might need it.
$(OBJ): | $(GEN_PINS_HDR)

# With conditional pins, we may need to regenerate qstrdefs.h when config
# options change.
$(HEADER_BUILD)/qstrdefs.generated.h: $(BOARD_DIR)/mpconfigboard.h

# main.c can't be even preprocessed without $(GEN_CDCINF_HEADER)
main.c: $(GEN_CDCINF_HEADER)

# Use a pattern rule here so that make will only call make-pins.py once to make
# both pins_$(BOARD).c and pins.h
.PRECIOUS: $(GEN_PINS_SRC)
$(BUILD)/%_$(BOARD).c $(HEADER_BUILD)/%.h $(HEADER_BUILD)/%_af_const.h $(HEADER_BUILD)/%_af_defs.h $(BUILD)/%_qstr.h: $(BOARD_DIR)/%.csv $(MAKE_PINS) $(AF_FILE) $(PREFIX_FILE) | $(HEADER_BUILD)
	$(ECHO) "GEN $@"
	$(Q)$(PYTHON) $(MAKE_PINS) --board $(BOARD_PINS) --af $(AF_FILE) --prefix $(PREFIX_FILE) --hdr $(GEN_PINS_HDR) --qstr $(GEN_PINS_QSTR) --ad-const $(GEN_PINS_AD_CONST) --af-const $(GEN_PINS_AF_CONST) --af-py $(GEN_PINS_AF_PY) > $(GEN_PINS_SRC)

$(BUILD)/pins_$(BOARD).o: $(GEN_PINS_SRC)
	$(call compile_c)

$(GEN_CDCINF_HEADER): $(GEN_CDCINF_FILE) $(FILE2H)
	$(ECHO) "GEN $@"
	$(Q)$(PYTHON) $(FILE2H) $< > $@

$(GEN_CDCINF_FILE): $(CDCINF_TEMPLATE) $(INSERT_USB_IDS) $(USB_IDS_FILE)
	$(ECHO) "GEN $@"
	$(Q)$(PYTHON) $(INSERT_USB_IDS) $(USB_IDS_FILE) $< > $@

include $(TOP)/py/mkrules.mk
