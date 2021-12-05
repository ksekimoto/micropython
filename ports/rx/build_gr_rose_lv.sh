#!/bin/bash
set -eu -o pipefail
git submodule update --init --recursive ../../lib/mbedtls
git submodule update --init --recursive ../../lib/lwip
git submodule update --init --recursive ../../lib/lv_bindings
DT=`date +%Y%m%d%H%M`
mv ../../lib/lv_bindings/lv_conf.h ../../lib/lv_bindings/lv_conf_org_${DT}.h
cp -f ./lvgl/lv_conf.h ../../lib/lv_bindings/lv_conf.h
cp -f ./lvgl/mp_lodepng.c ../../lib/lv_bindings/driver/png/mp_lodepng.c
export BOARD="GR_ROSE_LV"
DT=`date +%Y%m%d%H%M`
make V=1 DEBUG=1 BOARD=${BOARD} clean 2>&1 | tee ${BOARD}_build_${DT}.log
make V=1 DEBUG=1 BOARD=${BOARD} MICROPY_PY_ESP8266=1 MICROPY_PY_LWIP=1 MICROPY_SSL_MBEDTLS=1 MICROPY_PY_USSL=1 MICROPY_PY_LVGL=1 2>&1 | tee -a ${BOARD}_build_${DT}.log
export BOARD="GR_ROSE_LV_DD"
DT=`date +%Y%m%d%H%M`
make V=1 DEBUG=1 BOARD=${BOARD} clean 2>&1 | tee ${BOARD}_build_${DT}.log
make V=1 DEBUG=1 BOARD=${BOARD} MICROPY_PY_ESP8266=1 MICROPY_PY_LWIP=1 MICROPY_SSL_MBEDTLS=1 MICROPY_PY_USSL=1 MICROPY_PY_LVGL=1 2>&1 | tee -a ${BOARD}_build_${DT}.log
