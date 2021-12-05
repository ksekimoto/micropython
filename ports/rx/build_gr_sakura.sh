#!/bin/bash
set -eu -o pipefail
git submodule update --init --recursive ../../lib/mbedtls
git submodule update --init --recursive ../../lib/lwip
export BOARD="GR_SAKURA"
DT=`date +%Y%m%d%H%M`
make V=1 DEBUG=1 BOARD=${BOARD} clean 2>&1 | tee ${BOARD}_build_${DT}.log
make V=1 DEBUG=1 BOARD=${BOARD} 2>&1 | tee -a ${BOARD}_build_${DT}.log
export BOARD="GR_SAKURA_DD"
DT=`date +%Y%m%d%H%M`
make V=1 DEBUG=1 BOARD=${BOARD} clean 2>&1 | tee ${BOARD}_build_${DT}.log
make V=1 DEBUG=1 BOARD=${BOARD} 2>&1 | tee -a ${BOARD}_build_${DT}.log
