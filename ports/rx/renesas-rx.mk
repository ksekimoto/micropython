#
# Makefile fragment for Renesas RX MCUs.
#

# Create variables for the MCU name.

MCU_SERIES_UPPER = $(shell echo $(MCU_SERIES) | tr '[:lower:]' '[:upper:]')
MCU_SERIES_LOWER = $(shell echo $(MCU_SERIES) | tr '[:upper:]' '[:lower:]')

IS_RENESAS_GCC := $(shell if [ "`rx-elf-gcc --version | grep GNURX`" ]; then echo "GNURX"; fi)
ifeq ($(MCU_SERIES),RX63N)
CFLAGS += -mcpu=rx600
endif
ifeq ($(MCU_SERIES),RX65N)
ifeq ($(IS_RENESAS_GCC),GNURX)
CFLAGS += -mcpu=rx64m -fpu
else
CFLAGS += -mcpu=rx600 -Wa,-mcpu=rxv2
endif
endif

ifeq ($(BOARD),GR_CITRUS)
CFLAGS += -DGRCITRUS
endif
ifeq ($(BOARD),GR_CITRUS_DD)
CFLAGS += -DGRCITRUS
endif
ifeq ($(BOARD),GR_SAKURA)
CFLAGS += -DGRSAKURA
endif
ifeq ($(BOARD),GR_SAKURA_DD)
CFLAGS += -DGRSAKURA
endif
ifeq ($(BOARD),GR_ROSE)
CFLAGS += -DGRROSE
endif
ifeq ($(BOARD),GR_ROSE_DD)
CFLAGS += -DGRROSE
endif
ifeq ($(BOARD),GR_ROSE_LV)
CFLAGS += -DGRROSE
endif
ifeq ($(BOARD),GR_ROSE_LV_DD)
CFLAGS += -DGRROSE
endif
