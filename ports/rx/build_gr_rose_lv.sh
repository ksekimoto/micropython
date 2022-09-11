#!/bin/bash
set -eu -o pipefail
git submodule update --init --recursive ../../lib/mbedtls
git submodule update --init --recursive ../../lib/lwip
git submodule update --init --recursive ../../lib/lv_bindings
mv ../../lib/lv_bindings/lv_conf.h ../../lib/lv_bindings/lv_conf_org_${DT}.h
cp -f ./lvgl/lv_conf.h ../../lib/lv_bindings/lv_conf.h
cp -f ./lvgl/mp_lodepng.c ../../lib/lv_bindings/driver/png/mp_lodepng.c
DT=`date +%Y%m%d%H%M`
export LOG_PATH="./_logs"
mkdir -p ${LOG_PATH}
# export VERBOSE="V=1"
export VERBOSE=
export BOARD="GR_ROSE_LV"
DT=`date +%Y%m%d%H%M`
make ${VERBOSE} DEBUG=1 BOARD=${BOARD} clean 2>&1 | tee ${LOG_PATH}/${BOARD}_build_${DT}.log
make ${VERBOSE} DEBUG=1 BOARD=${BOARD} MICROPY_PY_ESP=1 MICROPY_PY_LWIP=1 MICROPY_SSL_MBEDTLS=1 MICROPY_PY_USSL=1 MICROPY_PY_LVGL=1 2>&1 | tee -a ${LOG_PATH}/${BOARD}_build_${DT}.log
export BOARD="GR_ROSE_LV_DD"
DT=`date +%Y%m%d%H%M`
make ${VERBOSE} DEBUG=1 BOARD=${BOARD} clean 2>&1 | tee ${LOG_PATH}/${BOARD}_build_${DT}.log
make ${VERBOSE} DEBUG=1 BOARD=${BOARD} MICROPY_PY_ESP=1 MICROPY_PY_LWIP=1 MICROPY_SSL_MBEDTLS=1 MICROPY_PY_USSL=1 MICROPY_PY_LVGL=1 2>&1 | tee -a ${LOG_PATH}/${BOARD}_build_${DT}.log
