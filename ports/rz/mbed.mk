#######################################################################
# MBED library build
#######################################################################
BUILD_MBED = $(BUILD)_mbed
BUILD_MBED_LIB = $(BUILD_MBED)_lib
LIB_MBED_HAL = $(BUILD_MBED_LIB)/libmbedhal.a
LIB_MBED = $(BUILD_MBED_LIB)/libmbed.a

#######################################################################
# MBED Include
#######################################################################
INC += -Imbed

ifeq ($(TARGET), TARGET_RZ_A2XX)
INC_MBED_HAL += -ITARGET_RENESAS
INC_MBED_HAL += -ITARGET_RENESAS/$(TARGET)
INC_MBED_HAL += -ITARGET_RENESAS/$(TARGET)/common
INC_MBED_HAL += -ITARGET_RENESAS/$(TARGET)/common/r_cache
INC_MBED_HAL += -ITARGET_RENESAS/$(TARGET)/common/r_cache/inc
INC_MBED_HAL += -ITARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)
INC_MBED_HAL += -ITARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device
INC_MBED_HAL += -ITARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/inc
INC_MBED_HAL += -ITARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/inc/iodefine
INC_MBED_HAL += -ITARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/inc/iodefine/iobitmasks
INC_MBED_HAL += -ITARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/inc/iodefine/iodefines
INC_MBED_HAL += -I$(TOP)/lib/mbed-os
INC_MBED_HAL += -I$(TOP)/lib/mbed-os/hal/include
INC_MBED_HAL += -I$(TOP)/lib/mbed-os/hal/include/hal
INC_MBED_HAL += -I$(TOP)/lib/mbed-os/drivers/include
INC_MBED_HAL += -I$(TOP)/lib/mbed-os/drivers/include/drivers
INC_MBED_HAL += -I$(TOP)/lib/mbed-os/drivers/include/drivers/internal
INC_MBED_HAL += -I$(TOP)/lib/mbed-os/platform/include
INC_MBED_HAL += -I$(TOP)/lib/mbed-os/platform/include/platform
INC_MBED_HAL += -I$(TOP)/lib/mbed-os/platform/include/platform/internal
INC_MBED_HAL += -I$(TOP)/lib/mbed-os/cmsis/CMSIS_5/CMSIS/TARGET_CORTEX_A/Include
endif

ifeq ($(MBED_GR_LIBS_components_CAMERA), 1)
INC_MBED_HAL += -I$(TOP)/lib/mbed-gr-libs/EasyAttach_CameraAndLCD
endif
ifeq ($(MBED_GR_LIBS_GR-PEACH_video), 1)
INC_MBED_HAL += -I$(TOP)/lib/mbed-gr-libs/GR-PEACH_video
endif

ifeq ($(MBED_SD_WRAPPER),1)
INC_MBED_HAL += -I$(TOP)/lib/mbed-gr-libs/SDBlockDevice_GRBoard/$(TARGET)/sdhi-driver
INC_MBED_HAL += -I$(TOP)/lib/mbed-gr-libs/SDBlockDevice_GRBoard/$(TARGET)/sdhi-driver/r_sdhi_simplified/inc
INC_MBED_HAL += -I$(TOP)/lib/mbed-gr-libs/SDBlockDevice_GRBoard/$(TARGET)/sdhi-driver/r_sdhi_simplified/src/sd/inc
INC_MBED_HAL += -I$(TOP)/lib/mbed-gr-libs/SDBlockDevice_GRBoard/$(TARGET)/sdhi-driver/r_sdhi_simplified/src/sd/inc/access
INC_MBED_HAL += -I$(TOP)/lib/mbed-gr-libs/SDBlockDevice_GRBoard/$(TARGET)/sdhi-driver/r_sdhi_simplified/userdef
endif

ifeq ($(MBED_OS_EMAC),1)
INC_MBED_HAL += emac-drivers/TARGET_RZ_A2_EMAC
INC_MBED_HAL += emac-drivers/TARGET_RZ_A2_EMAC/r_ether_rza2/
INC_MBED_HAL += emac-drivers/TARGET_RZ_A2_EMAC/r_ether_rza2/src
INC_MBED_HAL += emac-drivers/TARGET_RZ_A2_EMAC/r_ether_rza2/src/phy
INC_MBED_HAL += emac-drivers/TARGET_RZ_A2_EMAC/r_ether_rza2/src/targets/TARGET_GR_MANGO
endif

INC += $(INC_MBED_HAL)

#######################################################################
# MBED CFLAGS
#######################################################################

CFLAGS_MBED += $(C_CPP_COMMON)
CFLAGS_MBED += -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -fmessage-length=0
CFLAGS_MBED += -std=gnu99 -nostdlib
CFLAGS_MBED += -DMBED_DEBUG -DMBED_TRAP_ERRORS_ENABLED=1 

CPPFLAGS_MBED += $(C_CPP_COMMON)
#CPPFLAGS_MBED += -std=gnu++98 -fno-rtti -Wvla -c 
CPPFLAGS_MBED += -std=gnu++14 -fno-rtti -Wvla -c 
CPPFLAGS_MBED += -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -fmessage-length=0
CPPFLAGS_MBED += -DMBED_DEBUG -DMBED_TRAP_ERRORS_ENABLED=1 

ifeq ($(MBED_SD_WRAPPER),1)
MBED_GR_LIBS_SDBlockDevice_GRBoard = 1
CFLAGS_COMMON += -DMBED_SD_WRAPPER
endif

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

CFLAGS_MBED_COMMON += $(INC_MBED_HAL) -DAPPLICATION_ADDR=0x50000000 -DAPPLICATION_SIZE=0x1000000 -DMBED_ROM_START=0x50000000 -DMBED_ROM_SIZE=0x1000000 -DMBED_RAM_START=0x400000 -DMBED_RAM_SIZE=0x2000000 -DARM_MATH_CA9 -D__FPU_PRESENT -DDEVICE_EMAC=1 -D__MBED__=1 -DDEVICE_USBDEVICE=1 -DTARGET_LIKE_MBED -DDEVICE_PORTINOUT=1 -DMBED_BUILD_TIMESTAMP=1582947353.33 -DCOMPONENT_FLASHIAP=1 -DCOMPONENT_PSA_SRV_EMUL=1 -DDEVICE_SERIAL_ASYNCH=1 -D__CMSIS_RTOS -D__EVAL -DDEVICE_SPISLAVE=1 -DTOOLCHAIN_GCC -DTARGET_CORTEX_A -DDEVICE_I2C_ASYNCH=1 -DTARGET_DEBUG -DDEVICE_RTC=1 -D__MBED_CMSIS_RTOS_CA9 -DCOMPONENT_PSA_SRV_IMPL=1 -DTARGET_LIKE_CORTEX_A9 -DDEVICE_PWMOUT=1 -DDEVICE_SPI_ASYNCH=1 -DDEVICE_INTERRUPTIN=1 -DTARGET_CORTEX -DDEVICE_I2C=1 -DDEVICE_PORTOUT=1 -DDEVICE_I2CSLAVE=1 -DDEVICE_USTICKER=1 -DDEVICE_STDIO_MESSAGES=1 -DTARGET_MBRZA2M -DTARGET_RENESAS -DTARGET_RZA2M -DTARGET_NAME=GR_MANGO -DCOMPONENT_NSPE=1 -DDEVICE_SERIAL_FC=1 -DTARGET_A9 -DTARGET_RZ_A2XX -D__CORTEX_A9 -DDEVICE_PORTIN=1 -DDEVICE_SLEEP=1 -DTOOLCHAIN_GCC_ARM -DDEVICE_SPI=1 -DTARGET_GR_MANGO -DDEVICE_ANALOGIN=1 -DDEVICE_SERIAL=1 -DDEVICE_FLASH=1 -DMBED_CONF_APP_MAIN_STACK_SIZE=8192 -DTARGET_RZ_A2_EMAC
#CFLAGS_MBED_COMMON += -DMBEDTLS_NO_DEFAULT_ENTROPY_SOURCES -DMBEDTLS_USER_CONFIG_FILE="./mbedtls_entropy_config.h" -DMBEDTLS_TEST_NULL_ENTROPY
CFLAGS_MBED_COMMON += -DMBED_CONF_PLATFORM_CRASH_CAPTURE_ENABLED=0 
CFLAGS_MBED_COMMON += -DMBED_CONF_PLATFORM_CTHUNK_COUNT_MAX=8
CFLAGS_MBED_COMMON += -DMBED_CONF_PLATFORM_DEFAULT_SERIAL_BAUD_RATE=115200
CFLAGS_MBED_COMMON += -DMBED_CONF_PLATFORM_ERROR_ALL_THREADS_INFO=0
CFLAGS_MBED_COMMON += -DMBED_CONF_PLATFORM_ERROR_FILENAME_CAPTURE_ENABLED=0
CFLAGS_MBED_COMMON += -DMBED_CONF_PLATFORM_ERROR_HIST_ENABLED=0
CFLAGS_MBED_COMMON += -DMBED_CONF_PLATFORM_ERROR_HIST_SIZE=4
CFLAGS_MBED_COMMON += -DMBED_CONF_PLATFORM_ERROR_REBOOT_MAX=1
CFLAGS_MBED_COMMON += -DMBED_CONF_PLATFORM_FATAL_ERROR_AUTO_REBOOT_ENABLED=0
CFLAGS_MBED_COMMON += -DMBED_CONF_PLATFORM_FORCE_NON_COPYABLE_ERROR=0
CFLAGS_MBED_COMMON += -DMBED_CONF_PLATFORM_MAX_ERROR_FILENAME_LEN=16
CFLAGS_MBED_COMMON += -DMBED_CONF_PLATFORM_POLL_USE_LOWPOWER_TIMER=0
CFLAGS_MBED_COMMON += -DMBED_CONF_PLATFORM_STDIO_BAUD_RATE=115200
CFLAGS_MBED_COMMON += -DMBED_CONF_PLATFORM_STDIO_BUFFERED_SERIAL=0
CFLAGS_MBED_COMMON += -DMBED_CONF_PLATFORM_STDIO_CONVERT_NEWLINES=1
CFLAGS_MBED_COMMON += -DMBED_CONF_PLATFORM_STDIO_CONVERT_TTY_NEWLINE=0
CFLAGS_MBED_COMMON += -DMBED_CONF_PLATFORM_STDIO_FLUSH_AT_EXIT=1
CFLAGS_MBED_COMMON += -DMBED_CONF_PLATFORM_USE_MPU=1
ifeq ($(RTOS_LARGE_PARAM), 1)
CFLAGS_MBED_COMMON += -DMBED_CONF_RTOS_IDLE_THREAD_STACK_SIZE=1024
CFLAGS_MBED_COMMON += -DMBED_CONF_RTOS_IDLE_THREAD_STACK_SIZE_DEBUG_EXTRA=0
CFLAGS_MBED_COMMON += -DMBED_CONF_RTOS_IDLE_THREAD_STACK_SIZE_TICKLESS_EXTRA=512
CFLAGS_MBED_COMMON += -DMBED_CONF_RTOS_MAIN_THREAD_STACK_SIZE=8192
CFLAGS_MBED_COMMON += -DMBED_CONF_RTOS_PRESENT=1
CFLAGS_MBED_COMMON += -DMBED_CONF_RTOS_THREAD_STACK_SIZE=8192
CFLAGS_MBED_COMMON += -DMBED_CONF_RTOS_TIMER_THREAD_STACK_SIZE=1536
CFLAGS_MBED_COMMON += -DMBED_CONF_TARGET_BOOT_STACK_SIZE=0x800
else
CFLAGS_MBED_COMMON += -DMBED_CONF_RTOS_IDLE_THREAD_STACK_SIZE=512
CFLAGS_MBED_COMMON += -DMBED_CONF_RTOS_IDLE_THREAD_STACK_SIZE_DEBUG_EXTRA=0
CFLAGS_MBED_COMMON += -DMBED_CONF_RTOS_IDLE_THREAD_STACK_SIZE_TICKLESS_EXTRA=256
CFLAGS_MBED_COMMON += -DMBED_CONF_RTOS_MAIN_THREAD_STACK_SIZE=4096
CFLAGS_MBED_COMMON += -DMBED_CONF_RTOS_PRESENT=1
CFLAGS_MBED_COMMON += -DMBED_CONF_RTOS_THREAD_STACK_SIZE=4096
CFLAGS_MBED_COMMON += -DMBED_CONF_RTOS_TIMER_THREAD_STACK_SIZE=768
CFLAGS_MBED_COMMON += -DMBED_CONF_TARGET_BOOT_STACK_SIZE=0x400
endif
CFLAGS_MBED_COMMON += -DMBED_CONF_TARGET_DEFAULT_ADC_VREF=3.3
CFLAGS_MBED_COMMON += -DMBED_CRC_TABLE_SIZE=16
CFLAGS_MBED_COMMON += -DMBED_CONF_TARGET_DEEP_SLEEP_LATENCY=0
CFLAGS_MBED_COMMON += -D_RTE_
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
CFLAGS_MBED_COMMON += -DMBED_CONF_APP_LCD=1
CFLAGS_MBED_COMMON += -DMBED_CONF_APP_CAMERA=1
CFLAGS_MBED_COMMON += -DCAMERA_MODULE=MODULE_VDC
CFLAGS_MBED_COMMON += -DMBED_CONF_APP_CAMERA_TYPE=CAMERA_OV7725
CFLAGS_MBED_COMMON += -DMBED_GR_LIBS=1
CFLAGS_MBED_COMMON += -DMBED_GR_LIBS_DRP_FOR_MBED=1
CFLAGS_MBED_COMMON += -DMBED_GR_LIBS_components_LCD=1
CFLAGS_MBED_COMMON += -DMBED_GR_LIBS_components_CAMERA=1
CFLAGS_MBED_COMMON += -DMBED_GR_LIBS_EasyAttach_CameraAndLCD=1
endif

ifeq ($(MBED_GR_LIBS_LCD_TYPE), RGB_TO_HDMI)
CFLAGS_MBED_COMMON += -DMBED_CONF_APP_LCD_TYPE=RGB_TO_HDMI
endif

ifeq ($(MBED_GR_LIBS_LCD_TYPE), ATM0430D25)
CFLAGS_MBED_COMMON += -DMBED_CONF_APP_LCD_TYPE=ATM0430D25
endif

ifeq ($(MBED_OS_EMAC),1)
CFLAGS_COMMON += -DMBED_OS_EMAC
CFLAGS_MBED_COMMON += -DMBED_CONF_EVENTS_SHARED_EVENTSIZE=10
endif

ifeq ($(MBED_OS_EMAC),1)
endif

ifeq "$(IS_GCC_ABOVE_VERSION_9)" "1"
# gcc version 9.x
else 
ifeq "$(IS_GCC_ABOVE_VERSION_6)" "1"
# gcc version 6.x, 7.x or 8.x
else
# gcc version 4.x or 5.x
# analogin_api.c
#CFLAGS += -Wno-error=unused-variable
# i2c_api.c
#CFLAGS += -Wno-error=double-promotion
# pwmout_api.c
#CFLAGS += -Wno-error=float-conversion
# serial_api.c
#CFLAGS += -Wno-error=type-limits
# spi_api.c
#CFLAGS += -Wno-error=overflow
#CFLAGS += -Wno-error=unused-but-set-variable
# rtx_core_ca.h
#CFLAGS += -Wno-error=attributes
# r_mipi_api.c
#CFLAGS += -Wno-error=unused-function
# rz_a2m_evb_vdc.c
#CFLAGS += -Wno-error=aggressive-loop-optimizations
endif
endif

#######################################################################
# TARGET_RENESAS
#######################################################################
SRC_MBED_HAL_S += TARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/TOOLCHAIN_GCC_ARM/startup_RZA2M.S
SRC_MBED_HAL_S += TARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/TOOLCHAIN_GCC_ARM/weak_handler.S

ifeq ($(OVERRIDE_CONSOLE_USBSERIAL), 1)
SRC_MBED_CPP += TARGET_RENESAS/$(TARGET)/common/target_override_console.cpp
endif

SRC_MBED_HAL_C += TARGET_RENESAS/$(TARGET)/analogin_api.c
SRC_MBED_HAL_C += TARGET_RENESAS/$(TARGET)/flash_api.c
SRC_MBED_HAL_C += TARGET_RENESAS/$(TARGET)/gpio_api.c
SRC_MBED_HAL_C += TARGET_RENESAS/$(TARGET)/gpio_irq_api.c
SRC_MBED_HAL_C += TARGET_RENESAS/$(TARGET)/i2c_api.c
SRC_MBED_HAL_C += TARGET_RENESAS/$(TARGET)/pinmap.c
SRC_MBED_HAL_C += TARGET_RENESAS/$(TARGET)/port_api.c
SRC_MBED_HAL_C += TARGET_RENESAS/$(TARGET)/pwmout_api.c
SRC_MBED_HAL_C += TARGET_RENESAS/$(TARGET)/rtc_api.c
SRC_MBED_HAL_C += TARGET_RENESAS/$(TARGET)/serial_api.c
SRC_MBED_HAL_C += TARGET_RENESAS/$(TARGET)/sleep.c
SRC_MBED_HAL_C += TARGET_RENESAS/$(TARGET)/spi_api.c
SRC_MBED_HAL_C += TARGET_RENESAS/$(TARGET)/us_ticker.c
SRC_MBED_HAL_CPP += TARGET_RENESAS/$(TARGET)/USBPhy_RZ_A2.cpp
SRC_MBED_HAL_C += TARGET_RENESAS/$(TARGET)/common/rza_io_regrw.c
SRC_MBED_HAL_C += TARGET_RENESAS/$(TARGET)/common/r_cache/src/lld/r_cache_lld_rza2m.c
SRC_MBED_HAL_C += TARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/PeripheralPins.c
SRC_MBED_HAL_C += TARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/cmsis_nvic.c
SRC_MBED_HAL_C += TARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/mbed_sf_boot.c
SRC_MBED_HAL_C += TARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/mmu_RZ_A2M.c
SRC_MBED_HAL_C += TARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/nvic_wrapper.c
SRC_MBED_HAL_C += TARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/octaram_init.c
SRC_MBED_HAL_C += TARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/os_tick_ostm.c
SRC_MBED_HAL_C += TARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/RZ_A2_Init.c
SRC_MBED_HAL_C += TARGET_RENESAS/$(TARGET)/$(TARGET_BOARD)/device/system_RZ_A2M.c

ifeq ($(MBED_OS), 1)
#######################################################################
# mbed-os cmsis
#######################################################################
SRC_MBED_C += lib/mbed-os/cmsis/CMSIS_5/CMSIS/TARGET_CORTEX_A/Source/irq_ctrl_gic.c
#######################################################################
# mbed-os dirvers
#######################################################################
SRC_MBED_CPP += lib/mbed-os/drivers/source/AnalogIn.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/AnalogOut.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/BufferedSerial.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/BusIn.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/BusInOut.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/BusOut.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/CAN.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/DigitalIn.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/DigitalInOut.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/DigitalOut.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/FlashIAP.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/I2C.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/I2CSlave.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/InterruptIn.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/MbedCRC.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/PortIn.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/PortInOut.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/PortOut.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/PwmOut.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/QSPI.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/ResetReason.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/SerialBase.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/SerialWireOutput.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/SFDP.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/SPI.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/SPISlave.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/Ticker.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/Timeout.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/Timer.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/TimerEvent.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/UnbufferedSerial.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/Watchdog.cpp
#######################################################################
# mbed-os events
#######################################################################
ifeq ($(MBED_OS_EVENTS), 1)
SRC_MBED_CPP += lib/mbed-os/events/source/EventQueue.cpp
SRC_MBED_CPP += lib/mbed-os/events/source/mbed_shared_queues.cpp
SRC_MBED_CPP += lib/mbed-os/events/source/equeue_mbed.cpp
SRC_MBED_C += lib/mbed-os/events/source/equeue_posix.c
SRC_MBED_C += lib/mbed-os/events/source/equeue.c
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
SRC_MBED_CPP += lib/mbed-os/hal/source/LowPowerTickerWrapper.cpp
SRC_MBED_C += lib/mbed-os/hal/source/mbed_critical_section_api.c
SRC_MBED_C += lib/mbed-os/hal/source/mbed_flash_api.c
SRC_MBED_C += lib/mbed-os/hal/source/mbed_gpio.c
SRC_MBED_C += lib/mbed-os/hal/source/mbed_gpio_irq.c
SRC_MBED_C += lib/mbed-os/hal/source/mbed_itm_api.c
SRC_MBED_C += lib/mbed-os/hal/source/mbed_lp_ticker_api.c
SRC_MBED_CPP += lib/mbed-os/hal/source/mbed_lp_ticker_wrapper.cpp
SRC_MBED_C += lib/mbed-os/hal/source/mbed_pinmap_common.c
SRC_MBED_C += lib/mbed-os/hal/source/mbed_pinmap_default.c
SRC_MBED_C += lib/mbed-os/hal/source/mbed_ticker_api.c
SRC_MBED_C += lib/mbed-os/hal/source/mbed_us_ticker_api.c
SRC_MBED_CPP += lib/mbed-os/hal/source/static_pinmap.cpp
SRC_MBED_C += lib/mbed-os/hal/source/mpu/mbed_mpu_v7m.c
SRC_MBED_C += lib/mbed-os/hal/source/mpu/mbed_mpu_v8m.c
#SRC_MBED_C += lib/mbed-os/hal/TARGET_FLASH_CMSIS_ALGO/flash_common_algo.c
#######################################################################
# mbed-os platform (29 files)
#######################################################################
SRC_MBED_CPP += lib/mbed-os/platform/source/ATCmdParser.cpp
SRC_MBED_CPP += lib/mbed-os/platform/source/CriticalSectionLock.cpp
SRC_MBED_CPP += lib/mbed-os/platform/source/CThunkBase.cpp
SRC_MBED_CPP += lib/mbed-os/platform/source/DeepSleepLock.cpp
SRC_MBED_CPP += lib/mbed-os/platform/source/FileBase.cpp
SRC_MBED_CPP += lib/mbed-os/platform/source/FileHandle.cpp
SRC_MBED_CPP += lib/mbed-os/platform/source/FilePath.cpp
SRC_MBED_CPP += lib/mbed-os/platform/source/FileSystemHandle.cpp
SRC_MBED_CPP += lib/mbed-os/platform/source/LocalFileSystem.cpp
SRC_MBED_CPP += lib/mbed-os/platform/source/mbed_alloc_wrappers.cpp
SRC_MBED_C += lib/mbed-os/platform/source/mbed_application.c
SRC_MBED_C += lib/mbed-os/platform/source/mbed_assert.c
SRC_MBED_C += lib/mbed-os/platform/source/mbed_atomic_impl.c
SRC_MBED_C += lib/mbed-os/platform/source/mbed_board.c
SRC_MBED_C += lib/mbed-os/platform/source/mbed_critical.c
SRC_MBED_C += lib/mbed-os/platform/source/mbed_error.c
SRC_MBED_C += lib/mbed-os/platform/source/mbed_error_hist.c
SRC_MBED_C += lib/mbed-os/platform/source/mbed_interface.c
SRC_MBED_CPP += lib/mbed-os/platform/source/mbed_mem_trace.cpp
SRC_MBED_C += lib/mbed-os/platform/source/mbed_mktime.c
SRC_MBED_C += lib/mbed-os/platform/source/mbed_mpu_mgmt.c
SRC_MBED_CPP += lib/mbed-os/platform/source/mbed_os_timer.cpp
SRC_MBED_CPP += lib/mbed-os/platform/source/mbed_poll.cpp
SRC_MBED_C += lib/mbed-os/platform/source/mbed_power_mgmt.c
SRC_MBED_CPP += lib/mbed-os/platform/source/mbed_retarget.cpp
SRC_MBED_CPP += lib/mbed-os/platform/source/mbed_rtc_time.cpp
SRC_MBED_C += lib/mbed-os/platform/source/mbed_sdk_boot.c
SRC_MBED_C += lib/mbed-os/platform/source/mbed_semihost_api.c
SRC_MBED_C += lib/mbed-os/platform/source/mbed_stats.c
SRC_MBED_CPP += lib/mbed-os/platform/source/mbed_thread.cpp
SRC_MBED_C += lib/mbed-os/platform/source/mbed_wait_api_no_rtos.c
SRC_MBED_CPP += lib/mbed-os/platform/source/Stream.cpp
SRC_MBED_CPP += lib/mbed-os/platform/source/SysTimer.cpp
#######################################################################
# mbed-os rtos
#######################################################################
ifeq ($(MBED_OS_NO_RTOS), 1)
else
SRC_MBED_CPP += lib/mbed-os/rtos/source/ConditionVariable.cpp
SRC_MBED_CPP += lib/mbed-os/rtos/source/EventFlags.cpp
SRC_MBED_CPP += lib/mbed-os/rtos/source/Kernel.cpp
SRC_MBED_CPP += lib/mbed-os/rtos/source/Mutex.cpp
SRC_MBED_CPP += lib/mbed-os/rtos/source/Semaphore.cpp
SRC_MBED_CPP += lib/mbed-os/rtos/source/ThisThread.cpp
SRC_MBED_CPP += lib/mbed-os/rtos/source/Thread.cpp
SRC_MBED_C += lib/mbed-os/cmsis/device/rtos/source/mbed_boot.c
SRC_MBED_C += lib/mbed-os/cmsis/device/rtos/source/mbed_rtos_rtx.c
SRC_MBED_C += lib/mbed-os/cmsis/device/rtos/source/mbed_rtx_handlers.c
SRC_MBED_CPP += lib/mbed-os/cmsis/device/rtos/source/mbed_rtx_idle.cpp
SRC_MBED_C += lib/mbed-os/cmsis/CMSIS_5/CMSIS/RTOS2/RTX/Library/cmsis_os1.c
SRC_MBED_C += lib/mbed-os/cmsis/CMSIS_5/CMSIS/RTOS2/RTX/Config/RTX_Config.c
SRC_MBED_C += lib/mbed-os/cmsis/CMSIS_5/CMSIS/RTOS2/RTX/Config/TARGET_CORTEX_A/handlers.c
SRC_MBED_C += lib/mbed-os/cmsis/CMSIS_5/CMSIS/RTOS2/RTX/Source/rtx_delay.c
SRC_MBED_C += lib/mbed-os/cmsis/CMSIS_5/CMSIS/RTOS2/RTX/Source/rtx_evflags.c
SRC_MBED_C += lib/mbed-os/cmsis/CMSIS_5/CMSIS/RTOS2/RTX/Source/rtx_evr.c
SRC_MBED_C += lib/mbed-os/cmsis/CMSIS_5/CMSIS/RTOS2/RTX/Source/rtx_kernel.c
SRC_MBED_C += lib/mbed-os/cmsis/CMSIS_5/CMSIS/RTOS2/RTX/Source/rtx_lib.c
SRC_MBED_C += lib/mbed-os/cmsis/CMSIS_5/CMSIS/RTOS2/RTX/Source/rtx_memory.c
SRC_MBED_C += lib/mbed-os/cmsis/CMSIS_5/CMSIS/RTOS2/RTX/Source/rtx_mempool.c
SRC_MBED_C += lib/mbed-os/cmsis/CMSIS_5/CMSIS/RTOS2/RTX/Source/rtx_msgqueue.c
SRC_MBED_C += lib/mbed-os/cmsis/CMSIS_5/CMSIS/RTOS2/RTX/Source/rtx_mutex.c
SRC_MBED_C += lib/mbed-os/cmsis/CMSIS_5/CMSIS/RTOS2/RTX/Source/rtx_semaphore.c
SRC_MBED_C += lib/mbed-os/cmsis/CMSIS_5/CMSIS/RTOS2/RTX/Source/rtx_system.c
SRC_MBED_C += lib/mbed-os/cmsis/CMSIS_5/CMSIS/RTOS2/RTX/Source/rtx_thread.c
SRC_MBED_C += lib/mbed-os/cmsis/CMSIS_5/CMSIS/RTOS2/RTX/Source/rtx_timer.c
SRC_MBED_S += lib/mbed-os/cmsis/CMSIS_5/CMSIS/RTOS2/RTX/Source/TOOLCHAIN_GCC/TARGET_CORTEX_A/irq_ca.S
SRC_MBED_C += lib/mbed-os/cmsis/CMSIS_5/CMSIS/RTOS2/Source/os_systick.c
SRC_MBED_C += lib/mbed-os/cmsis/CMSIS_5/CMSIS/RTOS2/Source/os_tick_ptim.c
SRC_MBED_C += lib/mbed-os/cmsis/device/rtos/TOOLCHAIN_GCC_ARM/mbed_boot_gcc_arm.c
endif
#######################################################################
# mbed-os usb
#######################################################################
#SRC_MBED_CPP += lib/mbed-os/usb/device/hal/mbed_usb_phy.cpp
#SRC_MBED_CPP += lib/mbed-os/usb/device/targets/TARGET_RENESAS/$(TARGET)/USBPhy_RZ_A2.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/usb/AsyncOp.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/usb/ByteBuffer.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/usb/EndpointResolver.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/usb/LinkedListBase.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/usb/OperationListBase.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/usb/PolledQueue.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/usb/TaskBase.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/usb/USBAudio.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/usb/USBCDC.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/usb/USBCDC_ECM.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/usb/USBDevice.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/usb/USBHID.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/usb/USBKeyboard.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/usb/USBMIDI.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/usb/USBMouse.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/usb/USBMouseKeyboard.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/usb/USBMIDI.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/usb/USBMSD.cpp
SRC_MBED_CPP += lib/mbed-os/drivers/source/usb/USBSerial.cpp
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
SRC_MBED_CPP += lib/mbed-gr-libs/bd/RomRamBlockDevice/RomRamBlockDevice.cpp
endif
ifeq ($(MBED_GR_LIBS_components_AUDIO), 1)
endif
ifeq ($(MBED_GR_LIBS_components_CAMERA), 1)
SRC_MBED_CPP += lib/mbed-gr-libs/components/CAMERA/sccb.cpp
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
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_affine.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_argb2grayscale.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_bayer2grayscale.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_bayer2rgb.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_bayer2rgb_color_correction.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_binarization_adaptive.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_binarization_adaptive_bit.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_binarization_fixed.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_canny_calculate.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_canny_hysterisis.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_circle_fitting.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_corner_harris.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_cropping.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_cropping_rgb.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_dilate.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_erode.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_find_contours.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_gamma_correction.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_gaussian_blur.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_histogram.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_histogram_normalization.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_histogram_normalization_rgb.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_image_merging.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_image_rotate.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_laplacian.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_median_blur.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_minutiae_delete.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_minutiae_extract.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_prewitt.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_reed_solomon.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_reed_solomon_gf8.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_remap.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_resize_bilinear.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_resize_bilinear_fixed.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_resize_bilinear_fixed_rgb.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_resize_nearest.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_simple_isp_bayer2grayscale_3.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_simple_isp_bayer2grayscale_6.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_simple_isp_bayer2rgb_6.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_simple_isp_bayer2yuv_3.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_simple_isp_bayer2yuv_6.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_simple_isp_bayer2yuv_planar_3.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_simple_isp_bayer2yuv_planar_6.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_simple_isp_bg_subtraction_6.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_simple_isp_colcal_3dnr_6.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_simple_isp_distortion_correction_6.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_simple_isp_grayscale_3.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_simple_isp_grayscale_6.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_simple_isp_obj_det_color_6.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_simple_isp_obj_det_sobel_4.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_simple_isp_obj_det_sobel_6.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_simple_isp_scal_normaliz_b32_6.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_sobel.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_thinning.c
SRC_MBED_C += lib/mbed-gr-libs/drp-for-mbed/$(TARGET)/r_drp/drp_lib/r_drp_unsharp_masking.c
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
SRC_MBED_S += lib/mbed-gr-libs/GraphicsFramework/ospl/porting/TOOLCHAIN_GCC/r_ospl_os_less_asm.S
SRC_MBED_C += lib/mbed-gr-libs/GraphicsFramework/ospl/src/r_ospl.c
endif
ifeq ($(MBED_GR_LIBS_R_BSP), 1)
SRC_MBED_CPP += lib/mbed-gr-libs/R_BSP/common/R_BSP_Aio.cpp
SRC_MBED_CPP += lib/mbed-gr-libs/R_BSP/common/R_BSP_Scux.cpp
SRC_MBED_CPP += lib/mbed-gr-libs/R_BSP/common/R_BSP_SerialFamily.cpp
SRC_MBED_CPP += lib/mbed-gr-libs/R_BSP/common/R_BSP_Spdif.cpp
SRC_MBED_CPP += lib/mbed-gr-libs/R_BSP/common/R_BSP_Ssif.cpp
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
SRC_MBED_CPP += lib/mbed-gr-libs/SDBlockDevice_GRBoard/$(TARGET)/sdhi-driver/SDHSBlockDevice.cpp
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
ifeq ($(MBED_GR_LIBS_utility), 1)
SRC_MBED_C += lib/mbed-gr-libs/utility/mmu/r_mmu_lld.c
endif

endif

ifeq ($(MBED_HTTP), 1)
endif

#######################################################################
# source files for mbed_wrapper
#######################################################################

SRC_MBED_HAL_CPP += mbed/mbed_flash.cpp
SRC_MBED_HAL_CPP += mbed/mbed_timer.cpp

ifeq ($(MBED_OS_EMAC),1)
SRC_MBED_HAL_CPP += mbed/mbed_ether.cpp
endif

ifeq ($(MBED_UART_WRAPPER),1)
SRC_MBED_HAL_CPP += mbed/mbed_uart.cpp
endif

ifeq ($(MBED_SPI_WRAPPER),1)
SRC_MBED_HAL_CPP += mbed/mbed_spi.cpp
endif

ifeq ($(MBED_SD_WRAPPER),1)
# Doesn't work
SRC_MBED_HAL_CPP += mbed/mbed_sd.cpp
endif

ifeq ($(MBED_GR_LIBS), 1)
ifeq ($(MBED_GR_LIBS_components_CAMERA), 1)
SRC_MBED_CPP += mbed/mbed_camera.cpp
SRC_MBED_CPP += mbed/mbed_lcd.cpp
SRC_MBED_CPP += mbed/mbed_jpeg.cpp
endif
endif

#######################################################################
# object definition
#######################################################################

OBJ_MBED_HAL_C += $(addprefix $(BUILD_MBED)/, $(SRC_MBED_HAL_C:.c=.o))
OBJ_MBED_HAL_S += $(addprefix $(BUILD_MBED)/, $(SRC_MBED_HAL_S:.S=.o))
OBJ_MBED_HAL_CPP += $(addprefix $(BUILD_MBED)/, $(SRC_MBED_HAL_CPP:.cpp=.o))
OBJ_MBED_C += $(addprefix $(BUILD_MBED)/, $(SRC_MBED_C:.c=.o))
OBJ_MBED_S += $(addprefix $(BUILD_MBED)/, $(SRC_MBED_S:.S=.o))
OBJ_MBED_CPP += $(addprefix $(BUILD_MBED)/, $(SRC_MBED_CPP:.cpp=.o))
OBJ_MBED_HAL += $(OBJ_MBED_HAL_C) 
OBJ_MBED_HAL += $(OBJ_MBED_HAL_S) 
OBJ_MBED_HAL += $(OBJ_MBED_HAL_CPP) 
OBJ_MBED += $(OBJ_MBED_C) 
OBJ_MBED += $(OBJ_MBED_S) 
OBJ_MBED += $(OBJ_MBED_CPP) 

LDFLAGS_MBED += -DMBED_APP_START=0x50000000 -DMBED_APP_SIZE=0x1000000
LDFLAGS_MBED += -DMBED_ROM_START=0x50000000 -DMBED_ROM_SIZE=0x1000000
LDFLAGS_MBED += -DMBED_RAM_START=0x400000 -DMBED_RAM_SIZE=0x2000000 -DMBED_BOOT_STACK_SIZE=1024
LDFLAGS += $(LDFLAGS_MBED)

#######################################################################
# build rules
#######################################################################

vpath %.s . $(OBJ_MBED_HAL_S) $(OBJ_MBED_S)
$(BUILD_MBED)/%.o: %.s
	@dirname $@ | xargs mkdir -p
	$(ECHO) "AS $<"
	$(Q)$(AS) -g -o $@ $<

vpath %.S . $(OBJ_MBED_HAL_S) $(OBJ_MBED_S)
$(BUILD_MBED)/%.o: %.S
	@dirname $@ | xargs mkdir -p
	$(ECHO) "CC $<"
	$(Q)$(CC) $(CFLAGS_MBED) -g -c -o $@ $<

define compile_mbed_c
$(ECHO) "CC $<"
@dirname $@ | xargs mkdir -p
$(Q)$(CC) $(CFLAGS_MBED) $(CFLAGS_MBED_COMMON) $(CFLAGS_COMMON) -c -MD -o $@ $<
@# The following fixes the dependency file.
@# See http://make.paulandlesley.org/autodep.html for details.
@# Regex adjusted from the above to play better with Windows paths, etc.
@$(CP) $(@:.o=.d) $(@:.o=.P); \
  $(SED) -e 's/#.*//' -e 's/^.*:  *//' -e 's/ *\\$$//' \
      -e '/^$$/ d' -e 's/$$/ :/' < $(@:.o=.d) >> $(@:.o=.P); \
  $(RM) -f $(@:.o=.d)
endef

vpath %.c . $(OBJ_MBED_HAL_C) $(OBJ_MBED_C)
$(BUILD_MBED)/%.o: %.c
	$(call compile_mbed_c)

vpath %.c . $(OBJ_MBED_HAL_C) $(OBJ_MBED_C)

$(BUILD_MBED)/%.pp: %.c
	$(ECHO) "PreProcess $<"
	$(Q)$(CPP) $(CFLAGS_MBED) $(CFLAGS_MBED_COMMON) $(CFLAGS_COMMON) -Wp,-C,-dD,-dI -o $@ $<

define compile_mbed_cpp
$(ECHO) "CC $<"
@dirname $@ | xargs mkdir -p
$(Q)$(CXX) $(CPPFLAGS_MBED) $(CFLAGS_MBED_COMMON) $(CFLAGS_COMMON) -c -MD -o $@ $<
@# The following fixes the dependency file.
@# See http://make.paulandlesley.org/autodep.html for details.
@# Regex adjusted from the above to play better with Windows paths, etc.
@$(CP) $(@:.o=.d) $(@:.o=.P); \
  $(SED) -e 's/#.*//' -e 's/^.*:  *//' -e 's/ *\\$$//' \
      -e '/^$$/ d' -e 's/$$/ :/' < $(@:.o=.d) >> $(@:.o=.P); \
  $(RM) -f $(@:.o=.d)
endef

vpath %.cpp . $(OBJ_MBED_CPP)
$(BUILD_MBED)/%.o: %.cpp
	$(call compile_mbed_cpp)

QSTR_GEN_EXTRA_CPPFLAGS += -DNO_QSTR
QSTR_GEN_EXTRA_CPPFLAGS += -I$(BUILD)/tmp

vpath %.cpp . $(OBJ_MBED_CPP)

$(BUILD_MBED)/%.pp: %.cpp
	$(ECHO) "PreProcess $<"
	$(Q)$(CXX) $(CPPFLAGS_MBED) $(CFLAGS_MBED_COMMON) $(CFLAGS_COMMON) -Wp,-C,-dD,-dI -o $@ $<

#$(OBJ_MBED): $(OBJ_MBED_S) $(OBJ_MBED_C) $(OBJ_MBED_CPP)

#$(OBJ_MBED_HAL): $(OBJ_MBED_HAL_S) $(OBJ_MBED_HAL_C) $(OBJ_MBED_HAL_CPP)

AR_MBED_FLAGS = rcs

#$(LIB_MBED_HAL): $(OBJ_MBED_HAL)
#	$(ECHO) "LIB $@"
#	$(MKDIR) -p libs
#	$(Q)$(AR) $(AR_MBED_FLAGS) $(LIB_MBED_HAL) $^

$(LIB_MBED): $(OBJ_MBED) $(OBJ_MBED_HAL)
	$(ECHO) "LIB $@"
	$(MKDIR) -p $(BUILD_MBED_LIB)
	$(Q)$(AR) $(AR_MBED_FLAGS) $(LIB_MBED) $^
	

clean-mbed:
	$(RM) -rf $(BUILD_MBED)
	