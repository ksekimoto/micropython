# CMake fragment for MicroPython esp component

set(MICROPY_SOURCE_ESP
    ${CMAKE_CURRENT_LIST_DIR}/esp_driver.c
    ${CMAKE_CURRENT_LIST_DIR}/tinymalloc.c
    ${CMAKE_CURRENT_LIST_DIR}/vector.c
    ${MICROPY_DIR}/extmod/network_esp.c
)

set(MICROPY_INC_ESP
    ${CMAKE_CURRENT_LIST_DIR}
)

list(APPEND MICROPY_INC_CORE
    ${CMAKE_CURRENT_LIST_DIR}
)
