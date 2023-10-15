#!/bin/bash
set -eu -o pipefail
export BOARD="RA4M1_CLICKER"
# git submodule update --init --recursive ../../lib/fsp
export LOG_PATH="./../../../_logs"
mkdir -p ${LOG_PATH}
# export VERBOSE="V=1"
export VERBOSE=
DT=`date +%Y%m%d%H%M`
make ${VERBOSE} DEBUG=1 BOARD=${BOARD} clean 2>&1 | tee ${LOG_PATH}/${BOARD}_build_${DT}.log
make ${VERBOSE} DEBUG=1 BOARD=${BOARD}       2>&1 | tee -a ${LOG_PATH}/${BOARD}_build_${DT}.log
#
export BOARD="EK_RA6M2"
DT=`date +%Y%m%d%H%M`
make ${VERBOSE} DEBUG=1 BOARD=${BOARD} clean 2>&1 | tee ${LOG_PATH}/${BOARD}_build_${DT}.log
make ${VERBOSE} DEBUG=1 BOARD=${BOARD}       2>&1 | tee -a ${LOG_PATH}/${BOARD}_build_${DT}.log
#
export BOARD="EK_RA4M1"
DT=`date +%Y%m%d%H%M`
make ${VERBOSE} DEBUG=1 BOARD=${BOARD} clean 2>&1 | tee ${LOG_PATH}/${BOARD}_build_${DT}.log
make ${VERBOSE} DEBUG=1 BOARD=${BOARD}       2>&1 | tee -a ${LOG_PATH}/${BOARD}_build_${DT}.log
#
export BOARD="EK_RA4W1"
DT=`date +%Y%m%d%H%M`
make ${VERBOSE} DEBUG=1 BOARD=${BOARD} clean 2>&1 | tee ${LOG_PATH}/${BOARD}_build_${DT}.log
make ${VERBOSE} DEBUG=1 BOARD=${BOARD}       2>&1 | tee -a ${LOG_PATH}/${BOARD}_build_${DT}.log
#
export BOARD="EK_RA6M1"
DT=`date +%Y%m%d%H%M`
make ${VERBOSE} DEBUG=1 BOARD=${BOARD} clean 2>&1 | tee ${LOG_PATH}/${BOARD}_build_${DT}.log
make ${VERBOSE} DEBUG=1 BOARD=${BOARD}       2>&1 | tee -a ${LOG_PATH}/${BOARD}_build_${DT}.log
#
export BOARD="RA6M5_EK"
DT=`date +%Y%m%d%H%M`
make ${VERBOSE} DEBUG=1 BOARD=${BOARD} clean 2>&1 | tee ${LOG_PATH}/${BOARD}_build_${DT}.log
make ${VERBOSE} DEBUG=1 BOARD=${BOARD}       2>&1 | tee -a ${LOG_PATH}/${BOARD}_build_${DT}.log
#
export BOARD="RA6E1_FPB"
DT=`date +%Y%m%d%H%M`
make ${VERBOSE} DEBUG=1 BOARD=${BOARD} clean 2>&1 | tee ${LOG_PATH}/${BOARD}_build_${DT}.log
make ${VERBOSE} DEBUG=1 BOARD=${BOARD}       2>&1 | tee -a ${LOG_PATH}/${BOARD}_build_${DT}.log