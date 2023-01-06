# CMake fragment for MicroPython esp component
set(ESP_DRIVER_DIR "${MICROPY_DIR}/drivers/esp")

add_library(micropy_driver_esp INTERFACE)

target_sources(micropy_driver_esp INTERFACE
    ${ESP_DRIVER_DIR}/esp_driver.c
    ${ESP_DRIVER_DIR}/tinymalloc.c
    ${ESP_DRIVER_DIR}/vector.c
    ${ESP_DRIVER_DIR}/mpy_uart.c
    ${ESP_DRIVER_DIR}/mpy_debug.c
    ${MICROPY_DIR}/extmod/network_esp.c
)

target_include_directories(micropy_driver_esp INTERFACE
    ${MICROPY_DIR}/
    ${MICROPY_PORT_DIR}/
    ${MICROPY_BOARD_DIR}/
    ${ESP_DRIVER_DIR}/
)