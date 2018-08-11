# Select the board to build for: if not given on the command line,
# then default to PYBV10.
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
#QSTR_DEFS = qstrdefsport.h $(BUILD)/pins_qstr.h $(BUILD)/modstm_qstr.h
QSTR_DEFS = qstrdefsport.h
#QSTR_GLOBAL_DEPENDENCIES = mpconfigboard_common.h boards/$(BOARD)/mpconfigboard.h
QSTR_GLOBAL_DEPENDENCIES = boards/$(BOARD)/mpconfigboard.h

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
INC += -I$(GCCDIR)/include -I$(GCCDIR)/rx-elf/include

#######################################################################
# C flags.
#######################################################################
CFLAGS += $(INC) -Wall -Werror -std=c99 -nostdlib $(COPT)

ifeq ($(DEBUG), 1)
CFLAGS += -Os -ggdb
else
CFLAGS += -Os -DNDEBUG
endif

CFLAGS += -D$(MCU_SERIES)
CFLAGS += -DCONFIG_$(MCU_SERIES)

#######################################################################
# Linker flags.
#######################################################################
#LINC = $(GCCDIR)/rx-elf/lib
#LINC += $(GCCDIR)/lib/gcc/rx-elf/$(GCCVER)
#LDFLAGS += $(patsubst %,-L%,$(LINC))
LDFLAGS += -g
LFFLAGS += -nostdlib
LDFLAGS += -T $(LD_FILES)
LDFLAGS += -Map=$@.map --cref --gc-sections

LIBS =

SRC_C = main.c
SRC_C += $(MCU_SERIES)/uart_core.c
SRC_C += lib/utils/printf.c
SRC_C += lib/utils/stdout_helpers.c
SRC_C += lib/utils/pyexec.c
SRC_C += lib/libc/string0.c
SRC_C += lib/mp-readline/readline.c
SRC_C += $(BUILD)/_frozen_mpy.c
SRC_C += $(MCU_SERIES)/interrupt_handlers.c
SRC_C += $(MCU_SERIES)/vector_table.c
SRC_C += $(MCU_SERIES)/bootstrap.c
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_init.c
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_sci.c
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_timer.c

SRC_S = $(MCU_SERIES)/startup_$(MCU_SERIES).S
SRC_S += setjmp.S

OBJ = $(PY_CORE_O) $(addprefix $(BUILD)/, $(SRC_C:.c=.o) $(SRC_S:.S=.o))

all: $(BUILD)/firmware.elf

$(BUILD)/_frozen_mpy.c: frozentest.mpy $(BUILD)/genhdr/qstrdefs.generated.h
	$(ECHO) "MISC freezing bytecode"
	$(Q)$(TOP)/tools/mpy-tool.py -f -q $(BUILD)/genhdr/qstrdefs.preprocessed.h -mlongint-impl=none $< > $@

$(BUILD)/firmware.elf: $(OBJ)
	$(ECHO) "LINK $@"
	$(Q)$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)
	$(Q)$(SIZE) $@

#$(BUILD)/firmware.bin: $(BUILD)/firmware.elf
#	$(Q)$(OBJCOPY) -O binary -j .isr_vector -j .text -j .data $^ $(BUILD)/firmware.bin

# Run emulation build on a POSIX system with suitable terminal settings
#run:
#	stty raw opost -echo
#	build/firmware.elf
#	@echo Resetting terminal...
# This sleep is useful to spot segfaults
#	sleep 1
#	reset

#test: $(BUILD)/firmware.elf
#	$(Q)/bin/echo -e "print('hello world!', list(x+1 for x in range(10)), end='eol\\\\n')\\r\\n\\x04" | $(BUILD)/firmware.elf | tail -n2 | grep "^hello world! \\[1, 2, 3, 4, 5, 6, 7, 8, 9, 10\\]eol"

include $(TOP)/py/mkrules.mk
