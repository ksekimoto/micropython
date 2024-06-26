# Select the board to build for: if not given on the command line,
# then default to GR_CITURS
BOARD ?= GR_CITRUS
#BOARD ?= GR_SAKURA
#BOARD ?= GR_ROSE
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
USE_DBG_PRINT = 0

# enable micropython debug flag
#MICROPY_DEBUG_VERBOSE = 1
#MICROPY_DEBUG_PRINTERS = 1

# Use developing usb library instead of stable one for testing
# Use ramdisk instead of CPU internal flash for testing the related codes
#TEST_RAMDISK = 1

# To avoid flash operation during debugging
#FLASH_SKIP = 1

# Display erase and write log
#FLASH_DEBUG = 1

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
#QSTR_GENERATED_HEADERS = $(BUILD)/pins_qstr.h $(BUILD)/modstm_qstr.h
QSTR_GENERATED_HEADERS = $(BUILD)/pins_qstr.h
# qstr definitions (must come before including py.mk)
QSTR_DEFS += qstrdefsport.h $(QSTR_GENERATED_HEADERS)
QSTR_GLOBAL_DEPENDENCIES += mpconfigboard_common.h $(BOARD_DIR)/mpconfigboard.h $(QSTR_GENERATED_HEADERS)

# MicroPython feature configurations
MICROPY_ROM_TEXT_COMPRESSION ?= 1

# File containing description of content to be frozen into firmware.
FROZEN_MANIFEST ?= boards/manifest.py

ifeq ($(MICROPY_PY_LVGL), 1)
LVGL_ENABLE = 1
endif

# include py core make definitions
include $(TOP)/py/py.mk
include $(TOP)/extmod/extmod.mk

GIT_SUBMODULES += lib/lwip lib/mbedtls

GCC_VERSION = "`$(CC) -dumpversion`"
GCC_VERSION_9 = "9.0"
GCC_VERSION_8 = "8.0"
GCC_VERSION_7 = "7.0"
GCC_VERSION_6 = "6.0"
GCC_VERSION_5 = "5.0"

#######################################################################
# MCU configuration
#######################################################################

# Select the cross compile prefix
CROSS_COMPILE ?= rx-elf-

ifeq ($(MCU_SERIES),RX63N)
MCU_SERIES_LOW = rx63n
MCU_SERIES_CAP = RX63N
endif

ifeq ($(MCU_SERIES),RX65N)
MCU_SERIES_LOW = rx65n
MCU_SERIES_CAP = RX65N
endif

#######################################################################
# Compiler configuration
#######################################################################
#GCCDIR = /d/cross/GNURX-ELF-4.8.4.201701/rx-elf/rx-elf
#GCCVER = 4.8.4.201701-GNURX
IS_RENESAS_GCC := $(shell if [ "`rx-elf-gcc --version | grep GNURX`" ]; then echo "GNURX"; fi)
CROSS_COMPILE = rx-elf-

#######################################################################
# C include
#######################################################################
INC += -I.
INC += -I$(TOP)
INC += -I$(BUILD)
INC += -I$(BOARD_DIR)
INC += -Irx
INC += -Irx/$(MCU_SERIES_LOW)
INC += -I$(TOP)/lib/oofatfs
INC += -Idebug
INC += -Iramdisk
INC += -Iusb
#INC += -I$(GCCDIR)/include -I$(GCCDIR)/rx-elf/include
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
INC += -Ilibs/xpt2046
endif
INC += -Isd
ifeq ($(USE_DBG_PRINT) ,1)
INC += -Idebug
endif
ifeq ($(LVGL_ENABLE),1)
INC += -Ilvgl
INC += -Ilib/lv_bindings/lvgl
endif

#######################################################################
# C flags.
#######################################################################
CFLAGS += $(INC) -Wall -Wpointer-arith -Werror -Wdouble-promotion -Wfloat-conversion -std=gnu99 -nostdlib $(CFLAGS_MOD) $(CFLAGS_EXTRA)
CFLAGS += $(COPT)
CFLAGS += -I$(BOARD_DIR)
# CFLAGS += -ffunction-sections -fdata-sections -fomit-frame-pointer -fdollars-in-identifiers -fshort-wchar -fno-exceptions -funsigned-char
CFLAGS += -ffunction-sections -fdata-sections -fomit-frame-pointer 
CFLAGS += -Wno-missing-braces
CFLAGS += -Wno-error=pointer-arith
CFLAGS += -Wno-format
#CFLAGS += -Wno-error=incompatible-pointer-types
#CFLAGS += -Wno-error=tautological-compare
#CFLAGS += -Wno-error=unused-const-variable
#CFLAGS += -Wno-error=nonnull
#CFLAGS += -Wno-error=nonnull-compare
#CFLAGS += -Wno-tautological-compare
#CFLAGS += -Wno-error=unused-function
#CFLAGS += -Wno-unused-function
#CFLAGS += -Wno-error=unused-variable

IS_GCC_ABOVE_VERSION_9 := $(shell expr "$(GCC_VERSION)" ">=" "$(GCC_VERSION_9)")
IS_GCC_ABOVE_VERSION_6 := $(shell expr "$(GCC_VERSION)" ">=" "$(GCC_VERSION_6)")
ifeq "$(IS_GCC_ABOVE_VERSION_9)" "1"
CFLAGS += -Wno-error=unused-const-variable
CFLAGS += -Wno-error=nonnull-compare
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
# timer.c:85
CFLAGS += -Wno-error=unused-const-variable
CFLAGS += -Wno-error=nonnull
CFLAGS += -Wno-error=nonnull-compare
else
# gcc version 4.x or 5.x
# objexcept.c
CFLAGS += -Wno-error=sign-compare
# vm.c
CFLAGS += -Wno-error=override-init
CFLAGS += -Wno-error=main
CFLAGS += -Wno-error=format=
endif
endif

ifeq ($(DEBUG), 1)
#CFLAGS += -Os -ggdb
CFLAGS += -Os -g
#CFLAGS += -Os -g
# Disable text compression in debug builds
MICROPY_ROM_TEXT_COMPRESSION = 0
else
CFLAGS += -Os -DNDEBUG
endif

CFLAGS += -D$(MCU_SERIES_CAP)
CFLAGS += -DCONFIG_$(MCU_SERIES_CAP)
CFLAGS += -D__RX_LITTLE_ENDIAN__
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
CFLAGS += -DFFCONF_H=\"lib/oofatfs/ffconf.h\"

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
#######################################################################
# Linker flags.
#######################################################################
#LINC = $(GCCDIR)/rx-elf/lib
#LINC += $(GCCDIR)/lib/gcc/rx-elf/$(GCCVER)
#LDFLAGS += $(patsubst %,-L%,$(LINC))
LDFLAGS += -g
LDFLAGS += -nostdlib
LDFLAGS += -L boards
# LDFLAGS += -nostartfiles
LDFLAGS += -T $(LD_FILES)
LDFLAGS += -Map=$@.map --cref --defsym=_estack_reserve=8
LDFLAGS += --gc-sections
# print removed sections
#LDFLAGS += -Wl,--print-gc-sections
#LDFLAGS += -Wl,--no-undefined

# LIBS =
LIBS = "$(shell $(CC) $(CFLAGS) -print-libgcc-file-name)"

# Flags for optional C++ source code
CXXFLAGS += $(filter-out -Wmissing-prototypes -Wold-style-definition -std=gnu99,$(CFLAGS))
CXXFLAGS += $(CXXFLAGS_MOD)
ifneq ($(SRC_CXX)$(SRC_MOD_CXX),)
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

SRC_C += main.c
SRC_C += boardctrl.c
SRC_C += template/mymodule.c
SRC_C += accel.c
SRC_C += adc.c
SRC_C += bufhelper.c
SRC_C += dac.c
SRC_C += eth.c
SRC_C += extint.c
SRC_C += factoryreset.c
SRC_C += fatfs_port.c
SRC_C += flash.c
SRC_C += flashbdev.c
SRC_C += gccollect.c
SRC_C += help.c
SRC_C += i2c.c
SRC_C += irq.c
SRC_C += lcd.c
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
SRC_C += modrx.c
SRC_C += modrxreg.c
SRC_C += modutime.c
SRC_C += mphalport.c
SRC_C += mpnetworkport.c
SRC_C += mpthreadport.c
SRC_C += network_lan.c
SRC_C += pendsv.c
SRC_C += phy.c
SRC_C += pin.c
SRC_C += pin_defs_rx.c
SRC_C += pin_named_pins.c
SRC_C += posix_helpers.c
SRC_C += pwm.c
SRC_C += pyb_i2c.c
SRC_C += pyb_spi.c
SRC_C += pybthread.c
SRC_C += rng.c
SRC_C += rtc.c
SRC_C += sdcard.c
SRC_C += servo.c
SRC_C += softtimer.c
SRC_C += spi.c
SRC_C += storage.c
SRC_C += systick.c
SRC_C += timer.c
SRC_C += uart.c
SRC_C += usb.c
SRC_C += usrsw.c
SRC_C += mpy_file.c
SRC_C += wdt.c
#SRC_C += $(BUILD)/_frozen_mpy.c
SRC_C += esp8266/base64encode.c
SRC_C += esp8266/esp8266.c
SRC_C += esp8266/hmac.c
SRC_C += esp8266/ntp.c
#SRC_C += esp8266/qsort.c
SRC_C += esp8266/twitter.c
SRC_C += esp8266/urlencode.c
SRC_C += esp8266/wifi.c
#SRC_C += $(MCU_SERIES_LOW)/sbrk.c
SRC_C += rx/$(MCU_SERIES_LOW)/interrupt_handlers.c
SRC_C += rx/$(MCU_SERIES_LOW)/vector_table.c
SRC_C += rx/$(MCU_SERIES_LOW)/bootstrap.c
SRC_C += rx/rx_adc.c
SRC_C += rx/rx_dac.c
SRC_C += rx/rx_ether.c
SRC_C += rx/rx_exti.c
SRC_C += rx/rx_flash.c
SRC_C += rx/rx_gpio.c
SRC_C += rx/rx_i2c.c
SRC_C += rx/rx_init.c
SRC_C += rx/rx_int.c
SRC_C += rx/rx_sci.c
SRC_C += rx/rx_pwm.c
SRC_C += rx/rx_rtc.c
SRC_C += rx/rx_servo.c
SRC_C += rx/rx_spi.c
SRC_C += rx/rx_timer.c
SRC_C += rx/rx_tpu.c
SRC_C += rx/rx_utils.c
ifeq ($(USE_NEW_USBLIB), 1)
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
ifeq ($(TEST_RAMDISK), 1)
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
ifeq ($(USE_FONT), 1)
SRC_C += libs/font/font.c
SRC_C += libs/rx_font.c
endif
ifeq ($(USE_LCDSPI), 1)
SRC_C += libs/lcdspi/lcdspi_file.c
SRC_C += libs/lcdspi/lcdspi_info.c
SRC_C += libs/lcdspi/lcdspi.c
SRC_C += libs/jpeg/jpeg_disp.c
SRC_C += libs/jpeg/jpeg_src.c
SRC_C += libs/jpeg/jpeg.c
SRC_C += libs/jpeg/picojpeg.c
SRC_C += libs/xpt2046/xpt2046.c
SRC_C += libs/rx_lcdspi.c
SRC_C += libs/rx_xpt2046.c
endif

SRC_C += sd/sd.c
SRC_C += strftime.c
SRC_C += time_utils.c

SRC_S = rx/$(MCU_SERIES_LOW)/startup_$(MCU_SERIES_LOW).S
SRC_S += setjmp.S
SRC_S += gchelper.S

ifeq ($(MICROPY_PY_LWIP),1)
SRC_MOD += lwip_utils/sntp_client.c
endif

SRC_CXX += \
	$(SRC_MOD_CXX)

SRC_O += \
	$(STARTUP_FILE) \
	$(SYSTEM_FILE)

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

ifneq ($(MICROPY_PY_NETWORK_WIZNET5K),0)
WIZNET5K_DIR=drivers/wiznet5k
INC += -I$(TOP)/$(WIZNET5K_DIR)
CFLAGS_MOD += -DMICROPY_PY_WIZNET5K=$(MICROPY_PY_NETWORK_WIZNET5K) -D_WIZCHIP_=$(MICROPY_PY_NETWORK_WIZNET5K)
ifeq ($(MICROPY_PY_LWIP),1)
# When using MACRAW mode (with lwIP), maximum buffer space must be used for the raw socket
CFLAGS_MOD += -DWIZCHIP_USE_MAX_BUFFER
endif
SRC_MOD += network_wiznet5k.c modnwwiznet5k.c
SRC_MOD += $(addprefix $(WIZNET5K_DIR)/,\
	ethernet/w$(MICROPY_PY_NETWORK_WIZNET5K)/w$(MICROPY_PY_NETWORK_WIZNET5K).c \
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
SRC_C += lvgl/modlvrx.c
SRC_C += lvgl/driver/modILI9341.c
SRC_C += lvgl/driver/modrtch.c
SRC_C += lvgl/driver/modstmpe610.c
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

all: $(BUILD)/firmware.elf

#######################################################################
# build rules
#######################################################################

ifneq ($(FROZEN_MANIFEST),)
CFLAGS += -DMICROPY_QSTR_EXTRA_POOL=mp_qstr_frozen_const_pool
CFLAGS += -DMICROPY_MODULE_FROZEN_MPY
CFLAGS += -DMICROPY_MODULE_FROZEN_STR
endif




$(BUILD)/firmware.elf: $(OBJ) $(LIB_LVGL)
	$(ECHO) "LINK $@"
	$(Q)$(LD) $(LDFLAGS) -o $@ $^ $(LDFLAGS_MOD) $(LIBS)
	$(Q)$(OBJCOPY) -O binary $(BUILD)/firmware.elf  $(BUILD)/firmware.bin
	$(Q)$(OBJCOPY) -O srec -I elf32-rx-be-ns $(BUILD)/firmware.elf  $(BUILD)/firmware.mot
	$(Q)$(SIZE) $@
#	$(Q)$(OBJDUMP) -S -D $(BUILD)/firmware.elf > $(BUILD)/firmware.s

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
.PRECIOUS: $(GEN_PINS_SRC)
$(BUILD)/%_$(BOARD).c $(HEADER_BUILD)/%.h $(HEADER_BUILD)/%_af_const.h $(HEADER_BUILD)/%_af_defs.h $(BUILD)/%_qstr.h: $(BOARD_DIR)/%.csv $(MAKE_PINS) $(AF_FILE) $(PREFIX_FILE) | $(HEADER_BUILD)
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
