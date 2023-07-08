# CMake fragment for MicroPython lcdspi component

set(MICROPY_SOURCE_SPILCD
    ${CMAKE_CURRENT_LIST_DIR}/lcdspi/mpy_file.c
    ${CMAKE_CURRENT_LIST_DIR}/lcdspi/lcdspi.c
    ${CMAKE_CURRENT_LIST_DIR}/lcdspi/lcdspi_info.c
    ${CMAKE_CURRENT_LIST_DIR}/lcdspi/lcdspi_file.c
    ${CMAKE_CURRENT_LIST_DIR}/font/font.c
    ${CMAKE_CURRENT_LIST_DIR}/jpeg/jpeg.c
    ${CMAKE_CURRENT_LIST_DIR}/jpeg/picojpeg.c
    ${CMAKE_CURRENT_LIST_DIR}/jpeg/jpeg_src.c
    ${CMAKE_CURRENT_LIST_DIR}/jpeg/jpeg_disp.c
    ${CMAKE_CURRENT_LIST_DIR}/rp2_lcdspi.c
    ${CMAKE_CURRENT_LIST_DIR}/rp2_font.c
    ${CMAKE_CURRENT_LIST_DIR}/xpt2046/xpt2046.c
)

set(MICROPY_INC_SPILCD
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/lcdspi
    ${CMAKE_CURRENT_LIST_DIR}/font
    ${CMAKE_CURRENT_LIST_DIR}/jpeg
    ${CMAKE_CURRENT_LIST_DIR}/fatfs
    ${CMAKE_CURRENT_LIST_DIR}/xpt2046
)

list(APPEND MICROPY_INC_CORE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/lcdspi
    ${CMAKE_CURRENT_LIST_DIR}/font
    ${CMAKE_CURRENT_LIST_DIR}/jpeg
    ${CMAKE_CURRENT_LIST_DIR}/fatfs
    ${CMAKE_CURRENT_LIST_DIR}/xpt2046
)
