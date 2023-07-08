#!/bin/bash
set -eu -o pipefail
# git submodule update --init ../../lib/pico-sdk
# git submodule update --init ../../lib/tinyusb
# git submodule update --init ../../lib/mbedtls
# git submodule update --init ../../lib/lwip
# git submodule update --init ../../lib/micropython-lib
# cp -f ./pico_sdk/tools/elf2uf2/CMakeLists.txt ../../lib/pico-sdk/tools/elf2uf2/CMakeLists.txt
# export USER_C_MODULES=./user_modules/micropython.cmake
export LOG_PATH="./_logs"
mkdir -p ${LOG_PATH}
# export VERBOSE="V=1"
export VERBOSE=
export BOARD="PICO_W"
export CMAKE_BUILD_TYPE=Debug
# export CFG_TUSB_DEBUG=0
DT=`date +%Y%m%d%H%M`
make ${VERBOSE} DEBUG=1 BOARD=${BOARD} clean 2>&1 | tee ${LOG_PATH}/${BOARD}_build_${DT}.log
make ${VERBOSE} DEBUG=1 BOARD=${BOARD}       2>&1 | tee -a ${LOG_PATH}/${BOARD}_build_${DT}.log