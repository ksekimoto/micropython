#
# Makefile fragment for Renesas RZ MCUs.
#

# Create variables for the MCU name.

# MCU_SERIERS is defined in mpconfigboard.mk file under board folder
MCU_SERIES_UPPER = $(shell echo $(MCU_SERIES) | tr '[:lower:]' '[:upper:]')
MCU_SERIES_LOWER = $(shell echo $(MCU_SERIES) | tr '[:upper:]' '[:lower:]')
CMSIS_MCU_LOWER = $(shell echo $(CMSIS_MCU) | tr '[:upper:]' '[:lower:]')
