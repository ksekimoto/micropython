cmake_minimum_required(VERSION 2.8)

project(mbed-http)

set(MBED_HTTP_SOURCE_DIR source)
set(MBED_HTTP_HTTP_PARSER_DIR http_parser)

file(GLOB_RECURSE mbed_http_sources RELATIVE "${CMAKE_CURRENT_LIST_DIR}" "${MBED_HTTP_SOURCE_DIR}/*.h" "${MBED_HTTP_HTTP_PARSER_DIR}/*.c")

add_library(mbed_http_objs OBJECT
  ${mbed_http_sources}
)

add_library(carotene STATIC EXCLUDE_FROM_ALL 
  "$<TARGET_OBJECTS:mbed_http_objs>"
)