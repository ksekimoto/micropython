#!/bin/bash
set -eu -o pipefail
export BOARD="GR_ROSE_LV"
DT=`date +%Y%m%d%H%M`
make V=1 DEBUG=1 BOARD=${BOARD} MICROPY_PY_ESP8266=1 MICROPY_PY_LWIP=1 MICROPY_SSL_MBEDTLS=1 MICROPY_PY_USSL=1 MICROPY_PY_LVGL=1 2>&1 | tee -a ${BOARD}_build_${DT}.log
