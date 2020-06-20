#!/bin/bash
set -eu -o pipefail
make V=1 DEBUG=1 BOARD=GR_SAKURA clean 2>&1 | tee GR_SAKURA_build.log
make V=1 DEBUG=1 BOARD=GR_SAKURA 2>&1 | tee GR_SAKURA_build.log
make V=1 DEBUG=1 BOARD=GR_SAKURA_DD clean 2>&1 | tee GR_SAKURA_DD_build.log
make V=1 DEBUG=1 BOARD=GR_SAKURA_DD 2>&1 | tee GR_SAKURA_DD_build.log
