# Select the board to build for: if not given on the command line,
# then default to GR_MANGO
BOARD ?= GR_MANGO

CROSS ?= 1

# If the build directory is not given, make it reflect the board name.
BUILD ?= build-$(BOARD)

BOARD_DIR ?= boards/$(BOARD)
ifeq ($(wildcard $(BOARD_DIR)/.),)
$(error Invalid BOARD specified: $(BOARD_DIR))
endif

# enable debug_printf
USE_DBG_PRINT = 0

# enable micropython debug flag
#MICROPY_DEBUG_VERBOSE = 1
#MICROPY_DEBUG_PRINTERS = 1

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
MBED_OS = 1
MBED_OS_EMAC=0
MBED_OS_EVENTS=0
MBED_GR_LIBS = 1
#MBED_GR_LIBS_LCD_TYPE = RGB_TO_HDMI
MBED_GR_LIBS_LCD_TYPE = ATM0430D25
MBED_GR_LIBS_DRP_FOR_MBED = 1
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
QSTR_GENERATED_HEADERS = $(GEN_PINS_QSTR)

# qstr definitions (must come before including py.mk)
QSTR_DEFS += qstrdefsport.h $(QSTR_GENERATED_HEADERS)
QSTR_GLOBAL_DEPENDENCIES += mpconfigboard_common.h $(BOARD_DIR)/mpconfigboard.h $(QSTR_GENERATED_HEADERS)

# MicroPython feature configurations
MICROPY_ROM_TEXT_COMPRESSION ?= 1

# File containing description of content to be frozen into firmware.
FROZEN_MANIFEST ?= boards/manifest.py

# include py core make definitions
include $(TOP)/py/py.mk
include $(TOP)/extmod/extmod.mk

GIT_SUBMODULES += lib/libhydrogen
ifeq ($(MBED_OS), 1)
GIT_SUBMODULES += lib/mbed-os
endif
ifeq ($(MBED_GR_LIBS), 1)
GIT_SUBMODULES += lib/mbed-gr-libs
endif

query-variants:
	$(ECHO) "VARIANTS:" $(BOARD_VARIANTS)

LD_DIR=boards

include renesas-rz.mk
# PLLVALUES = boards/pllvalues.py
MAKE_PINS = boards/make-pins.py
BOARD_PINS = $(BOARD_DIR)/pins.csv
AF_FILE = boards/$(MCU_SERIES_LOWER)_af.csv
PREFIX_FILE = boards/$(MCU_SERIES_LOWER)_prefix.c
GEN_PINS_SRC = $(BUILD)/pins_$(BOARD).c
GEN_PINS_HDR = $(HEADER_BUILD)/pins.h
GEN_PINS_QSTR = $(BUILD)/pins_qstr.h
GEN_PINS_AF_CONST = $(HEADER_BUILD)/pins_af_const.h
# GEN_PINS_AF_DEFS = $(HEADER_BUILD)/pins_af_defs.h
GEN_PINS_AF_PY = $(BUILD)/pins_af.py

INSERT_USB_IDS = $(TOP)/tools/insert-usb-ids.py
FILE2H = $(TOP)/tools/file2h.py

USB_IDS_FILE = mpconfigboard_common.h
CDCINF_TEMPLATE = pybcdc.inf_template
GEN_CDCINF_FILE = $(HEADER_BUILD)/pybcdc.inf
GEN_CDCINF_HEADER = $(HEADER_BUILD)/pybcdc_inf.h

# GEN_PLLFREQTABLE_HDR = $(HEADER_BUILD)/pllfreqtable.h
# GEN_STMCONST_HDR = $(HEADER_BUILD)/modstm_const.h
# GEN_STMCONST_QSTR = $(BUILD)/modstm_qstr.h
# GEN_STMCONST_MPZ = $(HEADER_BUILD)/modstm_mpz.h
# CMSIS_MCU_HDR = $(STM32LIB_CMSIS_ABS)/Include/$(CMSIS_MCU_LOWER).h

# Select the cross compile prefix
CROSS_COMPILE ?= arm-none-eabi-

#######################################################################
# C include
#######################################################################
INC += -I.
INC += -I$(TOP)
INC += -I$(TOP)/lib/lv_bindings
INC += -I$(BUILD)
INC += -I$(BOARD_DIR)
INC += -Irz
INC += -Irz/$(MCU_SERIES_LOWER)
INC += -I$(TOP)/lib/oofatfs
ifeq ($(MICROPY_PY_LWIP),1)
INC += -Ilwip_inc
INC += -Ilwip_utils
endif
ifneq ($(MBED_SD_WRAPPER),1)
INC += -I$(MCU_SERIES_LOWER)/sd
endif
ifeq ($(USE_DBG_PRINT) ,1)
INC += -Idebug
endif
ifeq ($(USE_FONT), 1)
INC += -I$(TOP)/lib/lcdspi/font
endif
ifeq ($(USE_LCDSPI), 1)
INC += -I$(TOP)/lib/lcdspi/lcdspi
INC += -I$(TOP)/lib/lcdspi/jpeg
INC += -I$(TOP)/lib/lcdspi/xpt2046
endif
ifeq ($(MICROPY_PY_LVGL),1)
INC += -Ilvgl
INC += -I$(TOP)/lib/lv_bindings/lvgl
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
CFLAGS_MCU_a9 = -mcpu=cortex-a9 -mthumb-interwork -marm -mfpu=vfpv3 -mfloat-abi=hard -mno-unaligned-access

C_CPP_COMMON += $(CFLAGS_MCU_a9)
C_CPP_COMMON += -fno-exceptions -fno-builtin -ffunction-sections -fdata-sections -funsigned-char -fno-delete-null-pointer-checks -fomit-frame-pointer
C_CPP_COMMON += $(INC)

CFLAGS += $(C_CPP_COMMON)
#CFLAGS += -Wall -Wpointer-arith -Werror -Wdouble-promotion -Wfloat-conversion
CFLAGS += -Wall -Wpointer-arith -Wdouble-promotion -Wfloat-conversion
CFLAGS += -std=gnu99 -nostdlib $(CFLAGS_MOD) $(CFLAGS_EXTRA)

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
# COPT ?= -Og
CFLAGS += -Os -g3
C_CPP_COMMON += -Os -g3
MICROPY_ROM_TEXT_COMPRESSION = 0
else
COPT ?= -Os -DNDEBUG
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

ifeq ($(MBED_GR_LIBS),1)
CFLAGS += -DMBED_GR_LIBS
endif

ifeq ($(MBED_GR_LIBS_components_LCD),1)
CFLAGS += -DMBED_GR_LIBS_components_LCD
endif

ifeq ($(MBED_GR_LIBS_DRP_FOR_MBED),1)
CFLAGS += -DUSE_DRP=1
endif

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

DRIVERS_SRC_C += $(addprefix drivers/,\
	bus/softspi.c \
	bus/softqspi.c \
	memory/spiflash.c \
	dht/dht.c \
	)

SRC_C += \
	boardctrl.c \
	main.c \
	mphalport.c \
	mpnetworkport.c \
	mpthreadport.c \
	irq.c \
	pendsv.c \
	systick.c  \
	factoryreset.c \
	led.c \
	pin.c \
	pin_defs_rz.c \
	pin_named_pins.c \
	bufhelper.c \
	i2c.c \
	pyb_i2c.c \
	spi.c \
	pyb_spi.c \
	uart.c \
	eth.c \
	gccollect.c \
	help.c \
	machine_i2c.c \
	machine_spi.c \
	machine_uart.c \
	modmachine.c \
	modpyb.c \
	modrz.c \
	network_lan.c \
	extint.c \
	usrsw.c \
	rng.c \
	rtc.c \
	flash.c \
	flashbdev.c \
	storage.c \
	sdcard.c \
	fatfs_port.c \
	lcd.c \
	servo.c \
	adc.c \
	eth.c \
	modrzreg.c \
	mpy_file.c \
	phy.c \
	pwm.c \
	servo.c \
	softtimer.c \
	rz_buf.c \
	$(wildcard $(BOARD_DIR)/*.c)

SRC_S += 	gchelper_a9.S

include mbed.mk

#######################################################################
# RZA2M HAL
#######################################################################

ifeq ($(TARGET), TARGET_RZ_A2XX)

ifeq ($(BOOT_LOADER),1)
SRC_C += boards/$(BOARD)/$(BOARD)_bl.c
endif

ifeq ($(MBED_GR_LIBS), 1)
ifeq ($(MBED_GR_LIBS_components_CAMERA), 1)
SRC_C += display.c
SRC_C += rz_camera.c
SRC_C += rz_display.c
SRC_C += rz_lcd.c
SRC_C += rz_buf.c
ifeq ($(MBED_GR_LIBS_DRP_FOR_MBED),1)
SRC_C += rz_drp.c
SRC_C += rz_isp.c
endif
endif
endif

endif

ifeq ($(USE_DBG_PRINT) ,1)
SRC_C += debug/debug_printf.c
SRC_C += debug/vsnprintf.c
endif

SRC_CXX += \
	$(SRC_MOD_CXX)

SRC_O += \
	$(STARTUP_FILE) \
	$(SYSTEM_FILE)

HAL_SRC_C += \
	rz/$(MCU_SERIES_LOWER)/bootstrap.c \
	rz/rz_adc.c \
	rz/rz_exti.c \
	rz/rz_gpio.c \
	rz/rz_ether.c \
	rz/rz_init.c \
	rz/rz_mtu.c \
	rz/rz_servo.c \
	rz/rz_ostm2.c \
	rz/rz_rtc.c \
	rz/rz_sci.c \
	rz/rz_spi.c \
	rz/rz_utils.c \
	rz/rz_sd.c \
	rz/sd/sd_dev_low.c \
	rz/sd/sd_device.c \
	rz/sd/sd_cd.c \
	rz/sd/sd_cmd.c \
	rz/sd/sd_int.c \
	rz/sd/sd_init.c \
	rz/sd/sd_mount.c \
	rz/sd/sd_read.c \
	rz/sd/sd_trns.c \
	rz/sd/sd_util.c \
	rz/sd/sd_write.c

# for ESP module
ifeq ($(MICROPY_PY_ESP),1)
ESP_DIR=drivers/esp
INC += -I$(TOP)/$(ESP_DIR)
CFLAGS += -DMICROPY_PY_ESP=1
SRC_C += extmod/network_esp.c
DRIVERS_SRC_C += $(addprefix $(ESP_DIR)/,\
	esp_driver.c \
	mpy_uart.c \
	mpy_debug.c \
	vector.c \
	tinymalloc.c \
	)
endif

ifeq ($(MICROPY_SSL_MBEDTLS),1)
CFLAGS += -DMBEDTLS_CONFIG_FILE='"mbedtls/mbedtls_config.h"'
LIB_SRC_C += mbedtls/mbedtls_port.c
endif

ifeq ($(USE_FONT),1)
FONT_DIR=lib/lcdspi/font
INC += -I$(TOP)/lib/lcdspi/font
SRC_C += rz_font.c
LIB_SRC_C += $(addprefix $(FONT_DIR)/,\
    font.c \
    )
endif

ifeq ($(USE_LCDSPI),1)
LCDSPI_DIR=lib/lcdspi
INC += -I$(TOP)/lib/lcdspi/lcdspi/lcdspi
INC += -I$(TOP)/lib/lcdspi/lcdspi/jpeg
INC += -I$(TOP)/lib/lcdspi/lcdspi/xpt2046
SRC_C += rz_lcdspi.c
LIB_SRC_C += $(addprefix $(LCDSPI_DIR)/,\
    lcdspi/lcdspi_file.c \
    lcdspi/lcdspi_info.c \
    lcdspi/lcdspi.c \
    jpeg/jpeg_disp.c \
    jpeg/jpeg_src.c \
    jpeg/jpeg.c \
    jpeg/picojpeg.c \
    xpt2046/xpt2046.c \
    )
endif

ifeq ($(MICROPY_PY_LVGL) ,1)
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
# SRC_O should be placed first to work around this LTO bug with binutils <2.35:
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=83967
OBJ += $(addprefix $(BUILD)/, $(SRC_O))
OBJ += $(PY_O)
OBJ += $(addprefix $(BUILD)/, $(LIB_SRC_C:.c=.o))
OBJ += $(LIBM_O)
OBJ += $(addprefix $(BUILD)/, $(SHARED_SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(DRIVERS_SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(HAL_SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_CXX:.cpp=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_CPP:.cpp=.o))
OBJ += $(GEN_PINS_SRC:.c=.o)
OBJ += $(addprefix $(BUILD)/, $(SRC_S:.S=.o))

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

ifneq ($(FROZEN_MANIFEST),)
CFLAGS += -DMICROPY_QSTR_EXTRA_POOL=mp_qstr_frozen_const_pool
CFLAGS += -DMICROPY_MODULE_FROZEN_MPY
CFLAGS += -DMICROPY_MODULE_FROZEN_STR
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

# List of sources for qstr extraction
SRC_QSTR += $(SRC_C) $(SRC_CXX) $(SHARED_SRC_C)
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


$(GEN_CDCINF_HEADER): $(GEN_CDCINF_FILE) $(FILE2H) | $(HEADER_BUILD)
	$(ECHO) "GEN $@"
	$(Q)$(PYTHON) $(FILE2H) $< > $@

$(GEN_CDCINF_FILE): $(CDCINF_TEMPLATE) $(INSERT_USB_IDS) $(USB_IDS_FILE) | $(HEADER_BUILD)
	$(ECHO) "GEN $@"
	$(Q)$(PYTHON) $(INSERT_USB_IDS) $(USB_IDS_FILE) $< > $@

include $(TOP)/py/mkrules.mk
