#!/bin/bash
set -eu -o pipefail
make V=1 DEBUG=1 BOARD=GR_ROSE_LV clean 2>&1 | tee GR_ROSE_LV_build.log
make V=1 DEBUG=1 BOARD=GR_ROSE_LV 2>&1 | tee GR_ROSE_LV_build.log
make V=1 DEBUG=1 BOARD=GR_ROSE_LV_DD clean 2>&1 | tee GR_ROSE_LV_DD_build.log
make V=1 DEBUG=1 BOARD=GR_ROSE_LV_DD 2>&1 | tee GR_ROSE_LV_DD_build.log
