# Select the board to build for: if not given on the command line,
# then default to GR_MANGO
BOARD ?= GR_MANGO

CROSS ?= 1

# enable debug_printf
USE_DBG_PRINT = 0

# enable micropython debug flag
#MICROPY_DEBUG_VERBOSE = 1
#MICROPY_DEBUG_PRINTERS = 1

# If the build directory is not given, make it reflect the board name.
BUILD ?= build-$(BOARD)

BOARD_DIR ?= boards/$(BOARD)
ifeq ($(wildcard $(BOARD_DIR)/.),)
$(error Invalid BOARD specified: $(BOARD_DIR))
endif

#define MBED_CONF_APP_LCD_TYPE    GR_PEACH_4_3INCH_SHIELD
#define MBED_CONF_APP_LCD_TYPE    GR_PEACH_7_1INCH_SHIELD
#define MBED_CONF_APP_LCD_TYPE    GR_PEACH_DISPLAY_SHIELD
#define MBED_CONF_APP_LCD_TYPE    GR_PEACH_RSK_SHIELD
#define MBED_CONF_APP_LCD_TYPE    TF043HV001A0
#define MBED_CONF_APP_LCD_TYPE    ATM0430D25
#define MBED_CONF_APP_LCD_TYPE    FG040346DSSWBG03
#define MBED_CONF_APP_LCD_TYPE    DVI_STICK
#define MBED_CONF_APP_LCD_TYPE    RGB_TO_HDMI
#define MBED_CONF_APP_LCD_TYPE    LVDS_TO_HDMI
#define MBED_CONF_APP_LCD_TYPE    RSK_TFT

ifeq ($(BOARD), $(filter $(BOARD),GR_MANGO_DD GR_MANGO_BETA_DD))
BOOT_LOADER = 1
endif
ifeq ($(BOARD), $(filter $(BOARD),GR_MANGO GR_MANGO_DD GR_MANGO_BETA GR_MANGO_BETA_DD))
CROSS = 1
MCU_SERIES = RZA2M
TARGET = TARGET_RZ_A2XX
TARGET_BOARD = TARGET_GR_MANGO
TARGET_LD = boards/$(BOARD)/$(BOARD).ld
STACK_USAGE = 16384
LVGL_ENABLE = 1
MBED_OS = 1
MBED_OS_EMAC=0
MBED_OS_EVENTS=0
MBED_GR_LIBS = 1
#MBED_GR_LIBS_LCD_TYPE = RGB_TO_HDMI
MBED_GR_LIBS_LCD_TYPE = ATM0430D25
MBED_GR_LIBS_DRP_FOR_MBED = 0
MBED_GR_LIBS_dcache-control = 1
MBED_GR_LIBS_components_LCD = 1
MBED_GR_LIBS_components_CAMERA = 1
MBED_GR_LIBS_EasyAttach_CameraAndLCD = 1
MBED_GR_LIBS_GraphicsFramework = 1
MBED_GR_LIBS_GR-PEACH_video = 1
MBED_GR_LIBS_utility = 1
MBED_HTTP = 0
MBED_CONSOLE = 1
OVERRIDE_CONSOLE_USBSERIAL = 0
MICROPY_PY_LWIP = 1
MICROPY_SSL_MBEDTLS = 1
MICROPY_PY_USSL = 1
MBED_UART_WRAPPER = 0
MBED_SPI_WRAPPER = 0
MBED_SD_WRAPPER = 0
USE_FONT = 1
USE_LCDSPI = 1
endif

include ../../py/mkenv.mk
-include mpconfigport.mk
include $(BOARD_DIR)/mpconfigboard.mk

# Files that are generated and needed before the QSTR build.
#QSTR_GENERATED_HEADERS = $(BUILD)/pins_qstr.h $(BUILD)/modstm_qstr.h
QSTR_GENERATED_HEADERS = $(BUILD)/pins_qstr.h

# qstr definitions (must come before including py.mk)
QSTR_DEFS = qstrdefsport.h $(QSTR_GENERATED_HEADERS)
QSTR_GLOBAL_DEPENDENCIES = mpconfigboard_common.h $(BOARD_DIR)/mpconfigboard.h $(QSTR_GENERATED_HEADERS)

# MicroPython feature configurations
MICROPY_ROM_TEXT_COMPRESSION ?= 1

# File containing description of content to be frozen into firmware.
FROZEN_MANIFEST ?= boards/manifest.py

#ifeq ($(MICROPY_PY_LVGL), 1)
#LVGL_ENABLE = 1
#endif

# include py core make definitions
include $(TOP)/py/py.mk

GIT_SUBMODULES = lib/lwip lib/mbedtls lib/mynewt-nimble
ifeq ($(MBED_OS), 1)
GIT_SUBMODULES += lib/mbed-os
endif
ifeq ($(MBED_GR_LIBS), 1)
GIT_SUBMODULES += lib/mbed-gr-libs
endif

GCC_VERSION = "`$(CC) -dumpversion`"
GCC_VERSION_9 = "9.0"
GCC_VERSION_8 = "8.0"
GCC_VERSION_7 = "7.0"
GCC_VERSION_6 = "6.0"
GCC_VERSION_5 = "5.0"

#######################################################################
# MCU configuration
#######################################################################

# MCU_SERIERS is defined in mpconfigboard.mk file under board folder
ifeq ($(MCU_SERIES),RZA2M)
MCU_SERIES_LOW = rza2m
MCU_SERIES_CAP = RZA2M
endif
MCU_SERIES_UPPER = $(shell echo $(MCU_SERIES) | tr '[:lower:]' '[:upper:]')
CMSIS_MCU_LOWER = $(shell echo $(CMSIS_MCU) | tr '[:upper:]' '[:lower:]')

LD_DIR=boards

# Select the cross compile prefix
CROSS_COMPILE ?= arm-none-eabi-

#######################################################################
# C include
#######################################################################
INC += -I.
INC += -I$(TOP)
INC += -I$(BUILD)
INC += -I$(BOARD_DIR)
INC += -Irz
INC += -Irz/$(MCU_SERIES_LOW)
INC += -I$(TOP)/lib/oofatfs
ifeq ($(MICROPY_PY_LWIP),1)
INC += -I$(TOP)/lib/lwip/src/include
INC += -Ilwip_inc
INC += -Ilwip_utils
endif
ifeq ($(MICROPY_SSL_MBEDTLS),1)
INC += -I$(TOP)/lib/mbedtls/include
endif
ifeq ($(MICROPY_SSL_AXTLS),1)
INC += -I$(TOP)/lib/axtls/ssl -I$(TOP)/lib/axtls/crypto -I$(TOP)/extmod/axtls-include
endif
ifeq ($(MICROPY_PY_ESP8266),1)
INC += -I$(TOP)/drivers/esp8266/inc
endif
ifeq ($(USE_FONT), 1)
INC += -Ilibs/font
endif
ifeq ($(USE_LCDSPI), 1)
INC += -Ilibs/lcdspi
INC += -Ilibs/jpeg
endif
ifeq ($(USE_DBG_PRINT) ,1)
INC += -Idebug
endif
ifneq ($(MBED_SD_WRAPPER),1)
INC += -I$(MCU_SERIES_LOW)/sd
endif
ifeq ($(LVGL_ENABLE),1)
INC += -Ilvgl
INC += -Ilib/lv_bindings/lvgl
endif
ifeq ($(MBED_GR_LIBS_DRP_FOR_MBED),1)
INC += -I$(TOP)/lib/mbed-gr-libs/drp-for-mbed/TARGET_RZ_A2XX/r_drp/inc
INC += -I$(TOP)/lib/mbed-gr-libs/drp-for-mbed/TARGET_RZ_A2XX/r_drp/src
INC += -I$(TOP)/lib/mbed-gr-libs/drp-for-mbed/TARGET_RZ_A2XX/r_drp/drp_lib
endif

#######################################################################
# C flags.
#######################################################################

# Options for particular MCU series
#CFLAGS_MCU_f0 = $(CFLAGS_CORTEX_M) -mtune=cortex-m0 -mcpu=cortex-m0
#CFLAGS_MCU_f4 = $(CFLAGS_CORTEX_M) -mtune=cortex-m4 -mcpu=cortex-m4
#CFLAGS_MCU_f7 = $(CFLAGS_CORTEX_M) -mtune=cortex-m7 -mcpu=cortex-m7
#CFLAGS_MCU_l0 = $(CFLAGS_CORTEX_M) -mtune=cortex-m0plus -mcpu=cortex-m0plus
#CFLAGS_MCU_l4 = $(CFLAGS_CORTEX_M) -mtune=cortex-m4 -mcpu=cortex-m4
#CFLAGS_MCU_h7 = $(CFLAGS_CORTEX_M) -mtune=cortex-m7 -mcpu=cortex-m7
#CFLAGS_MCU_wb = $(CFLAGS_CORTEX_M) -mtune=cortex-m4 -mcpu=cortex-m4
#CFLAGS_MCU_a9 = -mcpu=cortex-a9 -mthumb-interwork -marm -march=armv7-a -mfpu=vfpv3 -mfloat-abi=hard -mno-unaligned-access
CFLAGS_MCU_a9 = -mcpu=cortex-a9 -mthumb-interwork -marm -mfpu=vfpv3 -mfloat-abi=hard -mno-unaligned-access

C_CPP_COMMON += $(CFLAGS_MCU_a9)
C_CPP_COMMON += -fno-exceptions -fno-builtin -ffunction-sections -fdata-sections -funsigned-char -fno-delete-null-pointer-checks -fomit-frame-pointer
C_CPP_COMMON += $(INC)

CFLAGS += $(C_CPP_COMMON)
#CFLAGS += -Wall -Wpointer-arith -Werror -Wdouble-promotion -Wfloat-conversion
CFLAGS += -Wall -Wpointer-arith -Wdouble-promotion -Wfloat-conversion
CFLAGS += -std=gnu99 -nostdlib $(CFLAGS_MOD) $(CFLAGS_EXTRA)

IS_GCC_ABOVE_VERSION_9 := $(shell expr "$(GCC_VERSION)" ">=" "$(GCC_VERSION_9)")
IS_GCC_ABOVE_VERSION_6 := $(shell expr "$(GCC_VERSION)" ">=" "$(GCC_VERSION_6)")
ifeq "$(IS_GCC_ABOVE_VERSION_9)" "1"
CFLAGS += -Wno-error=sign-compare
CFLAGS += -Wno-error=override-init
CFLAGS += -Wno-error=implicit-function-declaration
CFLAGS += -Wno-error=incompatible-pointer-types
CFLAGS += -Wno-error=unused-variable
CFLAGS += -Wno-error=unused-function
CFLAGS += -Wno-error=main
CFLAGS += -Wno-error=float-conversion
CFLAGS += -Wno-error=unused-but-set-variable
CFLAGS += -Wno-error=return-type
CFLAGS += -Wno-error=maybe-uninitialized
CFLAGS += -Wno-error=missing-braces
CFLAGS += -Wno-error=double-promotion
CFLAGS += -Wno-error=type-limits
CFLAGS += -Wno-error=overflow
CFLAGS += -Wno-error=attributes
CFLAGS += -Wno-error=array-bounds
CFLAGS += -Wno-error=aggressive-loop-optimizations
CFLAGS += -Wno-error=sequence-point
CFLAGS += -Wno-error=int-conversion
else 
ifeq "$(IS_GCC_ABOVE_VERSION_6)" "1"
# gcc version 6.x, 7.x or 8.x
CFLAGS += -Wno-error=main
# sd/sd_write.c
CFLAGS += -Wno-error=maybe-uninitialized
CFLAGS += -Wno-error=missing-braces
else
# gcc version 4.x or 5.x
# objexcept.c
CFLAGS += -Wno-error=sign-compare
# vm.c
CFLAGS += -Wno-error=override-init
# mpconfigport.h __WFI()
CFLAGS += -Wno-error=implicit-function-declaration
# camera.c
CFLAGS += -Wno-error=incompatible-pointer-types
# main.c
CFLAGS += -Wno-error=main
CFLAGS += -Wno-error=format=
# sd/sd_write.c
CFLAGS += -Wno-error=maybe-uninitialized
CFLAGS += -Wno-error=missing-braces
# font.c
CFLAGS += -Wno-incompatible-pointer-types
endif
endif

ifeq ($(MCU_SERIES),RZA2M)
CFLAGS += -DRZA2M
endif

# Configure for nan-boxing object model if requested
ifeq ($(NANBOX),1)
CFLAGS += -DMP_CONFIGFILE='"mpconfigport_nanbox.h"'
ifneq ($(MICROPY_FLOAT_IMPL),none)
MICROPY_FLOAT_IMPL = double
endif
endif

# Configure floating point support
ifeq ($(MICROPY_FLOAT_IMPL),double)
CFLAGS += -DMICROPY_FLOAT_IMPL=MICROPY_FLOAT_IMPL_DOUBLE
else
ifeq ($(MICROPY_FLOAT_IMPL),none)
CFLAGS += -DMICROPY_FLOAT_IMPL=MICROPY_FLOAT_IMPL_NONE
else
CFLAGS += -DMICROPY_FLOAT_IMPL=MICROPY_FLOAT_IMPL_FLOAT
CFLAGS += -fsingle-precision-constant
endif
endif

#LD = arm-none-eabi-ld
LD = arm-none-eabi-g++
ifeq ($(LD),arm-none-eabi-ld)
#LDFLAGS = -nostdlib -L $(LD_DIR) $(addprefix -T,$(LD_FILES)) -Map=$(@:.elf=.map) --cref
#LDFLAGS += --defsym=_estack_reserve=8
LIBS = $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)

# Remove uncalled code from the final image.
#CFLAGS += -fdata-sections -ffunction-sections
LDFLAGS += --gc-sections

else
# Remove uncalled code from the final image.
#CFLAGS += -fdata-sections -ffunction-sections
LDFLAGS += -Wl,--gc-sections

LDFLAGS += -T $(TARGET_LD) -Wl,-Map=$@.map
#LDFLAGS += -Wl,--wrap,_malloc_r -Wl,--wrap,_free_r -Wl,--wrap,_realloc_r -Wl,--wrap,_calloc_r
LDFLAGS += -Wl,--wrap,main -Wl,--wrap,_memalign_r -Wl,--wrap,exit -Wl,--wrap,atexit -Wl,-n 
LDFLAGS += -Wl,--start-group -Wl,--end-group 
LDFLAGS += $(CFLAGS_MCU_a9) -march=armv7-a 
LDFLAGS += -g
LDFLAGS += -lstdc++ -lsupc++ -lm -lc -lgcc -lnosys -specs=nosys.specs 
endif

# Debugging/Optimization
ifeq ($(DEBUG), 1)
#CFLAGS += -g -DPENDSV_DEBUG
CFLAGS += -g3
COPT += -Os
C_CPP_COMMON += -Os -g3
MICROPY_ROM_TEXT_COMPRESSION = 0
else
COPT += -Os -DNDEBUG
endif
CFLAGS += $(COPT)

# Flags for optional C++ source code
CXXFLAGS += $(filter-out -Wmissing-prototypes -Wold-style-definition -std=gnu99,$(CFLAGS))
CXXFLAGS += $(CXXFLAGS_MOD)
ifneq ($(SRC_CXX)$(SRC_MOD_CXX),)
LIBSTDCPP_FILE_NAME = "$(shell $(CXX) $(CXXFLAGS) -print-file-name=libstdc++.a)"
LDFLAGS += -L"$(shell dirname $(LIBSTDCPP_FILE_NAME))"
endif

# Options for mpy-cross
MPY_CROSS_FLAGS += -march=armv7m

ifeq ($(BOARD),$(filter $(BOARD),GR_MANGO GR_MANGO_DD GR_MANGO_BETA GR_MANGO_BETA_DD))
CFLAGS += -DGRMANGO
endif

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

ifeq ($(LVGL_ENABLE),1)
CFLAGS += -DLVGL_ENABLE
endif

ifeq ($(MBED_GR_LIBS),1)
CFLAGS += -DMBED_GR_LIBS
endif

ifeq ($(MBED_GR_LIBS_components_LCD),1)
CFLAGS += -DMBED_GR_LIBS_components_LCD
endif

ifeq ($(MBED_GR_LIBS_DRP_FOR_MBED),1)
CFLAGS += -DUSE_DRP=1
endif

#######################################################################
# source files
#######################################################################

SHARED_SRC_C += $(addprefix shared/,\
	libc/string0.c \
	netutils/dhcpserver.c \
	netutils/netutils.c \
	netutils/trace.c \
	readline/readline.c \
	runtime/gchelper_native.c \
	runtime/interrupt_char.c \
	runtime/mpirq.c \
	runtime/pyexec.c \
	runtime/stdout_helpers.c \
	runtime/sys_stdio_mphal.c \
	timeutils/timeutils.c \
	)

ifeq ($(MICROPY_FLOAT_IMPL),double)
LIBM_SRC_C += $(addprefix lib/libm_dbl/,\
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
	round.c \
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
LIBM_SRC_C += lib/libm_dbl/thumb_vfp_sqrt.c
else
LIBM_SRC_C += lib/libm_dbl/sqrt.c
endif
else
LIBM_SRC_C += $(addprefix lib/libm/,\
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
	roundf.c \
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
LIBM_SRC_C += lib/libm/thumb_vfp_sqrtf.c
else
LIBM_SRC_C += lib/libm/ef_sqrt.c
endif
endif

LIBM_O = $(addprefix $(BUILD)/, $(LIBM_SRC_C:.c=.o))

# Too many warnings in libm_dbl, disable for now.
ifeq ($(MICROPY_FLOAT_IMPL),double)
$(LIBM_O): CFLAGS := $(filter-out -Wdouble-promotion -Wfloat-conversion, $(CFLAGS))
endif

EXTMOD_SRC_C += $(addprefix extmod/,\
	modonewire.c \
	modnetwork.c \
	modusocket.c \
	)

DRIVERS_SRC_C += $(addprefix drivers/,\
	bus/softspi.c \
	bus/softqspi.c \
	memory/spiflash.c \
	dht/dht.c \
	)

SRC_C += \
	boardctrl.c \
	main.c \

include mbed.mk

#######################################################################
# RZA2M HAL
#######################################################################

ifeq ($(TARGET), TARGET_RZ_A2XX)

ifeq ($(BOOT_LOADER),1)
SRC_C += boards/$(BOARD)/$(BOARD)_bl.c
endif

SRC_C += rz/$(MCU_SERIES_LOW)/bootstrap.c
SRC_C += rz/rz_adc.c
SRC_C += rz/rz_exti.c
SRC_C += rz/rz_gpio.c
ifneq ($(MBED_OS_EMAC),1)
SRC_C += rz/rz_ether.c
endif
SRC_C += rz/rz_init.c
SRC_C += rz/rz_mtu.c
SRC_C += rz/rz_servo.c
SRC_C += rz/rz_ostm2.c
SRC_C += rz/rz_rtc.c
ifneq ($(MBED_UART_WRAPPER),1)
SRC_C += rz/rz_sci.c
endif
ifneq ($(MBED_SPI_WRAPPER),1)
SRC_C += rz/rz_spi.c
endif
SRC_C += rz/rz_utils.c
ifneq ($(MBED_SD_WRAPPER),1)
SRC_C += rz/rz_sd.c
SRC_C += rz/sd/sd_dev_low.c
SRC_C += rz/sd/sd_device.c
SRC_C += rz/sd/sd_cd.c
SRC_C += rz/sd/sd_cmd.c
SRC_C += rz/sd/sd_int.c
SRC_C += rz/sd/sd_init.c
SRC_C += rz/sd/sd_mount.c
SRC_C += rz/sd/sd_read.c
SRC_C += rz/sd/sd_trns.c
SRC_C += rz/sd/sd_util.c
SRC_C += rz/sd/sd_write.c
endif

ifeq ($(MBED_GR_LIBS), 1)
ifeq ($(MBED_GR_LIBS_components_CAMERA), 1)
EXTMOD_SRC_C += display.c
EXTMOD_SRC_C += rz_camera.c
EXTMOD_SRC_C += rz_display.c
EXTMOD_SRC_C += rz_lcd.c
EXTMOD_SRC_C += rz_buf.c
ifeq ($(MBED_GR_LIBS_DRP_FOR_MBED),1)
EXTMOD_SRC_C += rz_drp.c
EXTMOD_SRC_C += rz_isp.c
endif
endif
endif

endif

#######################################################################
# micropython rz CPU
#######################################################################

SRC_C += adc.c
SRC_C += bufhelper.c
SRC_C += eth.c
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
SRC_C += machine_i2c.c
SRC_C += machine_spi.c
SRC_C += machine_timer.c
SRC_C += machine_uart.c
SRC_C += modmachine.c
ifeq ($(MICROPY_PY_ESP8266),1)
CFLAGS += -DMICROPY_PY_ESP8266=1
SRC_C += network_esp8266.c
endif
SRC_C += modpyb.c
SRC_C += modrz.c
SRC_C += modrzreg.c
SRC_C += modutime.c
SRC_C += mphalport.c
SRC_C += mpnetworkport.c
SRC_C += mpthreadport.c
SRC_C += network_lan.c
SRC_C += pendsv.c
ifneq ($(MBED_OS_EMAC),1)
SRC_C += phy.c
endif
SRC_C += pin.c
SRC_C += pin_defs_rz.c
SRC_C += pin_named_pins.c
SRC_C += pwm.c
SRC_C += pyb_i2c.c
SRC_C += pyb_spi.c
SRC_C += rng.c
SRC_C += rtc.c
SRC_C += sdcard.c
SRC_C += servo.c
SRC_C += softtimer.c
SRC_C += spi.c
SRC_C += storage.c
SRC_C += systick.c
#SRC_C += timer.c
SRC_C += uart.c
SRC_C += usrsw.c
SRC_C += mpy_file.c
SRC_C += rz_buf.c

ifeq ($(USE_DBG_PRINT) ,1)
SRC_C += debug/debug_printf.c
SRC_C += debug/vsnprintf.c
endif

ifeq ($(USE_FONT), 1)
SRC_C += libs/font/font.c
SRC_C += rz_font.c
endif
ifeq ($(USE_LCDSPI), 1)
SRC_C += libs/jpeg/jpeg_disp.c
SRC_C += libs/jpeg/jpeg_src.c
SRC_C += libs/jpeg/jpeg.c
SRC_C += libs/jpeg/picojpeg.c
SRC_C += libs/lcdspi/lcdspi_file.c
SRC_C += libs/lcdspi/lcdspi_info.c
SRC_C += libs/lcdspi/lcdspi.c
SRC_C += rz_lcdspi.c
endif

ifeq ($(MICROPY_PY_BLUETOOTH),1)
CFLAGS_MOD += -DMICROPY_PY_BLUETOOTH=1
CFLAGS_MOD += -DMICROPY_PY_BLUETOOTH_ENABLE_CENTRAL_MODE=1
endif

ifeq ($(MICROPY_PY_NETWORK_CYW43),1)
CFLAGS_MOD += -DMICROPY_PY_NETWORK_CYW43=1
SRC_C += sdio.c
EXTMOD_SRC_C += extmod/network_cyw43.c
DRIVERS_SRC_C += drivers/cyw43/cyw43_ctrl.c drivers/cyw43/cyw43_lwip.c
LIBS += $(TOP)/drivers/cyw43/libcyw43.a
endif

ifneq ($(MICROPY_PY_WIZNET5K),0)
WIZNET5K_DIR=drivers/wiznet5k
INC += -I$(TOP)/$(WIZNET5K_DIR)
CFLAGS_MOD += -DMICROPY_PY_WIZNET5K=$(MICROPY_PY_WIZNET5K) -D_WIZCHIP_=$(MICROPY_PY_WIZNET5K)
ifeq ($(MICROPY_PY_LWIP),1)
# When using MACRAW mode (with lwIP), maximum buffer space must be used for the raw socket
CFLAGS_MOD += -DWIZCHIP_USE_MAX_BUFFER
endif
SRC_MOD += network_wiznet5k.c modnwwiznet5k.c
SRC_MOD += $(addprefix $(WIZNET5K_DIR)/,\
	ethernet/w$(MICROPY_PY_WIZNET5K)/w$(MICROPY_PY_WIZNET5K).c \
	ethernet/wizchip_conf.c \
	ethernet/socket.c \
	internet/dns/dns.c \
	)
endif

# for CC3000 module
ifeq ($(MICROPY_PY_CC3K),1)
CC3000_DIR=drivers/cc3000
INC += -I$(TOP)/$(CC3000_DIR)/inc
CFLAGS_MOD += -DMICROPY_PY_CC3K=1
SRC_MOD += modnwcc3k.c
SRC_MOD += $(addprefix $(CC3000_DIR)/src/,\
	cc3000_common.c \
	evnt_handler.c \
	hci.c \
	netapp.c \
	nvmem.c \
	security.c \
	socket.c \
	wlan.c \
	ccspi.c \
	inet_ntop.c \
	inet_pton.c \
	patch.c \
	patch_prog.c \
	)
endif

# for ESP8266 module
ifeq ($(MICROPY_PY_ESP8266),1)
ESP8266_DIR=drivers/esp8266
INC += -I$(TOP)/$(ESP8266_DIR)/inc
CFLAGS += -DMICROPY_PY_ESP8266=1
#SRC_MOD += modnwesp8266.c
SRC_MOD += $(addprefix $(ESP8266_DIR)/src/,\
	esp8266_driver.c \
	vector.c \
	tinymalloc.c \
	)
endif

ifeq ($(MICROPY_SSL_MBEDTLS),1)
CFLAGS_MOD += -DMBEDTLS_CONFIG_FILE='"mbedtls/mbedtls_config.h"'
SRC_MOD += mbedtls/mbedtls_port.c
# replace mbedtls' error.c by ours
SRC_MOD := $(filter-out %/mbedtls/library/error.c, $(SRC_MOD))
LIB_SRC_C += lib/mbedtls_errors/mp_mbedtls_errors.c
endif

ifeq ($(MICROPY_PY_BLUETOOTH),1)

SRC_C += mpbthciport.c

ifeq ($(MICROPY_BLUETOOTH_NIMBLE),1)
ifeq ($(MICROPY_BLUETOOTH_BTSTACK),1)
$(error Cannot enable both NimBLE and BTstack at the same time)
endif
endif

ifeq ($(MICROPY_BLUETOOTH_NIMBLE),1)
GIT_SUBMODULES += lib/mynewt-nimble
CFLAGS_MOD += -DMICROPY_PY_BLUETOOTH_ENABLE_L2CAP_CHANNELS=1
include $(TOP)/extmod/nimble/nimble.mk
SRC_C += mpnimbleport.c
endif

ifeq ($(MICROPY_BLUETOOTH_BTSTACK),1)
GIT_SUBMODULES += lib/btstack
MICROPY_BLUETOOTH_BTSTACK_H4 ?= 1
include $(TOP)/extmod/btstack/btstack.mk
SRC_C += mpbtstackport.c
endif

ifeq ($(MICROPY_PY_NETWORK_CYW43),1)
DRIVERS_SRC_C += drivers/cyw43/cywbt.c
endif

endif

ifeq ($(LVGL_ENABLE) ,1)
SRC_C += lvgl/modlvrz.c
SRC_C += lvgl/driver/modILI9341.c
SRC_C += lvgl/driver/modrtch.c
SRC_C += lvgl/driver/modstmpe610.c
SRC_C += lvgl/driver/modrzlcd.c
SRC_C += lvgl/driver/modxpt2046.c
endif

#######################################################################
# object files
#######################################################################

OBJ += $(PY_O)
OBJ += $(addprefix $(BUILD)/, $(LIB_SRC_C:.c=.o))
OBJ += $(LIBM_O)
OBJ += $(addprefix $(BUILD)/, $(SHARED_SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(EXTMOD_SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(DRIVERS_SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_CXX:.cpp=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_S:.S=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_CPP:.cpp=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_O))
OBJ += $(addprefix $(BUILD)/, $(SRC_MOD:.c=.o))
OBJ += $(BUILD)/pins_$(BOARD).o

# This file contains performance critical functions so turn up the optimisation
# level.  It doesn't add much to the code size and improves performance a bit.
# Don't use -O3 with this file because gcc tries to optimise memset in terms of itself.
$(BUILD)/shared/libc/string0.o: COPT += -O2

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

#######################################################################
# build rules
#######################################################################

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

$(BUILD)/firmware.hex: $(BUILD)/firmware.elf
	$(ECHO) "GEN $@"
	$(Q)$(OBJCOPY) -O ihex $< $@

$(BUILD)/firmware.elf: $(OBJ) $(LIB_MBED) $(LIB_LVGL)
	$(ECHO) "LINK $@"
	$(Q)$(LD) $(LDFLAGS) -o $@ $^
	$(Q)$(OBJCOPY) -O srec $(BUILD)/firmware.elf  $(BUILD)/firmware.mot	
	$(Q)$(SIZE) $@

ifeq ($(BOOT_LOADER),1)
$(BUILD)/firmware.bin: $(BUILD)/firmware.elf
	$(Q)$(OBJCOPY) -O binary -j .boot -j .isr_vector -j .text -j .ARM.extab -j .ARM.exidx -j .copy.table -j .zero.table -j .ram_code -j .data -j .nc_data -j .octa_data $^ $(BUILD)/firmware.bin
#	$(Q)$(OBJDUMP) -S -D $(BUILD)/firmware.elf > $(BUILD)/firmware.s	
else
$(BUILD)/firmware.bin: $(BUILD)/firmware.elf
	$(Q)$(OBJCOPY) -O binary -j .isr_vector -j .text -j .ARM.extab -j .ARM.exidx -j .copy.table -j .zero.table -j .ram_code -j .data -j .nc_data -j .octa_data $^ $(BUILD)/firmware.bin
#	$(Q)$(OBJDUMP) -S -D $(BUILD)/firmware.elf > $(BUILD)/firmware.s	
endif

MAKE_PINS = boards/make-pins.py
BOARD_PINS = $(BOARD_DIR)/pins.csv
AF_FILE = boards/$(MCU_SERIES_LOW)_af.csv
PREFIX_FILE = boards/$(MCU_SERIES_LOW)_prefix.c
GEN_PINS_SRC = $(BUILD)/pins_$(BOARD).c
GEN_PINS_HDR = $(HEADER_BUILD)/pins.h
GEN_PINS_QSTR = $(BUILD)/pins_qstr.h
GEN_PINS_AF_CONST = $(HEADER_BUILD)/pins_af_const.h
#GEN_PINS_AF_DEFS = $(HEADER_BUILD)/pins_af_defs.h
GEN_PINS_AF_PY = $(BUILD)/pins_af.py

INSERT_USB_IDS = $(TOP)/tools/insert-usb-ids.py
FILE2H = $(TOP)/tools/file2h.py

USB_IDS_FILE = mpconfigboard_common.h
CDCINF_TEMPLATE = pybcdc.inf_template
GEN_CDCINF_FILE = $(HEADER_BUILD)/pybcdc.inf
GEN_CDCINF_HEADER = $(HEADER_BUILD)/pybcdc_inf.h

# List of sources for qstr extraction
SRC_QSTR += $(SRC_C) $(SRC_CXX) $(SRC_MOD) $(SHARED_SRC_C) $(EXTMOD_SRC_C)
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
$(BUILD)/%_$(BOARD).c $(HEADER_BUILD)/%.h $(HEADER_BUILD)/%_af_const.h $(HEADER_BUILD)/%_af_defs.h $(BUILD)/%_qstr.h: $(BOARD_DIR)/%.csv $(MAKE_PINS) $(AF_FILE) $(PREFIX_FILE) | $(HEADER_BUILD)
	$(ECHO) "GEN $@"
	$(Q)$(PYTHON) $(MAKE_PINS) --board $(BOARD_PINS) --af $(AF_FILE) \
		--prefix $(PREFIX_FILE) --hdr $(GEN_PINS_HDR) \
		--qstr $(GEN_PINS_QSTR) --af-const $(GEN_PINS_AF_CONST) \
		--af-py $(GEN_PINS_AF_PY) > $(GEN_PINS_SRC)

$(BUILD)/pins_$(BOARD).o: $(BUILD)/pins_$(BOARD).c
	$(call compile_c)

#GEN_PLLFREQTABLE_HDR = $(HEADER_BUILD)/pllfreqtable.h
#GEN_STMCONST_HDR = $(HEADER_BUILD)/modstm_const.h
#GEN_STMCONST_QSTR = $(BUILD)/modstm_qstr.h
#GEN_STMCONST_MPZ = $(HEADER_BUILD)/modstm_mpz.h
#CMSIS_MCU_HDR = $(CMSIS_DIR)/$(CMSIS_MCU_LOWER).h

#modmachine.c: $(GEN_PLLFREQTABLE_HDR)
#$(GEN_PLLFREQTABLE_HDR): $(PLLVALUES) | $(HEADER_BUILD)
#	$(ECHO) "GEN $@"
#	$(Q)$(PYTHON) $(PLLVALUES) -c -m $(MCU_SERIES) #file:$(BOARD_DIR)/stm32$(MCU_SERIES)xx_hal_conf.h > $@

#$(BUILD)/modstm.o: $(GEN_STMCONST_HDR)
# Use a pattern rule here so that make will only call make-stmconst.py once to
# make both modstm_const.h and modstm_qstr.h
#$(HEADER_BUILD)/%_const.h $(BUILD)/%_qstr.h: $(CMSIS_MCU_HDR) #make-stmconst.py | $(HEADER_BUILD)
#	$(ECHO) "GEN stmconst $@"
#	$(Q)$(PYTHON) make-stmconst.py --qstr $(GEN_STMCONST_QSTR) --mpz #$(GEN_STMCONST_MPZ) $(CMSIS_MCU_HDR) > $(GEN_STMCONST_HDR)

$(GEN_CDCINF_HEADER): $(GEN_CDCINF_FILE) $(FILE2H) | $(HEADER_BUILD)
	$(ECHO) "GEN $@"
	$(Q)$(PYTHON) $(FILE2H) $< > $@

$(GEN_CDCINF_FILE): $(CDCINF_TEMPLATE) $(INSERT_USB_IDS) $(USB_IDS_FILE) | $(HEADER_BUILD)
	$(ECHO) "GEN $@"
	$(Q)$(PYTHON) $(INSERT_USB_IDS) $(USB_IDS_FILE) $< > $@

include $(TOP)/py/mkrules.mk
