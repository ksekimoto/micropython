#!/bin/bash
DT=`date +%Y%m%d%H%M`
mkdir -p ../../releases/gr_mango/${DT}
mkdir -p ../../releases/gr_mango/latest
cp build-GR_MANGO_DD/firmware.bin ../../releases/gr_mango/${DT}/MPY-GR_MANGO_DD.bin
cp build-GR_MANGO_DD/firmware.bin ../../releases/gr_mango/latest/MPY-GR_MANGO_DD.bin
mkdir -p ../../releases/gr_mango_beta/${DT}
mkdir -p ../../releases/gr_mango_beta/latest
cp build-GR_MANGO_BETA_DD/firmware.bin ../../releases/gr_mango_beta/${DT}/MPY-GR_MANGO_BETA_DD.bin
cp build-GR_MANGO_BETA_DD/firmware.bin ../../releases/gr_mango_beta/latest/MPY-GR_MANGO_BETA_DD.bin
