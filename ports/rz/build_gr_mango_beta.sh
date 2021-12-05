#!/bin/bash
set -eu -o pipefail
export BOARD="GR_MANGO_BETA"
DT=`date +%Y%m%d%H%M`
make V=1 DEBUG=1 BOARD=${BOARD} clean 2>&1 | tee ${BOARD}_build_${DT}.log
make V=1 DEBUG=1 BOARD=${BOARD} clean-mbed 2>&1 | tee -a ${BOARD}_build_${DT}.log
make V=1 DEBUG=1 BOARD=${BOARD} MICROPY_PY_ESP8266=1 MICROPY_PY_LWIP=1 MICROPY_SSL_MBEDTLS=1 MICROPY_PY_USSL=1 2>&1 | tee -a ${BOARD}_build_${DT}.log
export BOARD="GR_MANGO_BETA_DD"
DT=`date +%Y%m%d%H%M`
make V=1 DEBUG=1 BOARD=${BOARD} clean 2>&1 | tee ${BOARD}_build_${DT}.log
make V=1 DEBUG=1 BOARD=${BOARD} clean-mbed 2>&1 | tee -a ${BOARD}_build_${DT}.log
make V=1 DEBUG=1 BOARD=${BOARD} MICROPY_PY_ESP8266=1 MICROPY_PY_LWIP=1 MICROPY_SSL_MBEDTLS=1 MICROPY_PY_USSL=1 2>&1 | tee -a ${BOARD}_build_${DT}.log
