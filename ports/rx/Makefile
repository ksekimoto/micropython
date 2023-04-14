# Select the board to build for: if not given on the command line,
# then default to GR_ROSE
BOARD ?= GR_ROSE

# If the build directory is not given, make it reflect the board name.
BUILD ?= build-$(BOARD)

BOARD_DIR ?= boards/$(BOARD)
ifeq ($(wildcard $(BOARD_DIR)/.),)
$(error Invalid BOARD specified: $(BOARD_DIR))
endif

# enable debug_printf
USE_DBG_PRINT = 1

# enable micropython debug flag
#MICROPY_DEBUG_VERBOSE = 1
#MICROPY_DEBUG_PRINTERS = 1

# Use developing usb library instead of stable one for testing
# Use ramdisk instead of CPU internal flash for testing the related codes
#TEST_RAMDISK = 1

# Select USB device type
USB_CDC = 1
USB_MSC = 2
USB_CDC_MSC = 3
USB_MODE = 3

# Include modules
USE_FONT = 1
USE_LCDSPI = 1

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

query-variants:
	$(ECHO) "VARIANTS:" $(BOARD_VARIANTS)

LD_DIR=boards

include renesas-rx.mk

# PLLVALUES = boards/pllvalues.py
MAKE_PINS = boards/make-pins.py
BOARD_PINS = $(BOARD_DIR)/pins.csv
AF_FILE = boards/$(MCU_SERIES_LOWER)_af.csv
PREFIX_FILE = boards/$(MCU_SERIES_LOWER)_prefix.c
GEN_PINS_SRC = $(BUILD)/pins_$(BOARD).c
GEN_PINS_HDR = $(HEADER_BUILD)/pins.h
GEN_PINS_QSTR = $(BUILD)/pins_qstr.h
GEN_PINS_AD_CONST = $(HEADER_BUILD)/pins_ad_const.h
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
CROSS_COMPILE ?= rx-elf-
IS_RENESAS_GCC := $(shell if [ "`rx-elf-gcc --version | grep GNURX`" ]; then echo "GNURX"; fi)

INC += -I.
INC += -I$(TOP)
INC += -I$(BUILD)
INC += -I$(BOARD_DIR)
INC += -Irx
INC += -Irx/$(MCU_SERIES_LOWER)
INC += -Idebug
INC += -Iramdisk
INC += -Iusb
ifeq ($(MICROPY_PY_LWIP),1)
INC += -Ilwip_inc
INC += -Ilwip_utils
endif
INC += -Isd
ifeq ($(USE_DBG_PRINT),1)
INC += -Idebug
endif
ifeq ($(USE_FONT),1)
INC += -I$(TOP)/lib/lcdspi/font
endif
ifeq ($(USE_LCDSPI),1)
INC += -I$(TOP)/lib/lcdspi/lcdspi
INC += -I$(TOP)/lib/lcdspi/jpeg
INC += -I$(TOP)/lib/lcdspi/xpt2046
endif
ifeq ($(MICROPY_PY_LVGL),1)
INC += -Ilvgl
INC += -Ilib/lv_bindings/lvgl
endif

CFLAGS += $(INC) -Wall -std=gnu99 -nostdlib $(CFLAGS_MOD) $(CFLAGS_EXTRA)
CFLAGS += $(COPT)
CFLAGS += -DMPY_RX
CFLAGS += -I$(BOARD_DIR)
CFLAGS += -fomit-frame-pointer
# CFLAGS += -Wno-nonnull-compare

CFLAGS += -D$(MCU_SERIES_UPPER)
CFLAGS += -DCONFIG_$(MCU_SERIES_UPPER)
CFLAGS += -D__RX_LITTLE_ENDIAN__

ifeq ($(USE_DBG_PRINT),1)
CFLAGS += -DUSE_DBG_PRINT
#CFLAGS += -DMICROPY_DEBUG_PRINTERS=debug_printf
endif

ifeq ($(MICROPY_DEBUG_VERBOSE),1)
CFLAGS += -DMICROPY_DEBUG_VERBOSE=1
endif

ifeq ($(MICROPY_DEBUG_PRINTERS),1)
CFLAGS += -DMICROPY_DEBUG_PRINTERS=1
endif

ifeq ($(TEST_RAMDISK),1)
CFLAGS += -DDEBUG_USE_RAMDISK
endif

ifeq ($(USB_MODE),1)
CFLAGS += -DUSB_CDC
endif
ifeq ($(USB_MODE), 2)
CFLAGS += -DUSB_MSC
endif
ifeq ($(USB_MODE), 3)
CFLAGS += -DUSB_CDC_MSC
endif

ifeq ($(MICROPY_PY_LVGL),1)
CFLAGS += -DLVGL_ENABLE
endif

LDFLAGS += -nostdlib -L $(LD_DIR) $(addprefix -T,$(LD_FILES)) -Wl,-Map=$(@:.elf=.map) -Wl,--cref
LDFLAGS += -Wl,--defsym=_estack_reserve=8
LIBS += "$(shell $(CC) $(CFLAGS) -print-libgcc-file-name)"

# Remove uncalled code from the final image.
CFLAGS += -fdata-sections -ffunction-sections
LDFLAGS += -Wl,--gc-sections

# Debugging/Optimization
ifeq ($(DEBUG), 1)
# COPT ?= -Og
CFLAGS += -Os -g
# Disable text compression in debug builds
MICROPY_ROM_TEXT_COMPRESSION = 0
else
ifneq ($(LTO), 1)
CFLAGS += -g  # always include debug info in the ELF, unless LTO is on
endif
COPT ?= -Os -DNDEBUG
endif

# Flags for optional C++ source code
CXXFLAGS += $(filter-out -Wmissing-prototypes -Wold-style-definition -std=gnu99,$(CFLAGS))

# TODO make this common -- shouldn't be using these "private" vars from py.mk
ifneq ($(SRC_CXX)$(SRC_USERMOD_CXX)$(SRC_USERMOD_LIB_CXX),)
LIBSTDCPP_FILE_NAME = "$(shell $(CXX) $(CXXFLAGS) -print-file-name=libstdc++.a)"
LDFLAGS += -L"$(shell dirname $(LIBSTDCPP_FILE_NAME))"
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
	pybthread.c \
	factoryreset.c \
	timer.c \
	led.c \
	pin.c \
	pin_defs_rx.c \
	pin_named_pins.c \
	bufhelper.c \
	i2c.c \
	pyb_i2c.c \
	spi.c \
	pyb_spi.c \
	uart.c \
	usb.c \
	wdt.c \
	eth.c \
	gccollect.c \
	help.c \
	machine_i2c.c \
	machine_spi.c \
	machine_timer.c \
	machine_uart.c \
	modmachine.c \
	modpyb.c \
	modrx.c \
	modutime.c \
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
	accel.c \
	servo.c \
	dac.c \
	adc.c \
	modrxreg.c \
	mpy_file.c \
	pwm.c \
	softtimer.c \
	$(wildcard $(BOARD_DIR)/*.c)

SRC_C += esp8266/base64encode.c
SRC_C += esp8266/esp8266.c
SRC_C += esp8266/hmac.c
SRC_C += esp8266/ntp.c
SRC_C += esp8266/twitter.c
SRC_C += esp8266/urlencode.c
SRC_C += esp8266/wifi.c

ifeq ($(USE_NEW_USBLIB),1)
SRC_C += rx/rx_usb.c
SRC_C += rx/rx_usb_cdc.c
SRC_C += rx/rx_usb_descriptor.c
SRC_C += rx/usblib_hw.c
else
SRC_C += usb/usb_entry.c
SRC_C += usb/usb_cdc.c
SRC_C += usb/usb_hid.c
SRC_C += usb/usb_msc_scsi.c
SRC_C += usb/usb_msc.c
SRC_C += usb/usb_core.c
SRC_C += usb/usb_hal.c
SRC_C += usb/usbdescriptors.c
endif

ifeq ($(TEST_RAMDISK),1)
SRC_C += ramdisk/ram_disk.c
SRC_C += ramdisk/FAT_0000_200.c
SRC_C += ramdisk/FAT_0C00_005.c
SRC_C += ramdisk/FAT_0E00_005.c
SRC_C += ramdisk/FAT_1000_100.c
SRC_C += ramdisk/FAT_5000_041.c
endif

SRC_C += debug/debug_printf.c
SRC_C += debug/vsnprintf.c
SRC_C += debug/division.c

ifeq ($(USE_FONT),1)
SRC_C += rx_font.c
endif
ifeq ($(USE_LCDSPI),1)
SRC_C += rx_lcdspi.c
SRC_C += rx_xpt2046.c
endif

SRC_C += sd/sd.c

SRC_S = rx/$(MCU_SERIES_LOWER)/startup_$(MCU_SERIES_LOWER).S
SRC_S += setjmp.S
SRC_S += gchelper.S

ifeq ($(MICROPY_PY_LWIP),1)
SRC_MOD += lwip_utils/sntp_client.c
endif

SRC_O += \
	$(STARTUP_FILE) \
	$(SYSTEM_FILE)

HAL_SRC_C += \
	rx/$(MCU_SERIES_LOWER)/interrupt_handlers.c \
	rx/$(MCU_SERIES_LOWER)/vector_table.c \
	rx/$(MCU_SERIES_LOWER)/bootstrap.c \
	rx/rx_adc.c \
	rx/rx_dac.c \
	rx/rx_ether.c \
	rx/rx_exti.c \
	rx/rx_flash.c \
	rx/rx_gpio.c \
	rx/rx_i2c.c \
	rx/rx_init.c \
	rx/rx_int.c \
	rx/rx_sci.c \
	rx/rx_pwm.c \
	rx/rx_rtc.c \
	rx/rx_servo.c \
	rx/rx_spi.c \
	rx/rx_timer.c \
	rx/rx_tpu.c \
	rx/rx_utils.c

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
LIB_SRC_C += $(addprefix $(FONT_DIR)/,\
    font.c \
    )
endif

ifeq ($(USE_LCDSPI),1)
LCDSPI_DIR=lib/lcdspi
INC += -I$(TOP)/lib/lcdspi/lcdspi/lcdspi
INC += -I$(TOP)/lib/lcdspi/lcdspi/jpeg
INC += -I$(TOP)/lib/lcdspi/lcdspi/xpt2046
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

ifeq ($(MICROPY_PY_LVGL),1)
SRC_C += lvgl/modlvrx.c
SRC_C += lvgl/driver/modILI9341.c
SRC_C += lvgl/driver/modrtch.c
SRC_C += lvgl/driver/modstmpe610.c
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

define GENERATE_ELF
	$(ECHO) "LINK $(1)"
	$(Q)$(CC) $(LDFLAGS) -o $(1) $(2) $(LDFLAGS_MOD) $(LIBS)
	$(Q)$(SIZE) $(1)
endef

define GENERATE_BIN
	$(ECHO) "GEN $(1)"
	$(Q)$(OBJCOPY) -O binary $(2) $(1)
endef

# $(BUILD)/firmware.elf: $(OBJ) $(LIB_LVGL)
# 	$(ECHO) "LINK $@"
# 	$(Q)$(LD) $(LDFLAGS) -o $@ $^ $(LDFLAGS_MOD) $(LIBS)
# 	$(Q)$(OBJCOPY) -O binary $(BUILD)/firmware.elf  $(BUILD)/firmware.bin
# 	$(Q)$(OBJCOPY) -O srec -I elf32-rx-be-ns $(BUILD)/firmware.elf  $(BUILD)/firmware.mot
# 	$(Q)$(SIZE) $@
#	$(Q)$(OBJDUMP) -S -D $(BUILD)/firmware.elf > $(BUILD)/firmware.s

$(BUILD)/firmware.bin: $(BUILD)/firmware.elf
	$(call GENERATE_BIN,$@,$^)

$(BUILD)/firmware.elf: $(OBJ)  $(LIB_LVGL)
	$(call GENERATE_ELF,$@,$^)

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
.PRECIOUS: $(GEN_PINS_SRC)
$(BUILD)/%_$(BOARD).c $(HEADER_BUILD)/%.h $(HEADER_BUILD)/%_af_const.h $(HEADER_BUILD)/%_af_defs.h $(BUILD)/%_qstr.h: $(BOARD_DIR)/%.csv $(MAKE_PINS) $(AF_FILE) $(PREFIX_FILE) | $(HEADER_BUILD)
	$(ECHO) "GEN $@"
	$(Q)$(PYTHON) $(MAKE_PINS) --board $(BOARD_PINS) --af $(AF_FILE) \
		--prefix $(PREFIX_FILE) --hdr $(GEN_PINS_HDR) \
		--qstr $(GEN_PINS_QSTR) --ad-const $(GEN_PINS_AD_CONST) \
		--af-const $(GEN_PINS_AF_CONST) \
		--af-py $(GEN_PINS_AF_PY) > $(GEN_PINS_SRC)


$(GEN_CDCINF_HEADER): $(GEN_CDCINF_FILE) $(FILE2H)
	$(ECHO) "GEN $@"
	$(Q)$(PYTHON) $(FILE2H) $< > $@

$(GEN_CDCINF_FILE): $(CDCINF_TEMPLATE) $(INSERT_USB_IDS) $(USB_IDS_FILE)
	$(ECHO) "GEN $@"
	$(Q)$(PYTHON) $(INSERT_USB_IDS) $(USB_IDS_FILE) $< > $@

include $(TOP)/py/mkrules.mk
