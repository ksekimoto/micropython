#!/bin/bash
set -eu -o pipefail
make V=1 DEBUG=1 BOARD=GR_MANGO clean 2>&1 | tee GR_MANGO_build.log
make V=1 DEBUG=1 BOARD=GR_MANGO MICROPY_PY_USSL=1 2>&1 | tee GR_MANGO_build.log
make V=1 DEBUG=1 BOARD=GR_MANGO_DD clean 2>&1 | tee GR_MANGO_DD_build.log
make V=1 DEBUG=1 BOARD=GR_MANGO_DD MICROPY_PY_USSL=1 2>&1 | tee GR_MANGO_DD_build.log
