# Select the board to build for: if not given on the command line,
# then default to PYBV10.

#USE_NEW_USBLIB = 1
# Use developing usb library instead of stable one for testing

#TEST_RAMDISK = 1
# Use ramdisk instead of CPU internal flash for testing the related codes

#FLASH_SKIP = 1
# To avoid flash operation during debugging

#FLASH_DEBUG = 1
# Display erase and write log
USE_FONT = 1
USE_LCD = 1

USB_CDC = 1
USB_MSC = 2
USB_CDC_MSC = 3
USB_MODE = 3

#BOARD ?= GR_CITRUS
BOARD ?= GR_SAKURA
ifeq ($(wildcard boards/$(BOARD)/.),)
$(error Invalid BOARD specified)
endif

# If the build directory is not given, make it reflect the board name.
BUILD ?= build-$(BOARD)

FROZEN_DIR = modules

include ../../py/mkenv.mk
-include mpconfigport.mk
include boards/$(BOARD)/mpconfigboard.mk

# qstr definitions (must come before including py.mk)
QSTR_DEFS = qstrdefsport.h $(BUILD)/pins_qstr.h 
QSTR_GLOBAL_DEPENDENCIES = mpconfigboard_common.h boards/$(BOARD)/mpconfigboard.h

# directory containing scripts to be frozen as bytecode
FROZEN_MPY_DIR ?= modules

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
INC += -I$(TOP)/lib/oofatfs
INC += -I$(GCCDIR)/include -I$(GCCDIR)/rx-elf/include
ifeq ($(USE_FONT), 1)
INC += -Ifont
endif
ifeq ($(USE_LCD), 1)
INC += -Ilcd
endif
INC += -Isd

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
ifeq ($(BOARD),GR_CITRUS_DD)
CFLAGS += -DGRCITRUS
endif
ifeq ($(BOARD),GR_SAKURA)
CFLAGS += -DGRSAKURA
endif
ifeq ($(BOARD),GR_SAKURA_DD)
CFLAGS += -DGRSAKURA
endif
CFLAGS += -DFFCONF_H=\"lib/oofatfs/ffconf.h\"

ifeq ($(TEST_RAMDISK), 1)
CFLAGS += -DDEBUG_USE_RAMDISK
endif

ifeq ($(FLASH_SKIP), 1)
CFLAGS += -DDEBUG_FLASH_SKIP
endif

ifeq ($(FLASH_DEBUG), 1)
CFLAGS += -DDEBUG_FLASH_WriteX
CFLAGS += -DDEBUG_FLASH_EraseBlock
endif

#define DEBUG_FLASH_SKIP
#define DEBUG_FLASH_WriteX
#define DEBUG_FLASH_EraseBlock

ifeq ($(USB_MODE), 1)
CFLAGS += -DUSB_CDC
endif
ifeq ($(USB_MODE), 2)
CFLAGS += -DUSB_MSC
endif
ifeq ($(USB_MODE), 3)
CFLAGS += -DUSB_CDC_MSC
endif

#define USB_CDC_MSC
#define USB_CDC
#define USB_MSC

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
SRC_C += accel.c
SRC_C += adc.c
SRC_C += bufhelper.c
SRC_C += dac.c
SRC_C += extint.c
SRC_C += fatfs_port.c
SRC_C += flash.c
SRC_C += flashbdev.c
SRC_C += gccollect.c
SRC_C += help.c
SRC_C += i2c.c
SRC_C += irq.c
SRC_C += machine_i2c.c
SRC_C += modmachine.c
SRC_C += modnetwork.c
#SRC_C += modnwcc3k.c
#SRC_C += modnwwiznet5k.c
SRC_C += modpyb.c
SRC_C += moduos.c
SRC_C += modusocket.c
SRC_C += modutime.c
SRC_C += mphalport.c
SRC_C += mpthreadport.c
SRC_C += lcd.c
SRC_C += led.c
SRC_C += pendsv.c
SRC_C += pin.c
SRC_C += pin_defs_rx.c
SRC_C += pin_named_pins.c
SRC_C += pwm.c
SRC_C += pyb_i2c.c
SRC_C += pybthread.c
SRC_C += rng.c
SRC_C += rtc.c
SRC_C += sdcard.c
SRC_C += servo.c
SRC_C += spi.c
#SRC_C += spibdev.c
SRC_C += storage.c
SRC_C += systick.c
SRC_C += timer.c
SRC_C += uart.c
#SRC_C += uart_core.c
SRC_C += usb.c
SRC_C += usrsw.c
SRC_C += wdt.c
#SRC_C += $(BUILD)/_frozen_mpy.c
SRC_C += esp8266/base64encode.c
SRC_C += esp8266/esp8266.c
SRC_C += esp8266/hmac.c
SRC_C += esp8266/ntp.c
SRC_C += esp8266/qsort.c
SRC_C += esp8266/twitter.c
SRC_C += esp8266/urlencode.c
SRC_C += esp8266/wifi.c
SRC_C += $(MCU_SERIES)/sbrk.c
SRC_C += $(MCU_SERIES)/interrupt_handlers.c
SRC_C += $(MCU_SERIES)/vector_table.c
SRC_C += $(MCU_SERIES)/bootstrap.c
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_adc.c
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_dac.c
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_exti.c
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_flash.c
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_gpio.c
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_i2c.c
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_init.c
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_sci.c
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_pwm.c
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_rtc.c
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_servo.c
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_spi.c
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_timer.c
SRC_C += $(MCU_SERIES)/$(MCU_SERIES)_tpu.c
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
ifeq ($(DEBUG), 1)
SRC_C += debug/debug_printf.c
SRC_C += debug/vsnprintf.c
endif
ifeq ($(USE_FONT), 1)
SRC_C += font/font.c
endif
ifeq ($(USE_LCD), 1)
SRC_C += lcd/lcdspi.c
SRC_C += lcd/jpeg.c
SRC_C += lcd/picojpeg.c
endif
SRC_C += sd/sd.c

SRC_S = $(MCU_SERIES)/startup_$(MCU_SERIES).S
SRC_S += setjmp.S
SRC_S += gchelper.S

SRC_LIB += $(addprefix lib/, libc/string0.c)
SRC_LIB += $(addprefix lib/, mp-readline/readline.c)
SRC_LIB += $(addprefix lib/, netutils/netutils.c)
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

ifeq ($(MICROPY_PY_LWIP),1)
EXTMOD_SRC_C = $(addprefix extmod/,\
	modlwip.c \
	)
endif
EXTMOD_SRC_C += $(addprefix extmod/,\
	modonewire.c \
	)

DRIVERS_SRC_C = $(addprefix drivers/,\
	bus/softspi.c \
	bus/softqspi.c \
	memory/spiflash.c \
	dht/dht.c \
	)

#OBJ = $(PY_CORE_O)
ifneq ($(MICROPY_PY_WIZNET5K),0)
WIZNET5K_DIR=drivers/wiznet5k
INC += -I$(TOP)/$(WIZNET5K_DIR)
CFLAGS_MOD += -DMICROPY_PY_WIZNET5K=$(MICROPY_PY_WIZNET5K) -D_WIZCHIP_=$(MICROPY_PY_WIZNET5K)
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

ifeq ($(MICROPY_PY_LWIP),1)
LWIP_DIR = lib/lwip/src
INC += -I$(TOP)/$(LWIP_DIR)/include -Ilwip_inc
$(BUILD)/$(LWIP_DIR)/core/ipv4/dhcp.o: CFLAGS += -Wno-address
SRC_MOD += $(addprefix $(LWIP_DIR)/,\
	core/def.c \
	core/dns.c \
	core/inet_chksum.c \
	core/init.c \
	core/ip.c \
	core/mem.c \
	core/memp.c \
	core/netif.c \
	core/pbuf.c \
	core/raw.c \
	core/stats.c \
	core/sys.c \
	core/tcp.c \
	core/tcp_in.c \
	core/tcp_out.c \
	core/timeouts.c \
	core/udp.c \
	core/ipv4/autoip.c \
	core/ipv4/dhcp.c \
	core/ipv4/etharp.c \
	core/ipv4/icmp.c \
	core/ipv4/igmp.c \
	core/ipv4/ip4_addr.c \
	core/ipv4/ip4.c \
	core/ipv4/ip4_frag.c \
	core/ipv6/dhcp6.c \
	core/ipv6/ethip6.c \
	core/ipv6/icmp6.c \
	core/ipv6/inet6.c \
	core/ipv6/ip6_addr.c \
	core/ipv6/ip6.c \
	core/ipv6/ip6_frag.c \
	core/ipv6/mld6.c \
	core/ipv6/nd6.c \
	netif/ethernet.c \
	)
endif

OBJ =
OBJ += $(PY_O)
OBJ += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_S:.S=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_LIB:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_LIBM:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(EXTMOD_SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(DRIVERS_SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_O))
OBJ += $(addprefix $(BUILD)/, $(SRC_MOD:.c=.o))
OBJ += $(BUILD)/pins_$(BOARD).o

#ifneq ($(FROZEN_DIR),)
# To use frozen source modules, put your .py files in a subdirectory (eg scripts/)
# and then invoke make with FROZEN_DIR=scripts (be sure to build from scratch).
CFLAGS += -DMICROPY_MODULE_FROZEN_STR
endif

ifneq ($(FROZEN_MPY_DIR),)
# To use frozen bytecode, put your .py files in a subdirectory (eg frozen/) and
# then invoke make with FROZEN_MPY_DIR=frozen (be sure to build from scratch).
CFLAGS += -DMICROPY_QSTR_EXTRA_POOL=mp_qstr_frozen_const_pool
CFLAGS += -DMICROPY_MODULE_FROZEN_MPY
endif

all: $(BUILD)/firmware.elf

#$(BUILD)/_frozen_mpy.c: frozentest.mpy $(BUILD)/genhdr/qstrdefs.generated.h
#	$(ECHO) "MISC freezing bytecode"
#	$(Q)$(TOP)/tools/mpy-tool.py -f -q $(BUILD)/genhdr/qstrdefs.preprocessed.h -mlongint-impl=none $< > $@

$(BUILD)/firmware.elf: $(OBJ)
	$(ECHO) "LINK $@"
	$(Q)$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)
	$(Q)$(OBJCOPY) -O binary $(BUILD)/firmware.elf  $(BUILD)/firmware.bin
	$(Q)$(OBJCOPY) -O srec -I elf32-rx-be-ns $(BUILD)/firmware.elf  $(BUILD)/firmware.mot	
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

$(GEN_CDCINF_HEADER): $(GEN_CDCINF_FILE) $(FILE2H)
	$(ECHO) "GEN $@"
	$(Q)$(PYTHON) $(FILE2H) $< > $@

$(GEN_CDCINF_FILE): $(CDCINF_TEMPLATE) $(INSERT_USB_IDS) $(USB_IDS_FILE)
	$(ECHO) "GEN $@"
	$(Q)$(PYTHON) $(INSERT_USB_IDS) $(USB_IDS_FILE) $< > $@

include $(TOP)/py/mkrules.mk
