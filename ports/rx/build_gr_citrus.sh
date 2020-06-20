#!/bin/bash
set -eu -o pipefail
make V=1 DEBUG=1 BOARD=GR_CITRUS clean 2>&1 | tee GR_CITRUS_build.log
make V=1 DEBUG=1 BOARD=GR_CITRUS 2>&1 | tee GR_CITRUS_build.log
make V=1 DEBUG=1 BOARD=GR_CITRUS_DD clean 2>&1 | tee GR_CITRUS_DD_build.log
make V=1 DEBUG=1 BOARD=GR_CITRUS_DD 2>&1 | tee GR_CITRUS_DD_build.log
