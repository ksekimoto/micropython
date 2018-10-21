# Select the board to build for: if not given on the command line,
# then default to PYBV10.

#USE_NEW_USBLIB = 1
# Use developing usb library instead of stable one for testing

#TEST_RAMDISK = 1
# Use ramdisk instead of CPU internal flash for testing the related codes

BOARD ?= GR_SAKURA
ifeq ($(wildcard boards/$(BOARD)/.),)
$(error Invalid BOARD specified)
endif

# If the build directory is not given, make it reflect the board name.
BUILD ?= build-$(BOARD)

include ../../py/mkenv.mk
-include mpconfigport.mk
include boards/$(BOARD)/mpconfigboard.mk

# qstr definitions (must come before including py.mk)
QSTR_DEFS = qstrdefsport.h $(BUILD)/pins_qstr.h 
QSTR_GLOBAL_DEPENDENCIES = mpconfigboard_common.h boards/$(BOARD)/mpconfigboard.h

# directory containing scripts to be frozen as bytecode
#FROZEN_MPY_DIR ?= modules

# include py core make definitions
include $(TOP)/py/py.mk

#######################################################################
# Compiler configuration
#######################################################################
GCCDIR = /d/cross/GNURX-ELF-4.8.4.201701/rx-elf/rx-elf
GCCVER = 4.8.4.201701-GNURX
CROSS_COMPILE = rx-elf-

#######################################################################
# C include
#######################################################################
INC += -I.
INC += -I$(TOP)
INC += -I$(BUILD)
INC += -I$(MCU_SERIES)
INC += -Iboards/$(BOARD)
INC += -Idebug
INC += -Iramdisk
INC += -Iusb
INC += -I$(GCCDIR)/include -I$(GCCDIR)/rx-elf/include

#######################################################################
# C flags.
#######################################################################
#CFLAGS += $(INC) -Wall -Werror -std=c99 -nostdlib $(COPT)
CFLAGS += $(INC) -w -std=c99 -nostdlib $(COPT)
#CFLAGS += -fno-leading-underscore

ifeq ($(DEBUG), 1)
#CFLAGS += -Os -ggdb
CFLAGS += -Os -g -DUSE_DBG_PRINT
#CFLAGS += -Os -g
else
CFLAGS += -Os -DNDEBUG
endif

CFLAGS += -D$(MCU_SERIES)
CFLAGS += -DCONFIG_$(MCU_SERIES)
ifeq ($(BOARD),GR_CITRUS)
CFLAGS += -DGRCITRUS
endif
ifeq ($(BOARD),GR_SAKURA)
CFLAGS += -DGRSAKURA
endif
CFLAGS += -DFFCONF_H=\"lib/oofatfs/ffconf.h\"

ifeq ($(TEST_RAMDISK), 1)
CFLAGS += -DDEBUG_USE_RAMDISK
endif

#######################################################################
# Linker flags.
#######################################################################
#LINC = $(GCCDIR)/rx-elf/lib
#LINC += $(GCCDIR)/lib/gcc/rx-elf/$(GCCVER)
#LDFLAGS += $(patsubst %,-L%,$(LINC))
LDFLAGS += -g
#LFFLAGS += -nostdlib
LDFLAGS += -nostartfiles
LDFLAGS += -T $(LD_FILES)
LDFLAGS += -Wl,-Map=$@.map
#LDFLAGS += --cref --gc-sections

LIBS =

SRC_C = main.c
SRC_C += template/mymodule.c
SRC_C += adc.c
SRC_C += bufhelper.c
SRC_C += extint.c
SRC_C += fatfs_port.c
SRC_C += flash.c
SRC_C += flashbdev.c
SRC_C += gccollect.c
SRC_C += help.c
SRC_C += modmachine.c
SRC_C += modpyb.c
SRC_C += moduos.c
#SRC_C += modutime.c
SRC_C += mphalport.c
SRC_C += led.c
SRC_C += pendsv.c
SRC_C += pin.c
SRC_C += pin_defs_rx.c
SRC_C += pin_named_pins.c
SRC_C += sdcard.c
SRC_C += spi.c
SRC_C += storage.c
SRC_C += systick.c
SRC_C += uart.c
#SRC_C += uart_core.c
SRC_C += usrsw.c
#SRC_C += $(BUILD)/_frozen_mpy.c
SRC_C += $(MCU_SERIES)/sbrk.c
SRC_C += $(MCU_SERIES)/interrupt_handlers.c
SRC_C += $(MCU_SERIES)/vector_table.c
SRC_C += $(MCU_SERIES)/bootstrap.c
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_ad.c
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_exti.c
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_flash.c
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_gpio.c
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_init.c
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_sci.c
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_spi.c
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_timer.c
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_utils.c
ifeq ($(USE_NEW_USBLIB), 1)
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_usb.c
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_usb_cdc.c
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_usb_descriptor.c
SRC_C += $(MCU_SERIES)/usblib_hw.c
else
SRC_C += usb/usb_entry.c
SRC_C += usb/usb_cdc.c
SRC_C += usb/usb_msc_scsi.c
SRC_C += usb/usb_msc.c
SRC_C += usb/usb_core.c
SRC_C += usb/usb_hal.c
SRC_C += usb/usbdescriptors.c
endif
ifeq ($(TEST_RAMDISK), 1)
SRC_C += ramdisk/ram_disk.c
SRC_C += ramdisk/FAT_0000_200.c
SRC_C += ramdisk/FAT_0C00_005.c
SRC_C += ramdisk/FAT_0E00_005.c
SRC_C += ramdisk/FAT_1000_100.c
SRC_C += ramdisk/FAT_5000_041.c
endif
#SRC_C += $(MCU_SERIES)/util.c
ifeq ($(DEBUG), 1)
SRC_C += debug/debug_printf.c
SRC_C += debug/vsnprintf.c
endif

SRC_S = $(MCU_SERIES)/startup_$(MCU_SERIES).S
SRC_S += setjmp.S
SRC_S += gchelper.S

SRC_LIB += $(addprefix lib/, libc/string0.c)
SRC_LIB += $(addprefix lib/, mp-readline/readline.c)
SRC_LIB += $(addprefix lib/, oofatfs/ff.c)
SRC_LIB += $(addprefix lib/, oofatfs/option/unicode.c)
SRC_LIB += $(addprefix lib/, timeutils/timeutils.c)
SRC_LIB += $(addprefix lib/, utils/interrupt_char.c)
SRC_LIB += $(addprefix lib/, utils/printf.c)
SRC_LIB += $(addprefix lib/, utils/pyexec.c)
#SRC_LIB += $(addprefix lib/, utils/stdout_helpers.c)
SRC_LIB += $(addprefix lib/, utils/sys_stdio_mphal.c)

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
	sqrt.c \
	tan.c \
	tanh.c \
	tgamma.c \
	trunc.c \
	)
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
SRC_LIBM += lib/libm/ef_sqrt.c

DRIVERS_SRC_C = $(addprefix drivers/,\
	bus/softspi.c \
	bus/softqspi.c \
	)
endif

#OBJ = $(PY_CORE_O)
OBJ += $(PY_O)
OBJ += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_S:.S=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_LIB:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_LIBM:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(EXTMOD_SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(DRIVERS_SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_MOD:.c=.o))
OBJ += $(BUILD)/pins_$(BOARD).o

#ifneq ($(FROZEN_DIR),)
# To use frozen source modules, put your .py files in a subdirectory (eg scripts/)
# and then invoke make with FROZEN_DIR=scripts (be sure to build from scratch).
#CFLAGS += -DMICROPY_MODULE_FROZEN_STR
#endif

#ifneq ($(FROZEN_MPY_DIR),)
# To use frozen bytecode, put your .py files in a subdirectory (eg frozen/) and
# then invoke make with FROZEN_MPY_DIR=frozen (be sure to build from scratch).
#CFLAGS += -DMICROPY_QSTR_EXTRA_POOL=mp_qstr_frozen_const_pool
#CFLAGS += -DMICROPY_MODULE_FROZEN_MPY
#endif

all: $(BUILD)/firmware.elf

#$(BUILD)/_frozen_mpy.c: frozentest.mpy $(BUILD)/genhdr/qstrdefs.generated.h
#	$(ECHO) "MISC freezing bytecode"
#	$(Q)$(TOP)/tools/mpy-tool.py -f -q $(BUILD)/genhdr/qstrdefs.preprocessed.h -mlongint-impl=none $< > $@

$(BUILD)/firmware.elf: $(OBJ)
	$(ECHO) "LINK $@"
	$(Q)$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)
	$(Q)$(SIZE) $@

MAKE_PINS = boards/make-pins.py
BOARD_PINS = boards/$(BOARD)/pins.csv
AF_FILE = boards/$(MCU_SERIES)_af.csv
PREFIX_FILE = boards/$(MCU_SERIES)_prefix.c
GEN_PINS_SRC = $(BUILD)/pins_$(BOARD).c
GEN_PINS_HDR = $(HEADER_BUILD)/pins.h
GEN_PINS_QSTR = $(BUILD)/pins_qstr.h
GEN_PINS_AD_CONST = $(HEADER_BUILD)/pins_ad_const.h
GEN_PINS_AF_CONST = $(HEADER_BUILD)/pins_af_const.h
#GEN_PINS_AF_DEFS = $(HEADER_BUILD)/pins_af_defs.h
GEN_PINS_AF_PY = $(BUILD)/pins_af.py

# List of sources for qstr extraction
SRC_QSTR += $(SRC_C) $(SRC_MOD) $(SRC_LIB) $(EXTMOD_SRC_C)

# Making OBJ use an order-only depenedency on the generated pins.h file
# has the side effect of making the pins.h file before we actually compile
# any of the objects. The normal dependency generation will deal with the
# case when pins.h is modified. But when it doesn't exist, we don't know
# which source files might need it.
$(OBJ): | $(GEN_PINS_HDR)

# With conditional pins, we may need to regenerate qstrdefs.h when config
# options change.
$(HEADER_BUILD)/qstrdefs.generated.h: boards/$(BOARD)/mpconfigboard.h

# main.c can't be even preprocessed without $(GEN_CDCINF_HEADER)
main.c: $(GEN_CDCINF_HEADER)

# Use a pattern rule here so that make will only call make-pins.py once to make
# both pins_$(BOARD).c and pins.h
.PRECIOUS: $(GEN_PINS_SRC)
$(BUILD)/%_$(BOARD).c $(HEADER_BUILD)/%.h $(HEADER_BUILD)/%_ad_const.h $(HEADER_BUILD)/%_af_const.h $(HEADER_BUILD)/%_af_defs.h $(BUILD)/%_qstr.h: boards/$(BOARD)/%.csv $(MAKE_PINS) $(AF_FILE) $(PREFIX_FILE) | $(HEADER_BUILD)
	$(ECHO) "GEN $@"
	$(Q)$(PYTHON) $(MAKE_PINS) --board $(BOARD_PINS) --af $(AF_FILE) --prefix $(PREFIX_FILE) --hdr $(GEN_PINS_HDR) --qstr $(GEN_PINS_QSTR) --ad-const $(GEN_PINS_AD_CONST) --af-const $(GEN_PINS_AF_CONST) --af-py $(GEN_PINS_AF_PY) > $(GEN_PINS_SRC)

#$(BUILD)/pins_$(BOARD).o: $(GEN_PINS_SRC)
#	$(call compile_c)

include $(TOP)/py/mkrules.mk
