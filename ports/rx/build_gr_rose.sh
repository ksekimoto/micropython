#!/bin/bash
set -eu -o pipefail
make V=1 DEBUG=1 BOARD=GR_ROSE clean 2>&1 | tee GR_ROSE_build.log
make V=1 DEBUG=1 BOARD=GR_ROSE 2>&1 | tee GR_ROSE_build.log
make V=1 DEBUG=1 BOARD=GR_ROSE_DD clean 2>&1 | tee GR_ROSE_DD_build.log
make V=1 DEBUG=1 BOARD=GR_ROSE_DD 2>&1 | tee GR_ROSE_DD_build.log
