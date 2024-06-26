#!/bin/bash
set -eu -o pipefail
git submodule update --init --recursive ../../lib/mbedtls
git submodule update --init --recursive ../../lib/lwip
export LOG_PATH="./_logs"
mkdir -p ${LOG_PATH}
# export VERBOSE="V=1"
export VERBOSE=
export BOARD="GR_CITRUS"
DT=`date +%Y%m%d%H%M`
make ${VERBOSE} DEBUG=1 BOARD=${BOARD} clean 2>&1 | tee ${LOG_PATH}/${BOARD}_build_${DT}.log
make ${VERBOSE} DEBUG=1 BOARD=${BOARD} MICROPY_PY_ESP8266=1 MICROPY_SSL_MBEDTLS=1 MICROPY_PY_USSL=1 2>&1 | tee -a ${LOG_PATH}/${BOARD}_build_${DT}.log
export BOARD="GR_CITRUS_DD"
DT=`date +%Y%m%d%H%M`
make ${VERBOSE} DEBUG=1 BOARD=${BOARD} clean 2>&1 | tee ${LOG_PATH}/${BOARD}_build_${DT}.log
make ${VERBOSE} DEBUG=1 BOARD=${BOARD} MICROPY_PY_ESP8266=1 MICROPY_SSL_MBEDTLS=1 MICROPY_PY_USSL=1 2>&1 | tee -a ${LOG_PATH}/${BOARD}_build_${DT}.log
