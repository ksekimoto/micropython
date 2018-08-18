# The Renesas RX port

This port is intended to be a Renesas RX MicroPython port that actually runs.
It can run on RX63N/RX631 MCU (eg GR-SAKURA or GR-CITRUS).

## Building for an RX63N/RX631 MCU

The Makefile has the ability to build for Renesas RX CPU, and by default
includes some start-up code for an RX63N MCU and also enables a UART
for communication.  To build:

    $ make CROSS=1

If you previously built the Linux version, you will need to first run
`make clean` to get rid of incompatible object files.

This version of the build will work out-of-the-box on a pyboard (and
anything similar), and will give you a MicroPython REPL on UART1 at 115200
baud. 

## Building without the built-in MicroPython compiler

This Renesas RX port can be built with the built-in MicroPython compiler
disabled. Without the compiler the REPL will be
disabled, but pre-compiled scripts can still be executed.

To test out this feature, change the `MICROPY_ENABLE_COMPILER` config
option to "0" in the mpconfigport.h file in this directory.  Then
recompile and run the firmware and it will execute the frozentest.py
file.
