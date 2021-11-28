#!/bin/bash
set -eu -o pipefail
export BOARD="GR_SAKURA"
DT=`date +%Y%m%d%H%M`
make V=1 DEBUG=1 BOARD=${BOARD} 2>&1 | tee -a ${BOARD}_build_${DT}.log
