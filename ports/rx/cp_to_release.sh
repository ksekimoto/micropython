#!/bin/bash
DT=`date +%Y%m%d%H%M`
mkdir -p ../../releases/gr_citrus/${DT}
mkdir -p ../../releases/gr_citrus/latest
cp build-GR_CITRUS_DD/firmware.bin ../../releases/gr_citrus/${DT}/MPY-GR_CITRUS_DD.bin
cp build-GR_CITRUS_DD/firmware.bin ../../releases/gr_citrus/latest/MPY-GR_CITRUS_DD.bin
mkdir -p ../../releases/gr_rose/${DT}
mkdir -p ../../releases/gr_rose/latest
cp build-GR_ROSE_DD/firmware.bin ../../releases/gr_rose/${DT}/MPY-GR_ROSE_DD.bin
cp build-GR_ROSE_DD/firmware.bin ../../releases/gr_rose/latest/MPY-GR_ROSE_DD.bin
mkdir -p ../../releases/gr_sakura/${DT}
mkdir -p ../../releases/gr_sakura/latest
cp build-GR_SAKURA_DD/firmware.bin ../../releases/gr_sakura/${DT}/MPY-GR_SAKURA_DD.bin
cp build-GR_SAKURA_DD/firmware.bin ../../releases/gr_sakura/latest/MPY-GR_SAKURA_DD.bin