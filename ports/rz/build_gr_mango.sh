#!/bin/bash
set -eu -o pipefail
git submodule update --init --recursive ../../lib/mbedtls
git submodule update --init --recursive ../../lib/lwip
git submodule update --init --recursive ../../lib/mbed-os
git submodule update --init             ../../lib/mbed-gr-libs
git submodule update --init --recursive ../../lib/lv_bindings
DT=`date +%Y%m%d%H%M`
mv ../../lib/lv_bindings/lv_conf.h ../../lib/lv_bindings/lv_conf_${DT}.h 
cp lvgl/lv_conf.h ../../lib/lv_bindings/lv_conf.h
export LOG_PATH="./_logs"
mkdir -p ${LOG_PATH}
export VERBOSE="V=1"
# export VERBOSE=
export BOARD="GR_MANGO"
DT=`date +%Y%m%d%H%M`
make ${VERBOSE} DEBUG=1 BOARD=${BOARD} clean 2>&1 | tee ${LOG_PATH}/${BOARD}_build_${DT}.log
make ${VERBOSE} DEBUG=1 BOARD=${BOARD} clean-mbed 2>&1 | tee -a ${LOG_PATH}/${BOARD}_build_${DT}.log
make ${VERBOSE} DEBUG=1 BOARD=${BOARD} MICROPY_PY_ESP=1 MICROPY_PY_LWIP=1 MICROPY_SSL_MBEDTLS=1 MICROPY_PY_USSL=1 2>&1 | tee -a ${LOG_PATH}/${BOARD}_build_${DT}.log
export BOARD="GR_MANGO_DD"
DT=`date +%Y%m%d%H%M`
make ${VERBOSE} DEBUG=1 BOARD=${BOARD} clean 2>&1 | tee ${LOG_PATH}/${BOARD}_build_${DT}.log
make ${VERBOSE} DEBUG=1 BOARD=${BOARD} clean-mbed 2>&1 | tee -a ${LOG_PATH}/${BOARD}_build_${DT}.log
make ${VERBOSE} DEBUG=1 BOARD=${BOARD} MICROPY_PY_ESP=1 MICROPY_PY_LWIP=1 MICROPY_SSL_MBEDTLS=1 MICROPY_PY_USSL=1 2>&1 | tee -a ${LOG_PATH}/${BOARD}_build_${DT}.log