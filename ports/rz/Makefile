# Select the board to build for: if not given on the command line,
# then default to GR_MANGO
BOARD ?= GR_MANGO
CROSS ?= 1

ifeq ($(wildcard boards/$(BOARD)/.),)
$(error Invalid BOARD specified)
endif

# If the build directory is not given, make it reflect the board name.
BUILD ?= build-$(BOARD)

BOARD_DIR ?= boards/$(BOARD)
ifeq ($(wildcard $(BOARD_DIR)/.),)
$(error Invalid BOARD specified: $(BOARD_DIR))
endif

# enable debug_printf
#USE_DBG_PRINT = 1

# enable micropython debug flag
#MICROPY_DEBUG_VERBOSE = 1
#MICROPY_DEBUG_PRINTERS = 1

ifeq ($(BOARD),GR_MANGO)
CROSS = 1
TARGET = TARGET_RZ_A2XX
TARGET_BOARD = TARGET_GR_MANGO
TARGET_LD = boards/$(BOARD)/$(BOARD).ld
STACK_USAGE = 16384
LVGL_ENABLE = 0
MBED_OS = 1
MBED_OS_EMAC=0
MBED_OS_EVENTS=0
MBED_GR_LIBS = 0
MBED_GR_LIBS_DRP_FOR_MBED = 1
MBED_GR_LIBS_dcache-control = 1
MBED_GR_LIBS_components_LCD = 1
MBED_GR_LIBS_components_CAMERA = 1
MBED_GR_LIBS_EasyAttach_CameraAndLCD = 1
MBED_HTTP = 0
MBED_CONSOLE = 1
OVERRIDE_CONSOLE_USBSERIAL = 0
MICROPY_PY_LWIP = 1
MICROPY_SSL_MBEDTLS = 1
MICROPY_PY_USSL = 1
MBED_UART_WRAPPER = 0
MBED_SPI_WRAPPER = 0
endif

ifeq ($(BOARD),GR_MANGO_DD)
CROSS = 1
BOOT_LOADER = 1
TARGET = TARGET_RZ_A2XX
TARGET_BOARD = TARGET_GR_MANGO
TARGET_LD = boards/$(BOARD)/$(BOARD).ld
STACK_USAGE = 16384
LVGL_ENABLE = 0
MBED_OS = 1
MBED_OS_EMAC=0
MBED_OS_EVENTS=0
MBED_GR_LIBS = 0
MBED_GR_LIBS_DRP_FOR_MBED = 1
MBED_GR_LIBS_dcache-control = 1
MBED_GR_LIBS_components_LCD = 1
MBED_GR_LIBS_components_CAMERA = 1
MBED_GR_LIBS_EasyAttach_CameraAndLCD = 1
MBED_HTTP = 0
MBED_CONSOLE = 1
OVERRIDE_CONSOLE_USBSERIAL = 0
MICROPY_PY_LWIP = 1
MICROPY_SSL_MBEDTLS = 1
MICROPY_PY_USSL = 1
MBED_UART_WRAPPER = 0
MBED_SPI_WRAPPER = 0
endif

# LAN debug
LAN_NEW = 0

include ../../py/mkenv.mk
-include mpconfigport.mk
include $(BOARD_DIR)/mpconfigboard.mk

# qstr definitions (must come before including py.mk)
QSTR_DEFS = qstrdefsport.h $(BUILD)/pins_qstr.h 
QSTR_GLOBAL_DEPENDENCIES = mpconfigboard_common.h $(BOARD_DIR)/mpconfigboard.h

# File containing description of content to be frozen into firmware.
FROZEN_MANIFEST ?= boards/manifest.py

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
INC += -Ilvgl
ifeq ($(USE_DBG_PRINT) ,1)
INC += -Idebug
endif

ifeq ($(MICROPY_PY_LWIP),1)
INC += -I$(TOP)/lib/lwip/src/include
INC += -Ilwip_inc -Ilwip_utils
endif
ifeq ($(MICROPY_SSL_MBEDTLS),1)
INC += -I$(TOP)/lib/mbedtls/include
endif
ifeq ($(MICROPY_SSL_AXTLS),1)
INC += -I$(TOP)/lib/axtls/ssl -I$(TOP)/lib/axtls/crypto -I$(TOP)/extmod/axtls-include
endif

ifeq ($(TARGET), TARGET_RZ_A2XX)
INC_MBED += -ITARGET_RENESAS
INC_MBED += -ITARGET_RENESAS/$(TARGET)
INC_MBED += -ITARGET_RENESAS/$(TARGET)/common
INC_MBED += -ITARGET_RENESAS/$(TARGET)/common/r_cache
INC_MBED += -ITARGET_RENESAS/$(TARGET)/common/r_cache/inc
INC_MBED += -ITARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)
INC_MBED += -ITARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device
INC_MBED += -ITARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/inc
INC_MBED += -ITARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/inc/iodefine
INC_MBED += -ITARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/inc/iodefine/iobitmasks
INC_MBED += -ITARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/inc/iodefine/iodefines

INC += -ITARGET_RENESAS
INC += -ITARGET_RENESAS/$(TARGET)
INC += -ITARGET_RENESAS/$(TARGET)/common
INC += -ITARGET_RENESAS/$(TARGET)/common/r_cache
INC += -ITARGET_RENESAS/$(TARGET)/common/r_cache/inc
INC += -ITARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)
INC += -ITARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device
INC += -ITARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/inc
INC += -ITARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/inc/iodefine
INC += -ITARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/inc/iodefine/iobitmasks
INC += -ITARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/inc/iodefine/iodefines
INC += -Ilib/mbed-os
INC += -Ilib/mbed-os/hal
INC += -Ilib/mbed-os/drivers
INC += -Ilib/mbed-os/platform
INC += -Ilib/mbed-os/cmsis/TARGET_CORTEX_A
INC += -Ilib/mbed-gr-libs/EasyAttach_CameraAndLCD
INC += -Ilib/mbed-gr-libs/GR-PEACH_video

INC += -Ilib/lv_bindings/lvgl

ifeq ($(MBED_OS_EMAC),1)
INC += emac-drivers/TARGET_RZ_A2_EMAC
INC += emac-drivers/TARGET_RZ_A2_EMAC/r_ether_rza2/
INC += emac-drivers/TARGET_RZ_A2_EMAC/r_ether_rza2/src
INC += emac-drivers/TARGET_RZ_A2_EMAC/r_ether_rza2/src/phy
INC += emac-drivers/TARGET_RZ_A2_EMAC/r_ether_rza2/src/targets/TARGET_GR_MANGO
endif

endif

#######################################################################
# C flags.
#######################################################################

LIB_MBED = libmbed.a
LD = gcc
#-Wstack-usage=$(STACK_USAGE)

ifeq ($(MBED_UART_WRAPPER),1)
CFLAGS_COMMON += -DMBED_UART_WRAPPER
endif

ifeq ($(MBED_SPI_WRAPPER),1)
CFLAGS_COMMON += -DMBED_SPI_WRAPPER
endif

ifeq ($(LVGL_ENABLE),1)
CFLAGS_COMMON += -DLVGL_ENABLE=1
CFLAGS_COMMON += -DLV_CONF_INCLUDE_SIMPLE
endif

CFLAGS_MBED_COMMON += $(INC_MBED) -DAPPLICATION_ADDR=0x50000000 -DAPPLICATION_SIZE=0x1000000 -DMBED_ROM_START=0x50000000 -DMBED_ROM_SIZE=0x1000000 -DMBED_RAM_START=0x400000 -DMBED_RAM_SIZE=0x2000000 -DARM_MATH_CA9 -D__FPU_PRESENT -DDEVICE_EMAC=1 -D__MBED__=1 -DDEVICE_USBDEVICE=1 -DTARGET_LIKE_MBED -DDEVICE_PORTINOUT=1 -DMBED_BUILD_TIMESTAMP=1582947353.33 -DCOMPONENT_FLASHIAP=1 -DCOMPONENT_PSA_SRV_EMUL=1 -DDEVICE_SERIAL_ASYNCH=1 -D__CMSIS_RTOS -D__EVAL -DDEVICE_SPISLAVE=1 -DTOOLCHAIN_GCC -DTARGET_CORTEX_A -DDEVICE_I2C_ASYNCH=1 -DTARGET_DEBUG -DDEVICE_RTC=1 -D__MBED_CMSIS_RTOS_CA9 -DCOMPONENT_PSA_SRV_IMPL=1 -DTARGET_LIKE_CORTEX_A9 -DDEVICE_PWMOUT=1 -DDEVICE_SPI_ASYNCH=1 -DDEVICE_INTERRUPTIN=1 -DTARGET_CORTEX -DDEVICE_I2C=1 -DDEVICE_PORTOUT=1 -DDEVICE_I2CSLAVE=1 -DDEVICE_USTICKER=1 -DDEVICE_STDIO_MESSAGES=1 -DTARGET_MBRZA2M -DTARGET_RENESAS -DTARGET_RZA2M -DTARGET_NAME=GR_MANGO -DCOMPONENT_NSPE=1 -DDEVICE_SERIAL_FC=1 -DTARGET_A9 -DTARGET_RZ_A2XX -D__CORTEX_A9 -DDEVICE_PORTIN=1 -DDEVICE_SLEEP=1 -DTOOLCHAIN_GCC_ARM -DDEVICE_SPI=1 -DTARGET_GR_MANGO -DDEVICE_ANALOGIN=1 -DDEVICE_SERIAL=1 -DDEVICE_FLASH=1 -DMBED_CONF_APP_MAIN_STACK_SIZE=8192 -DTARGET_RZ_A2_EMAC
#CFLAGS_MBED_COMMON += -DMBEDTLS_NO_DEFAULT_ENTROPY_SOURCES -DMBEDTLS_USER_CONFIG_FILE="./mbedtls_entropy_config.h" -DMBEDTLS_TEST_NULL_ENTROPY 
CFLAGS_MBED_COMMON += -DMBED_CONF_PLATFORM_CTHUNK_COUNT_MAX=1
CFLAGS_MBED_COMMON += -DMBED_CONF_PLATFORM_DEFAULT_SERIAL_BAUD_RATE=115200
CFLAGS_MBED_COMMON += -DMBED_CONF_PLATFORM_STDIO_BAUD_RATE=115200
CFLAGS_MBED_COMMON += -DMBED_CONF_RTOS_THREAD_STACK_SIZE=4096
#CFLAGS_COMMON += -include lib/mbed-os/mbed_config.h
#ifeq ($(MBED_OS_USE), 1)
CFLAGS_MBED_COMMON += @./mbed-os-include.txt
#endif
ifeq ($(OVERRIDE_CONSOLE_USBSERIAL), 1)
CFLAGS_MBED_COMMON += -DOVERRIDE_CONSOLE_USBSERIAL
endif
ifeq ($(MBED_CONSOLE), 1)
CFLAGS_MBED_COMMON += -DMBED_CONSOLE
endif

ifeq ($(MBED_GR_LIBS), 1)
CFLAGS_MBED_COMMON += @./mbed-gr-libs-include.txt
CFLAGS_MBED_COMMON += -DLCD_SIZE=SD_7INCH
CFLAGS_MBED_COMMON += -DMBED_CONF_APP_LCD=1
CFLAGS_MBED_COMMON += -DMBED_CONF_APP_CAMERA=1
CFLAGS_MBED_COMMON += -DCAMERA_MODULE=MODULE_VDC
CFLAGS_MBED_COMMON += -DMBED_CONF_APP_CAMERA_TYPE=CAMERA_OV7725
CFLAGS_MBED_COMMON += -DMBED_CONF_APP_LCD_TYPE=RGB_TO_HDMI
CFLAGS_MBED_COMMON += -DMBED_GR_LIBS=1
CFLAGS_MBED_COMMON += -DMBED_GR_LIBS_DRP_FOR_MBED=1
CFLAGS_MBED_COMMON += -DMBED_GR_LIBS_components_LCD=1
CFLAGS_MBED_COMMON += -DMBED_GR_LIBS_components_CAMERA=1
CFLAGS_MBED_COMMON += -DMBED_GR_LIBS_EasyAttach_CameraAndLCD=1
endif

ifeq ($(MBED_OS_EMAC),1)
CFLAGS_MBED_COMMON += -DMBED_CONF_EVENTS_SHARED_EVENTSIZE=10
endif

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
ifeq ($(BOARD),GR_MANGO_DD)
CFLAGS += -DGRMANGO
endif
CFLAGS += -DFFCONF_H=\"lib/oofatfs/ffconf.h\"
CFLAGS += -DMICROPY_VFS_FAT=1

ifeq ($(USE_DBG_PRINT), 1)
CFLAGS += -DUSE_DBG_PRINT
#CFLAGS += -DMICROPY_DEBUG_PRINTERS=debug_printf
endif

ifeq ($(MICROPY_DEBUG_VERBOSE), 1)
CFLAGS += -DMICROPY_DEBUG_VERBOSE=1
endif

ifeq ($(MICROPY_DEBUG_PRINTERS), 1)
CFLAGS += -DMICROPY_DEBUG_PRINTERS=1
endif

CSUPEROPT = -Os # save some code space

# Tune for Debugging or Optimization
ifeq ($(DEBUG), 1)
CFLAGS += -O0 -ggdb
else
CFLAGS += -Os -DNDEBUG
CFLAGS += -fdata-sections -ffunction-sections
endif

ifeq ($(MICROPY_PY_LWIP),1)
CFLAGS += -DMICROPY_PY_LWIP=1
endif

ifeq ($(MICROPY_SSL_MBEDTLS),1)
CFLAGS += -DMICROPY_SSL_MBEDTLS=1
endif

ifeq ($(MICROPY_PY_USSL),1)
CFLAGS += -DMICROPY_PY_USSL=1
endif

ifeq ($(MICROPY_SSL_AXTLS),1)
CFLAGS += -DMICROPY_SSL_AXTLS
CFLAGS += -DMICROPY_STREAMS_POSIX_API
endif

ifeq ($(MBED_OS_EMAC),1)
CFLAGS += -DMBED_OS_EMAC
CPPFLAGS += -DMBED_OS_EMAC
endif

#######################################################################
# source files
#######################################################################

SRC_LIB = $(addprefix lib/,\
	libc/string0.c \
	mp-readline/readline.c \
	netutils/netutils.c \
	netutils/trace.c \
	netutils/dhcpserver.c \
	timeutils/timeutils.c \
	utils/pyexec.c \
	utils/interrupt_char.c \
	utils/printf.c \
	utils/sys_stdio_mphal.c \
	utils/mpirq.c \
	)

EXTMOD_SRC_C += $(addprefix extmod/,\
	virtpin.c \
    )

ifeq ($(MICROPY_FLOAT_IMPL),double)
SRC_LIBM = $(addprefix lib/libm_dbl/,\
	__cos.c \
	__expo2.c \
	__fpclassify.c \
	__rem_pio2.c \
	__rem_pio2_large.c \
	__signbit.c \
	__sin.c \
	__tan.c \
	acos.c \
	acosh.c \
	asin.c \
	asinh.c \
	atan.c \
	atan2.c \
	atanh.c \
	ceil.c \
	cos.c \
	cosh.c \
	copysign.c \
	erf.c \
	exp.c \
	expm1.c \
	floor.c \
	fmod.c \
	frexp.c \
	ldexp.c \
	lgamma.c \
	log.c \
	log10.c \
	log1p.c \
	modf.c \
	nearbyint.c \
	pow.c \
	rint.c \
	scalbn.c \
	sin.c \
	sinh.c \
	tan.c \
	tanh.c \
	tgamma.c \
	trunc.c \
	)
ifeq ($(SUPPORTS_HARDWARE_FP_DOUBLE),1)
# ToDo
SRC_LIBM += lib/libm_dbl/thumb_vfp_sqrt.c
else
SRC_LIBM += lib/libm_dbl/sqrt.c
endif
else
SRC_LIBM = $(addprefix lib/libm/,\
	math.c \
	acoshf.c \
	asinfacosf.c \
	asinhf.c \
	atan2f.c \
	atanf.c \
	atanhf.c \
	ef_rem_pio2.c \
	erf_lgamma.c \
	fmodf.c \
	kf_cos.c \
	kf_rem_pio2.c \
	kf_sin.c \
	kf_tan.c \
	log1pf.c \
	nearbyintf.c \
	sf_cos.c \
	sf_erf.c \
	sf_frexp.c \
	sf_ldexp.c \
	sf_modf.c \
	sf_sin.c \
	sf_tan.c \
	wf_lgamma.c \
	wf_tgamma.c \
	)
ifeq ($(SUPPORTS_HARDWARE_FP_SINGLE),1)
SRC_LIBM += lib/libm/thumb_vfp_sqrtf.c
else
SRC_LIBM += lib/libm/ef_sqrt.c
endif
endif

EXTMOD_SRC_C = $(addprefix extmod/,\
	modonewire.c \
	)

DRIVERS_SRC_C = $(addprefix drivers/,\
	bus/softspi.c \
	bus/softqspi.c \
	memory/spiflash.c \
	dht/dht.c \
	)

SRC_C = \
	main.c \
#	$(BUILD)/_frozen_mpy.c \

#######################################################################
# TARGET_RENESAS
#######################################################################
SRC_MBED_S += TARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/TOOLCHAIN_GCC_ARM/startup_RZA2M.S
SRC_MBED_S += TARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/TOOLCHAIN_GCC_ARM/weak_handler.S

ifeq ($(OVERRIDE_CONSOLE_USBSERIAL), 1)
SRC_MBED_CPP += TARGET_RENESAS/$(TARGET)/common/target_override_console.cpp
endif

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
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/PeripheralPins.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/cmsis_nvic.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/mbed_sf_boot.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/mmu_RZ_A2M.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/nvic_wrapper.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/octaram_init.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/os_tick_ostm.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/RZ_A2_Init.c
SRC_MBED_C += TARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/system_RZ_A2M.c

ifeq ($(MBED_OS), 1)
#######################################################################
# mbed-os cmsis
#######################################################################
SRC_MBED_C += lib/mbed-os/cmsis/TARGET_CORTEX_A/irq_ctrl_gic.c
#######################################################################
# mbed-os dirvers
#######################################################################
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
#######################################################################
# mbed-os events
#######################################################################
ifeq ($(MBED_OS_EVENTS), 1)
SRC_MBED_CPP += lib/mbed-os/events/EventQueue.cpp
SRC_MBED_CPP += lib/mbed-os/events/mbed_shared_queues.cpp
SRC_MBED_CPP += lib/mbed-os/events/equeue/equeue_mbed.cpp
SRC_MBED_C += lib/mbed-os/events/equeue/equeue_posix.c
SRC_MBED_C += lib/mbed-os/events/equeue/equeue.c
endif
#######################################################################
# mbed-os features
#######################################################################
ifeq ($(MBED_OS_FEATURES_cellular), 1)
SRC_MBED_CPP += lib/mbed-os/features/cellular/framework/AT/ATHandler.cpp
SRC_MBED_CPP += lib/mbed-os/features/cellular/framework/AT/ATHandler_factory.cpp
SRC_MBED_CPP += lib/mbed-os/features/cellular/framework/AT/AT_CellularBase.cpp
SRC_MBED_CPP += lib/mbed-os/features/cellular/framework/AT/AT_CellularContext.cpp
SRC_MBED_CPP += lib/mbed-os/features/cellular/framework/AT/AT_CellularDevice.cpp
SRC_MBED_CPP += lib/mbed-os/features/cellular/framework/AT/AT_CellularInformation.cpp
SRC_MBED_CPP += lib/mbed-os/features/cellular/framework/AT/AT_CellularNetwork.cpp
SRC_MBED_CPP += lib/mbed-os/features/cellular/framework/AT/AT_CellularSMS.cpp
SRC_MBED_CPP += lib/mbed-os/features/cellular/framework/AT/AT_CellularStack.cpp
SRC_MBED_CPP += lib/mbed-os/features/cellular/framework/AT/AT_ControlPlane_netif.cpp
SRC_MBED_CPP += lib/mbed-os/features/cellular/framework/common/CellularLog.cpp
SRC_MBED_CPP += lib/mbed-os/features/cellular/framework/common/CellularStateMachine.cpp
SRC_MBED_CPP += lib/mbed-os/features/cellular/framework/divice/CellularContext.cpp
SRC_MBED_CPP += lib/mbed-os/features/cellular/framework/divice/CellularDevice.cpp
SRC_MBED_CPP += lib/mbed-os/features/cellular/framework/divice/CellularStateMachine.cpp
SRC_MBED_CPP += lib/mbed-os/features/cellular/framework/targets/GEMALTO/CINTERION/GEMALTO_CINTERION.cpp
SRC_MBED_CPP += lib/mbed-os/features/cellular/framework/targets/GEMALTO/CINTERION/GEMALTO_CINTERION_CellularContext.cpp
SRC_MBED_CPP += lib/mbed-os/features/cellular/framework/targets/GEMALTO/CINTERION/GEMALTO_CINTERION_CellularInformation.cpp
SRC_MBED_CPP += lib/mbed-os/features/cellular/framework/targets/GEMALTO/CINTERION/GEMALTO_CINTERION_CellularStack.cpp
SRC_MBED_CPP += lib/mbed-os/features/cellular/framework/targets/GENERIC/GENERIC_AT3GPP/GENERIC_AT3GPP.cpp
SRC_MBED_CPP += lib/mbed-os/features/cellular/framework/targets/MultiTech/DragonflyNano/PPP/SARA4_PPP.cpp
SRC_MBED_CPP += lib/mbed-os/features/cellular/framework/targets/MultiTech/DragonflyNano/PPP/SARA4_PPP_CellularNetwork.cpp
# ToDo
endif
ifeq ($(MBED_OS_FEATURES_cryptocell), 1)
# ToDo
endif
ifeq ($(MBED_OS_FEATURES_deprecated_warnings), 1)
# ToDo
endif
ifeq ($(MBED_OS_FEATURES_device_key), 1)
# ToDo
endif
ifeq ($(MBED_OS_FEATURES_FEATURE_BLE), 1)
SRC_MBED_CPP += lib/mbed-os/features/FEATURE_BLE/source/BLE.cpp
SRC_MBED_CPP += lib/mbed-os/features/FEATURE_BLE/source/BLEInstanceBase.cpp
SRC_MBED_CPP += lib/mbed-os/features/FEATURE_BLE/source/DiscoveredCharacteristic.cpp
SRC_MBED_CPP += lib/mbed-os/features/FEATURE_BLE/source/GapScanningParams.cpp
SRC_MBED_CPP += lib/mbed-os/features/FEATURE_BLE/source/gap/AdvertisingDataBuilder.cpp
SRC_MBED_CPP += lib/mbed-os/features/FEATURE_BLE/source/gap/AdvertisingParameters.cpp
SRC_MBED_CPP += lib/mbed-os/features/FEATURE_BLE/source/gap/ConnectionParameters.cpp
SRC_MBED_CPP += lib/mbed-os/features/FEATURE_BLE/source/generic/FileSecurityDb.cpp
SRC_MBED_CPP += lib/mbed-os/features/FEATURE_BLE/source/services/DFUService.cpp
SRC_MBED_CPP += lib/mbed-os/features/FEATURE_BLE/source/services/UARTService.cpp
SRC_MBED_CPP += lib/mbed-os/features/FEATURE_BLE/source/services/URIBeaconConfigService.cpp
# ToDo
endif
ifeq ($(MBED_OS_FEATURES_FEATURE_BOOTLOADER), 1)
endif
ifeq ($(MBED_OS_FEATURES_frameworks), 1)
endif
ifeq ($(MBED_OS_FEATURES_lorawan), 1)
endif
ifeq ($(MBED_OS_FEATURES_lwipstack), 1)
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/LWIPInterface.cpp
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/LWIPInterfaceEMAC.cpp
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/LWIPInterfaceL3IP.cpp
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/LWIPMemoryManager.cpp
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/LWIPStack.cpp
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip_tools.cpp
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/ppp_lwip.cpp
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/api/lwip_api_lib.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/api/lwip_api_msg.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/api/lwip_err.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/api/lwip_netbuf.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/api/lwip_netdb.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/api/lwip_netifapi.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/api/lwip_sockets.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/api/lwip_tcpip.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/httpd/lwip_fs.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/httpd/lwip_fsdata.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/httpd/lwip_httpd.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/httpd/makefsdata/lwip_makefsdata.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/lwiperf/lwip_lwiperf.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/mdns/lwip_mdns.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/mqtt/lwip_mqtt.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/netbiosns/lwip_netbiosns.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/snmp/lwip_snmpv3.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/snmp/lwip_snmpv3_dummy.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/snmp/lwip_snmpv3_mbedtls.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/snmp/lwip_snmp_asn1.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/snmp/lwip_snmp_core.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/snmp/lwip_snmp_mib2.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/snmp/lwip_snmp_mib2_icmp.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/snmp/lwip_snmp_mib2_interfaces.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/snmp/lwip_snmp_mib2_ip.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/snmp/lwip_snmp_mib2_snmp.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/snmp/lwip_snmp_mib2_system.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/snmp/lwip_snmp_mib2_tcp.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/snmp/lwip_snmp_mib2_udp.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/snmp/lwip_snmp_msg.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/snmp/lwip_snmp_netconn.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/snmp/lwip_snmp_pbuf_stream.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/snmp/lwip_snmp_raw.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/snmp/lwip_snmp_scalar.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/snmp/lwip_snmp_table.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/snmp/lwip_snmp_threadsync.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/snmp/lwip_snmp_traps.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/sntp/lwip_sntp.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/apps/tftp/lwip_tftp_server.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/lwip_def.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/lwip_dns.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/lwip_inet_chksum.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/lwip_init.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/lwip_ip.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/lwip_mem.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/lwip_memp.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/lwip_netif.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/lwip_pbuf.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/lwip_raw.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/lwip_stats.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/lwip_sys.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/lwip_tcp.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/lwip_tcp_in.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/lwip_tcp_out.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/lwip_timeouts.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/lwip_udp.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/ipv4/lwip_autoip.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/ipv4/lwip_dhcp.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/ipv4/lwip_etharp.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/ipv4/lwip_icmp.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/ipv4/lwip_igmp.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/ipv4/lwip_ip4.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/ipv4/lwip_ip4_addr.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/ipv4/lwip_ip4_frag.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/ipv6/lwip_dhcp6.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/ipv6/lwip_ethip6.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/ipv6/lwip_icmp6.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/ipv6/lwip_inet6.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/ipv6/lwip_ip6.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/ipv6/lwip_ip6_addr.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/ipv6/lwip_ip6_frag.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/ipv6/lwip_mld6.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/core/ipv6/lwip_nd6.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/lwip_ethernet.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/lwip_ethernetif.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/lwip_lowpan6.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/lwip_slipif.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/lwip_auth.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/lwip_ccp.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/lwip_chap-md5.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/lwip_chap-new.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/lwip_chap_ms.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/lwip_demand.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/lwip_eap.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/lwip_ecp.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/lwip_eui64.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/lwip_fsm.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/lwip_ipcp.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/lwip_ipv6cp.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/lwip_lcp.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/lwip_magic.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/lwip_mppe.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/lwip_multilink.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/lwip_ppp.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/lwip_pppapi.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/lwip_pppcrypt.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/lwip_pppoe.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/lwip_pppol2tp.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/lwip_pppos.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/lwip_upap.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/lwip_utils.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/lwip_vj.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/polarssl/lwip_arc4.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/polarssl/lwip_des.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/polarssl/lwip_md4.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/polarssl/lwip_md5.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/src/netif/ppp/polarssl/lwip_sha1.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/test/fuzz/fuzz.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/test/unit/lwip_unittests.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/test/unit/core/test_mem.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/test/unit/core/test_pbuf.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/test/unit/dhcp/test_dhcp.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/test/unit/etharp/test_etharp.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/test/unit/mdns/test_mdns.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/test/unit/tcp/tcp_helper.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/test/unit/tcp/test_tcp.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/test/unit/tcp/test_tcp_oos.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip/test/unit/udp/test_udp.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip-sys/lwip_random.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip-sys/lwip_tcp_isn.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip-sys/arch/lwip_checksum.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip-sys/arch/lwip_memcpy.c
SRC_MBED_CPP += lib/mbed-os/features/lwipstack/lwip-sys/arch/lwip_sys_arch.c
endif
ifeq ($(MBED_OS_FEATURES_mbedtls), 1)
endif
ifeq ($(MBED_OS_FEATURES_nanostack), 1)
endif
ifeq ($(MBED_OS_FEATURES_netsocket), 1)
endif
ifeq ($(MBED_OS_EMAC),1)
SRC_CPP += lib/mbed-os/features/netsocket/emac-drivers/TARGET_RZ_A2_EMAC/rza2_emac.cpp
SRC_C += lib/mbed-os/features/netsocket/emac-drivers/TARGET_RZ_A2_EMAC/r_ether_rza2/src/phy/phy.c
SRC_C += lib/mbed-os/features/netsocket/emac-drivers/TARGET_RZ_A2_EMAC/r_ether_rza2/src/r_ether_rza2.c
SRC_C += lib/mbed-os/features/netsocket/emac-drivers/TARGET_RZ_A2_EMAC/r_ether_rza2/src/targets/TARGET_GR_MANGO/r_ether_setting_rza2m.c
endif
ifeq ($(MBED_OS_FEATURES_nfc), 1)
endif
ifeq ($(MBED_OS_FEATURES_storage), 1)
endif
ifeq ($(MBED_OS_FEATURES_unsupported), 1)
endif
#######################################################################
# mbed-os hal
#######################################################################
SRC_MBED_CPP += lib/mbed-os/hal/LowPowerTickerWrapper.cpp
SRC_MBED_C += lib/mbed-os/hal/mbed_critical_section_api.c
SRC_MBED_C += lib/mbed-os/hal/mbed_flash_api.c
SRC_MBED_C += lib/mbed-os/hal/mbed_gpio.c
SRC_MBED_C += lib/mbed-os/hal/mbed_itm_api.c
SRC_MBED_C += lib/mbed-os/hal/mbed_lp_ticker_api.c
SRC_MBED_CPP += lib/mbed-os/hal/mbed_lp_ticker_wrapper.cpp
SRC_MBED_C += lib/mbed-os/hal/mbed_pinmap_common.c
SRC_MBED_C += lib/mbed-os/hal/mbed_pinmap_default.c
SRC_MBED_C += lib/mbed-os/hal/mbed_ticker_api.c
SRC_MBED_C += lib/mbed-os/hal/mbed_us_ticker_api.c
SRC_MBED_C += lib/mbed-os/hal/mpu/mbed_mpu_v7m.c
SRC_MBED_C += lib/mbed-os/hal/mpu/mbed_mpu_v8m.c
#SRC_MBED_C += lib/mbed-os/hal/TARGET_FLASH_CMSIS_ALGO/flash_common_algo.c
#######################################################################
# mbed-os platform (29 files)
#######################################################################
SRC_MBED_CPP += lib/mbed-os/platform/ATCmdParser.cpp
SRC_MBED_CPP += lib/mbed-os/platform/CallChain.cpp
SRC_MBED_CPP += lib/mbed-os/platform/CThunkBase.cpp
SRC_MBED_CPP += lib/mbed-os/platform/FileBase.cpp
SRC_MBED_CPP += lib/mbed-os/platform/FileHandle.cpp
SRC_MBED_CPP += lib/mbed-os/platform/FilePath.cpp
SRC_MBED_CPP += lib/mbed-os/platform/FileSystemHandle.cpp
SRC_MBED_CPP += lib/mbed-os/platform/LocalFileSystem.cpp
SRC_MBED_CPP += lib/mbed-os/platform/mbed_alloc_wrappers.cpp
SRC_MBED_C += lib/mbed-os/platform/mbed_application.c
SRC_MBED_C += lib/mbed-os/platform/mbed_assert.c
SRC_MBED_C += lib/mbed-os/platform/mbed_board.c
SRC_MBED_C += lib/mbed-os/platform/mbed_critical.c
SRC_MBED_C += lib/mbed-os/platform/mbed_error.c
SRC_MBED_C += lib/mbed-os/platform/mbed_error_hist.c
SRC_MBED_C += lib/mbed-os/platform/mbed_interface.c
SRC_MBED_CPP += lib/mbed-os/platform/mbed_mem_trace.cpp
SRC_MBED_C += lib/mbed-os/platform/mbed_mktime.c
SRC_MBED_C += lib/mbed-os/platform/mbed_mpu_mgmt.c
SRC_MBED_CPP += lib/mbed-os/platform/mbed_poll.cpp
SRC_MBED_CPP += lib/mbed-os/platform/mbed_retarget.cpp
SRC_MBED_CPP += lib/mbed-os/platform/mbed_rtc_time.cpp
SRC_MBED_C += lib/mbed-os/platform/mbed_sdk_boot.c
SRC_MBED_C += lib/mbed-os/platform/mbed_semihost_api.c
SRC_MBED_C += lib/mbed-os/platform/mbed_sleep_manager.c
SRC_MBED_C += lib/mbed-os/platform/mbed_stats.c
SRC_MBED_C += lib/mbed-os/platform/mbed_wait_api_no_rtos.c
SRC_MBED_CPP += lib/mbed-os/platform/mbed_wait_api_rtos.cpp
SRC_MBED_CPP += lib/mbed-os/platform/Stream.cpp
#######################################################################
# mbed-os rtos
#######################################################################
SRC_MBED_CPP += lib/mbed-os/rtos/ConditionVariable.cpp
SRC_MBED_CPP += lib/mbed-os/rtos/EventFlags.cpp
SRC_MBED_CPP += lib/mbed-os/rtos/Kernel.cpp
SRC_MBED_CPP += lib/mbed-os/rtos/Mutex.cpp
SRC_MBED_CPP += lib/mbed-os/rtos/RtosTimer.cpp
SRC_MBED_CPP += lib/mbed-os/rtos/Semaphore.cpp
SRC_MBED_CPP += lib/mbed-os/rtos/ThisThread.cpp
SRC_MBED_CPP += lib/mbed-os/rtos/Thread.cpp
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/mbed_boot.c
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/mbed_rtos_rtx.c
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/mbed_rtx_handlers.c
SRC_MBED_CPP += lib/mbed-os/rtos/TARGET_CORTEX/mbed_rtx_idle.cpp
SRC_MBED_CPP += lib/mbed-os/rtos/TARGET_CORTEX/SysTimer.cpp
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/rtx4/cmsis_os1.c
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Config/RTX_Config.c
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/rtx5/RTX/Config/TARGET_CORTEX_A/handlers.c
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
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/rtx5/Source/os_systick.c
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/rtx5/Source/os_tick_ptim.c
SRC_MBED_C += lib/mbed-os/rtos/TARGET_CORTEX/TOOLCHAIN_GCC_ARM/mbed_boot_gcc_arm.c
#######################################################################
# mbed-os usb
#######################################################################
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
endif

ifeq ($(MBED_GR_LIBS), 1)
ifeq ($(MBED_GR_LIBS_AlarmTimer), 1)
SRC_MBED_CPP += lib/mbed-gr-libs/AlarmTimer/$(TARGET)/AlarmTimer.cpp
endif
ifeq ($(MBED_GR_LIBS_AsciiFont), 1)
SRC_MBED_C += lib/mbed-gr-libs/AsciiFont/ascii.c
SRC_MBED_CPP += lib/mbed-gr-libs/AsciiFont/AsciiFont.cpp
endif
ifeq ($(MBED_GR_LIBS_bd), 1)
SRC_MBED_C += lib/mbed-gr-libs/bd/RomRamBlockDevice/RomRamBlockDevice.cpp
endif
ifeq ($(MBED_GR_LIBS_components_AUDIO), 1)
endif
ifeq ($(MBED_GR_LIBS_components_CAMERA), 1)
endif
ifeq ($(MBED_GR_LIBS_components_LCD), 1)
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
endif
ifeq ($(MBED_GR_LIBS_components_WIFI), 1)
SRC_MBED_CPP += lib/mbed-gr-libs/components/WIFI/esp32-driver/ESP32Interface.cpp
SRC_MBED_CPP += lib/mbed-gr-libs/components/WIFI/esp32-driver/ESP32InterfaceAP.cpp
SRC_MBED_CPP += lib/mbed-gr-libs/components/WIFI/esp32-driver/ESP32Stack.cpp
SRC_MBED_CPP += lib/mbed-gr-libs/components/WIFI/esp32-driver/ESP32/ESP32.cpp
endif
ifeq ($(MBED_GR_LIBS_dcache-control), 1)
SRC_MBED_C += lib/mbed-gr-libs/dcache-control/dcache-control.c
endif
ifeq ($(MBED_GR_LIBS_DeepStandby), 1)
SRC_MBED_CPP += lib/mbed-gr-libs/DeepStandby/DeepStandby.cpp
endif
ifeq ($(MBED_GR_LIBS_DisplayApp), 1)
SRC_MBED_CPP += lib/mbed-gr-libs/DisplayApp/DisplayApp.cpp
endif
ifeq ($(MBED_GR_LIBS_DRP_FOR_MBED), 1)
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/src/r_dk2_core.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/src/r_dk2_if.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_affine/r_drp_affine.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_argb2grayscale/r_drp_argb2grayscale.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_bayer2grayscale/r_drp_bayer2grayscale.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_bayer2rgb/r_drp_bayer2rgb.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_bayer2rgb_color_correction/r_drp_bayer2rgb_color_correction.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_binarization_adaptive/r_drp_binarization_adaptive.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_binarization_adaptive_bit/r_drp_binarization_adaptive_bit.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_binarization_fixed/r_drp_binarization_fixed.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_canny_calculate/r_drp_canny_calculate.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_canny_hysterisis/r_drp_canny_hysterisis.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_circle_fitting/r_drp_circle_fitting.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_corner_harris/r_drp_corner_harris.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_cropping/r_drp_cropping.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_cropping_rgb/r_drp_cropping_rgb.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_dilate/r_drp_dilate.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_erode/r_drp_erode.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_find_contours/r_drp_find_contours.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_gamma_correction/r_drp_gamma_correction.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_gaussian_blur/r_drp_gaussian_blur.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_histogram/r_drp_histogram.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_histogram_normalization/r_drp_histogram_normalization.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_histogram_normalization_rgb/r_drp_histogram_normalization_rgb.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_image_rotate/r_drp_image_rotate.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_laplacian/r_drp_laplacian.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_median_blur/r_drp_median_blur.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_minutiae_delete/r_drp_minutiae_delete.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_minutiae_extract/r_drp_minutiae_extract.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_prewitt/r_drp_prewitt.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_reed_solomon/r_drp_reed_solomon.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_reed_solomon_gf8/r_drp_reed_solomon_gf8.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_resize_bilinear/r_drp_resize_bilinear.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_resize_bilinear_fixed/r_drp_resize_bilinear_fixed.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_resize_bilinear_fixed_rgb/r_drp_resize_bilinear_fixed_rgb.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_resize_nearest/r_drp_resize_nearest.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_simple_isp/r_drp_simple_isp_bayer2grayscale_3.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_simple_isp/r_drp_simple_isp_bayer2grayscale_6.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_simple_isp/r_drp_simple_isp_bayer2yuv_3.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_simple_isp/r_drp_simple_isp_bayer2yuv_6.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_sobel/r_drp_sobel.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_thinning/r_drp_thinning.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_unsharp_masking/r_drp_unsharp_masking.c
endif
ifeq ($(MBED_GR_LIBS_EasyAttach_CameraAndLCD), 1)
SRC_MBED_CPP += lib/mbed-gr-libs/EasyAttach_CameraAndLCD/EasyAttach_CameraAndLCD.cpp
endif
ifeq ($(MBED_GR_LIBS_EasyPlayback), 1)
SRC_MBED_CPP += lib/mbed-gr-libs/EasyPlayback/EasyPlayback.cpp
SRC_MBED_CPP += lib/mbed-gr-libs/EasyPlayback/decoder/EasyDec_Mov.cpp
SRC_MBED_CPP += lib/mbed-gr-libs/EasyPlayback/decoder/EasyDec_WavCnv2ch.cpp
endif
ifeq ($(MBED_GR_LIBS_GR-PEACH_video), 1)
SRC_MBED_CPP += lib/mbed-gr-libs/GR-PEACH_video/DisplayBase.cpp
SRC_MBED_C += lib/mbed-gr-libs/GR-PEACH_video/targets/$(TARGET)/drivers/ceu/src/r_ceu_driver.c
SRC_MBED_C += lib/mbed-gr-libs/GR-PEACH_video/targets/$(TARGET)/drivers/ceu/userdef/ceu_userdef.c
SRC_MBED_C += lib/mbed-gr-libs/GR-PEACH_video/targets/$(TARGET)/drivers/r_mipi/src/r_mipi_api.c
SRC_MBED_C += lib/mbed-gr-libs/GR-PEACH_video/targets/$(TARGET)/drivers/r_mipi/userdef/r_mipi_userdef_api.c
SRC_MBED_C += lib/mbed-gr-libs/GR-PEACH_video/targets/$(TARGET)/TARGET_RZA2M/rz_a2m_evb_vdc.c
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
SRC_MBED_C += lib/mbed-gr-libs/GR-PEACH_video/targets/$(TARGET)/TARGET_RZA2M/lvds/lvds_pll_data.c
SRC_MBED_C += lib/mbed-gr-libs/GR-PEACH_video/targets/$(TARGET)/TARGET_RZA2M/lvds/lvds_pll_main.c
endif
ifeq ($(MBED_GR_LIBS_GraphicsFramework), 1)
SRC_MBED_C += lib/mbed-gr-libs/GraphicsFramework/jcu/jcu_driver/converter_wrapper.c
SRC_MBED_C += lib/mbed-gr-libs/GraphicsFramework/jcu/jcu_driver/jcu_api.c
SRC_MBED_C += lib/mbed-gr-libs/GraphicsFramework/jcu/jcu_driver/jcu_para.c
SRC_MBED_CPP += lib/mbed-gr-libs/GraphicsFramework/jcu/jcu_driver/JPEG_Coverter.cpp
SRC_MBED_C += lib/mbed-gr-libs/GraphicsFramework/jcu/jcu_driver/$(TARGET)/jcu_reg.c
SRC_MBED_C += lib/mbed-gr-libs/GraphicsFramework/jcu/porting/jcu_pl.c
SRC_MBED_C += lib/mbed-gr-libs/GraphicsFramework/jcu/userdef/jcu_user.c
SRC_MBED_C += lib/mbed-gr-libs/GraphicsFramework/ospl/porting/DebugBreak.c
SRC_MBED_C += lib/mbed-gr-libs/GraphicsFramework/ospl/porting/inline_body.c
SRC_MBED_C += lib/mbed-gr-libs/GraphicsFramework/ospl/porting/locking.c
SRC_MBED_C += lib/mbed-gr-libs/GraphicsFramework/ospl/porting/mcu_interrupts.c
SRC_MBED_C += lib/mbed-gr-libs/GraphicsFramework/ospl/porting/r_ospl_debug.c
SRC_MBED_C += lib/mbed-gr-libs/GraphicsFramework/ospl/porting/r_ospl_memory.c
SRC_MBED_C += lib/mbed-gr-libs/GraphicsFramework/ospl/porting/r_ospl_RTX.c
SRC_MBED_C += lib/mbed-gr-libs/GraphicsFramework/ospl/porting/r_ospl_unrecoverable.c
SRC_MBED_C += lib/mbed-gr-libs/GraphicsFramework/ospl/porting/TOOLCHAIN_GCC/r_ospl_os_less_asm.s
SRC_MBED_C += lib/mbed-gr-libs/GraphicsFramework/ospl/porting/src/r_ospl.c
endif
ifeq ($(MBED_GR_LIBS_R_BSP), 1)
SRC_MBED_C += lib/mbed-gr-libs/R_BSP/common/R_BSP_Aio.cpp
SRC_MBED_C += lib/mbed-gr-libs/R_BSP/common/R_BSP_Scux.cpp
SRC_MBED_C += lib/mbed-gr-libs/R_BSP/common/R_BSP_SerialFamily.cpp
SRC_MBED_C += lib/mbed-gr-libs/R_BSP/common/R_BSP_Spdif.cpp
SRC_MBED_C += lib/mbed-gr-libs/R_BSP/common/R_BSP_Ssif.cpp
SRC_MBED_C += lib/mbed-gr-libs/R_BSP/RenesasBSP/drv_src/ioif/aioif.c
SRC_MBED_C += lib/mbed-gr-libs/R_BSP/RenesasBSP/$(TARGET)/dma/dma.c
SRC_MBED_C += lib/mbed-gr-libs/R_BSP/RenesasBSP/$(TARGET)/dma/dma_if.c
SRC_MBED_C += lib/mbed-gr-libs/R_BSP/RenesasBSP/$(TARGET)/dma/dma_ver.c
SRC_MBED_C += lib/mbed-gr-libs/R_BSP/RenesasBSP/$(TARGET)/spdif/spdif.c
SRC_MBED_C += lib/mbed-gr-libs/R_BSP/RenesasBSP/$(TARGET)/spdif/spdif_cfg.c
SRC_MBED_C += lib/mbed-gr-libs/R_BSP/RenesasBSP/$(TARGET)/spdif/spdif_dma.c
SRC_MBED_C += lib/mbed-gr-libs/R_BSP/RenesasBSP/$(TARGET)/spdif/spdif_if.c
SRC_MBED_C += lib/mbed-gr-libs/R_BSP/RenesasBSP/$(TARGET)/spdif/spdif_ver.c
SRC_MBED_C += lib/mbed-gr-libs/R_BSP/RenesasBSP/$(TARGET)/ssif/ssif.c
SRC_MBED_C += lib/mbed-gr-libs/R_BSP/RenesasBSP/$(TARGET)/ssif/ssif_cfg.c
SRC_MBED_C += lib/mbed-gr-libs/R_BSP/RenesasBSP/$(TARGET)/ssif/ssif_dma.c
SRC_MBED_C += lib/mbed-gr-libs/R_BSP/RenesasBSP/$(TARGET)/ssif/ssif_if.c
SRC_MBED_C += lib/mbed-gr-libs/R_BSP/RenesasBSP/$(TARGET)/ssif/ssif_int.c
SRC_MBED_C += lib/mbed-gr-libs/R_BSP/RenesasBSP/$(TARGET)/ssif/ssif_ver.c
SRC_MBED_C += lib/mbed-gr-libs/R_BSP/tools/bsp_util.c
SRC_MBED_C += lib/mbed-gr-libs/R_BSP/tools/r_bsp_cmn.c
SRC_MBED_C += lib/mbed-gr-libs/R_BSP/tools/spdif_api.c
SRC_MBED_C += lib/mbed-gr-libs/R_BSP/tools/ssif_api.c
endif
ifeq ($(MBED_GR_LIBS_SDBlockDevice_GRBoard), 1)
SRC_MBED_C += lib/mbed-gr-libs/SDBlockDevice_GRBoard/$(TARGET)/sdhi-driver/SDHSBlockDevice.cpp
SRC_MBED_C += lib/mbed-gr-libs/SDBlockDevice_GRBoard/$(TARGET)/sdhi-driver/r_sdhi_simplified/src/sd/access/sd_cd.c
SRC_MBED_C += lib/mbed-gr-libs/SDBlockDevice_GRBoard/$(TARGET)/sdhi-driver/r_sdhi_simplified/src/sd/access/sd_cmd.c
SRC_MBED_C += lib/mbed-gr-libs/SDBlockDevice_GRBoard/$(TARGET)/sdhi-driver/r_sdhi_simplified/src/sd/access/sd_init.c
SRC_MBED_C += lib/mbed-gr-libs/SDBlockDevice_GRBoard/$(TARGET)/sdhi-driver/r_sdhi_simplified/src/sd/access/sd_int.c
SRC_MBED_C += lib/mbed-gr-libs/SDBlockDevice_GRBoard/$(TARGET)/sdhi-driver/r_sdhi_simplified/src/sd/access/sd_mount.c
SRC_MBED_C += lib/mbed-gr-libs/SDBlockDevice_GRBoard/$(TARGET)/sdhi-driver/r_sdhi_simplified/src/sd/access/sd_read.c
SRC_MBED_C += lib/mbed-gr-libs/SDBlockDevice_GRBoard/$(TARGET)/sdhi-driver/r_sdhi_simplified/src/sd/access/sd_trns.c
SRC_MBED_C += lib/mbed-gr-libs/SDBlockDevice_GRBoard/$(TARGET)/sdhi-driver/r_sdhi_simplified/src/sd/access/sd_util.c
SRC_MBED_C += lib/mbed-gr-libs/SDBlockDevice_GRBoard/$(TARGET)/sdhi-driver/r_sdhi_simplified/src/sd/access/sd_write.c
SRC_MBED_C += lib/mbed-gr-libs/SDBlockDevice_GRBoard/$(TARGET)/sdhi-driver/r_sdhi_simplified/userdef/sd_dev_dmacdrv.c
SRC_MBED_C += lib/mbed-gr-libs/SDBlockDevice_GRBoard/$(TARGET)/sdhi-driver/r_sdhi_simplified/userdef/sd_dev_low.c
endif
ifeq ($(MBED_GR_LIBS_USBHost_custom), 1)
SRC_MBED_CPP += lib/mbed-gr-libs/USBHost_custom/USBHost/USBDeviceConnected.cpp
SRC_MBED_CPP += lib/mbed-gr-libs/USBHost_custom/USBHost/USBEndpoint.cpp
#SRC_MBED_CPP += lib/mbed-gr-libs/USBHost_custom/USBHost/USBHALHost_LPC17.cpp
#SRC_MBED_CPP += lib/mbed-gr-libs/USBHost_custom/USBHost/USBHALHost_RZ_A1.cpp
SRC_MBED_CPP += lib/mbed-gr-libs/USBHost_custom/USBHost/USBHALHost_RZ_A2.cpp
SRC_MBED_CPP += lib/mbed-gr-libs/USBHost_custom/USBHost/USBHost.cpp
SRC_MBED_CPP += lib/mbed-gr-libs/USBHost_custom/USBHost/TARGET_RENESAS/TARGET_RZ_A2XX/NonCacheMem.c
SRC_MBED_C += lib/mbed-gr-libs/USBHost_custom/USBHost3GModule/WANDongle.cpp
SRC_MBED_CPP += lib/mbed-gr-libs/USBHost_custom/USBHost3GModule/WANDongleSerialPort.cpp
SRC_MBED_CPP += lib/mbed-gr-libs/USBHost_custom/USBHostHID/USBHostKeyboard.cpp
SRC_MBED_CPP += lib/mbed-gr-libs/USBHost_custom/USBHostHID/USBHostMouse.cpp
SRC_MBED_CPP += lib/mbed-gr-libs/USBHost_custom/USBHostHub/USBHostHub.cpp
SRC_MBED_CPP += lib/mbed-gr-libs/USBHost_custom/USBHostMIDI/USBHostMIDI.cpp
SRC_MBED_CPP += lib/mbed-gr-libs/USBHost_custom/USBHostMSD/USBHostMSD.cpp
SRC_MBED_CPP += lib/mbed-gr-libs/USBHost_custom/USBHostSerial/USBHostSerial.cpp
endif
endif

ifeq ($(MBED_HTTP), 1)
endif

#######################################################################
# micropython rz CPU
#######################################################################
ifeq ($(BOOT_LOADER),1)
SRC_C += boards/$(BOARD)/$(BOARD)_bl.c
endif
SRC_C += adc.c
SRC_C += bufhelper.c
ifeq ($(LAN_NEW),1)
SRC_C += eth.c
else
SRC_C += ethernetif.c
endif
SRC_C += extint.c
SRC_C += factoryreset.c
SRC_C += fatfs_port.c
SRC_C += flash.c
SRC_C += flashbdev.c
SRC_C += gccollect.c
SRC_S += gchelper_a9.S
SRC_C += help.c
SRC_C += i2c.c
SRC_C += irq.c
SRC_C += led.c
SRC_C += machine_uart.c
SRC_C += machine_timer.c
SRC_C += modmachine.c
SRC_C += modnetwork.c
SRC_C += modnwrxeth.c
SRC_C += modpyb.c
SRC_C += moduos.c
SRC_C += modusocket.c
SRC_C += modutime.c
SRC_C += mphalport.c
ifeq ($(LAN_NEW),1)
SRC_C += network_lan.c
else
SRC_C += modnwrxeth.c
endif
SRC_C += pendsv.c
ifeq ($(MBED_OS_EMAC),1)
else
SRC_C += phy.c
endif
SRC_C += pin.c
SRC_C += pin_defs_rz.c
SRC_C += pin_named_pins.c
SRC_C += rng.c
SRC_C += rtc.c
SRC_C += softtimer.c
SRC_C += spi.c
SRC_C += storage.c
SRC_C += systick.c
#SRC_C += timer.c
SRC_C += uart.c
SRC_C += usrsw.c
SRC_C += $(MCU_SERIES_LOW)/bootstrap.c
SRC_C += $(MCU_SERIES_LOW)/rza2m_adc.c
SRC_C += $(MCU_SERIES_LOW)/rza2m_exti.c
SRC_C += $(MCU_SERIES_LOW)/rza2m_gpio.c
ifeq ($(MBED_OS_EMAC),1)
SRC_CPP += mbed/mbed_ether.cpp
else
SRC_C += $(MCU_SERIES_LOW)/rza2m_ether.c
endif
SRC_C += $(MCU_SERIES_LOW)/rza2m_init.c
SRC_C += $(MCU_SERIES_LOW)/rza2m_ostm2.c
SRC_C += $(MCU_SERIES_LOW)/rza2m_rtc.c
ifeq ($(MBED_UART_WRAPPER),1)
SRC_CPP += mbed/mbed_uart.cpp
else
SRC_C += $(MCU_SERIES_LOW)/rza2m_sci.c
endif
ifeq ($(MBED_SPI_WRAPPER),1)
SRC_CPP += mbed/mbed_spi.cpp
else
SRC_C += $(MCU_SERIES_LOW)/rza2m_spi.c
endif
SRC_C += $(MCU_SERIES_LOW)/rza2m_utils.c
SRC_CPP += mbed/mbed_flash.cpp
SRC_CPP += mbed/mbed_timer.cpp
SRC_CPP += mbed/mbed_uart.cpp
ifeq ($(MBED_GR_LIBS), 1)
ifeq ($(MBED_GR_LIBS_components_CAMERA), 1)
EXTMOD_SRC_C += camera_lcd.c
SRC_CPP += mbed/mbed_camera_lcd.cpp
endif
endif
ifeq ($(LVGL_ENABLE) ,1)
SRC_C += lvgl/modlvrz.c
endif

ifeq ($(USE_DBG_PRINT) ,1)
SRC_C += debug/debug_printf.c
SRC_C += debug/vsnprintf.c
endif

ifeq ($(MICROPY_SSL_MBEDTLS),1)
#CFLAGS += -DMBEDTLS_CONFIG_FILE='"mbedtls/rx_config.h"'
CFLAGS += -DMBEDTLS_CONFIG_FILE='"mbedtls/mbedtls_config.h"'
SRC_MOD += mbedtls/mbedtls_port.c
SRC_MOD += $(addprefix lib/,\
	mbedtls/library/aes.c \
	mbedtls/library/aesni.c \
	mbedtls/library/arc4.c \
	mbedtls/library/aria.c \
	mbedtls/library/asn1parse.c \
	mbedtls/library/asn1write.c \
	mbedtls/library/base64.c \
	mbedtls/library/bignum.c \
	mbedtls/library/blowfish.c \
	mbedtls/library/camellia.c \
	mbedtls/library/ccm.c \
	mbedtls/library/certs.c \
	mbedtls/library/chacha20.c \
	mbedtls/library/chachapoly.c \
	mbedtls/library/cipher_wrap.c \
	mbedtls/library/cipher.c \
	mbedtls/library/cmac.c \
	mbedtls/library/ctr_drbg.c \
	mbedtls/library/debug.c \
	mbedtls/library/des.c \
	mbedtls/library/dhm.c \
	mbedtls/library/ecdh.c \
	mbedtls/library/ecdsa.c \
	mbedtls/library/ecjpake.c \
	mbedtls/library/ecp.c \
	mbedtls/library/ecp_curves.c \
	mbedtls/library/entropy.c \
	mbedtls/library/entropy_poll.c \
	mbedtls/library/error.c \
	mbedtls/library/gcm.c \
	mbedtls/library/havege.c \
	mbedtls/library/hmac_drbg.c \
	mbedtls/library/md.c \
	mbedtls/library/md_wrap.c \
	mbedtls/library/md2.c \
	mbedtls/library/md4.c \
	mbedtls/library/md5.c \
	mbedtls/library/memory_buffer_alloc.c \
	mbedtls/library/oid.c \
	mbedtls/library/padlock.c \
	mbedtls/library/pem.c \
	mbedtls/library/pk.c \
	mbedtls/library/pk_wrap.c \
	mbedtls/library/pkcs11.c \
	mbedtls/library/pkcs12.c \
	mbedtls/library/pkcs5.c \
	mbedtls/library/pkparse.c \
	mbedtls/library/pkwrite.c \
	mbedtls/library/platform.c \
	mbedtls/library/platform_util.c \
	mbedtls/library/poly1305.c \
	mbedtls/library/ripemd160.c \
	mbedtls/library/rsa.c \
	mbedtls/library/rsa_internal.c \
	mbedtls/library/sha1.c \
	mbedtls/library/sha256.c \
	mbedtls/library/sha512.c \
	mbedtls/library/ssl_cache.c \
	mbedtls/library/ssl_ciphersuites.c \
	mbedtls/library/ssl_cli.c \
	mbedtls/library/ssl_cookie.c \
	mbedtls/library/ssl_srv.c \
	mbedtls/library/ssl_ticket.c \
	mbedtls/library/ssl_tls.c \
	mbedtls/library/threading.c \
	mbedtls/library/timing.c \
	mbedtls/library/version.c \
	mbedtls/library/version_features.c \
	mbedtls/library/x509.c \
	mbedtls/library/x509_create.c \
	mbedtls/library/x509_crl.c \
	mbedtls/library/x509_crt.c \
	mbedtls/library/x509_csr.c \
	mbedtls/library/x509write_crt.c \
	mbedtls/library/x509write_csr.c \
)
endif

#######################################################################
# object files
#######################################################################

OBJ =
OBJ += $(PY_O)
OBJ += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_S:.S=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_CPP:.cpp=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_LIB:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_LIBM:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(EXTMOD_SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(DRIVERS_SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_MOD:.c=.o))
OBJ += $(BUILD)/pins_$(BOARD).o

OBJ_MBED_C += $(addprefix $(BUILD)/, $(SRC_MBED_C:.c=.o))
OBJ_MBED_S += $(addprefix $(BUILD)/, $(SRC_MBED_S:.S=.o))
OBJ_MBED_CPP += $(addprefix $(BUILD)/, $(SRC_MBED_CPP:.cpp=.o))
OBJ_MBED += $(OBJ_MBED_C) 
OBJ_MBED += $(OBJ_MBED_S) 
OBJ_MBED += $(OBJ_MBED_CPP) 

# This file contains performance critical functions so turn up the optimisation
# level.  It doesn't add much to the code size and improves performance a bit.
# Don't use -O3 with this file because gcc tries to optimise memset in terms of itself.
$(BUILD)/lib/libc/string0.o: COPT += -O2

# We put several files into the first 16K section with the ISRs.
# If we compile these using -O0 then it won't fit. So if you really want these
# to be compiled with -O0, then edit boards/common.ld (in the .isr_vector section)
# and comment out the following lines.
$(BUILD)/$(OOFATFS_DIR)/ff.o: COPT += -Os
$(filter $(PY_BUILD)/../extmod/vfs_fat_%.o, $(PY_O)): COPT += -Os
$(PY_BUILD)/formatfloat.o: COPT += -Os
$(PY_BUILD)/parsenum.o: COPT += -Os
$(PY_BUILD)/mpprint.o: COPT += -Os

all: $(BUILD)/firmware.bin

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
	$(Q)$(SIZE) $@

ifeq ($(BOOT_LOADER),1)
$(BUILD)/firmware.bin: $(BUILD)/firmware.elf
	$(Q)$(OBJCOPY) -O binary -j .boot -j .isr_vector -j .text -j .ARM.extab -j .ARM.exidx -j .copy.table -j .zero.table -j .ram_code -j .data -j .nc_data -j .octa_data $^ $(BUILD)/firmware.bin
	$(Q)$(OBJDUMP) -S -D $(BUILD)/firmware.elf > $(BUILD)/firmware.s	
else
$(BUILD)/firmware.bin: $(BUILD)/firmware.elf
	$(Q)$(OBJCOPY) -O binary -j .isr_vector -j .text -j .ARM.extab -j .ARM.exidx -j .copy.table -j .zero.table -j .ram_code -j .data -j .nc_data -j .octa_data $^ $(BUILD)/firmware.bin
	$(Q)$(OBJDUMP) -S -D $(BUILD)/firmware.elf > $(BUILD)/firmware.s	
endif

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
