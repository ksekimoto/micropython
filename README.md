[![CI badge](https://github.com/micropython/micropython/workflows/unix%20port/badge.svg)](https://github.com/micropython/micropython/actions?query=branch%3Amaster+event%3Apush) [![Coverage badge](https://coveralls.io/repos/micropython/micropython/badge.png?branch=master)](https://coveralls.io/r/micropython/micropython?branch=master)
The MicroPython project
=======================

The repository includes the MicroPython ported to Renesas RZ MPU and RX MPU, based on the [MicroPython](https://github.com/micropython/micropython).

The RZ version
-----------------

The supported board is

[GR_MANGO](http://japan.renesasrulz.com/gr_user_forum_japanese/f/gr-mangol)

- [Prebuild Image File](https://github.com/ksekimoto/micropython/blob/renesas/releases/gr_mango/lcd43/latest/MPY-GR_MANGO_DD.bin)
- [How to use](https://github.com/ksekimoto/micropython/tree/renesas/releases/docs/ja) (Japanese)

<p align="center">
  <img src="https://user-images.githubusercontent.com/3172869/91561306-55049d80-e976-11ea-91e2-da9a3035fec0.jpg" alt="GR-MANGO"/>
</p>

The RX version
-----------------

The supported boards are

[GR_ROSE](https://www.renesas.com/jp/ja/products/gadget-renesas/boards/gr-rose.html)
    
- [Prebuild Image File](https://github.com/ksekimoto/micropython/blob/renesas/releases/gr_rose/latest/MPY-GR_ROSE_DD.bin)
- [How to use](https://github.com/ksekimoto/micropython/tree/renesas/releases/docs/en) (English)
- [How to use](https://github.com/ksekimoto/micropython/tree/renesas/releases/docs/ja) (Japanese)

<p align="center">
  <img src="https://user-images.githubusercontent.com/3172869/82746147-82660c00-9dc7-11ea-9dfb-f6c94c8e3fdc.jpg" alt="GR-ROSE"/>
</p>

[GR_CITRUS](https://www.renesas.com/jp/ja/products/gadget-renesas/boards/gr-citrus.html)

- [Prebuild Image File](https://github.com/ksekimoto/micropython/blob/renesas/releases/gr_citrus/latest/MPY-GR_CITRUS_DD.bin)
- [How to use](https://github.com/ksekimoto/micropython/tree/renesas/releases/docs/ja) (Japanese)

<p align="center">
  <img src="https://user-images.githubusercontent.com/3172869/82746167-c0fbc680-9dc7-11ea-9723-80d6eb817c4c.jpg" alt="GR-CITRUS"/>
</p>

[GR_SAKURA](https://www.renesas.com/jp/ja/products/gadget-renesas/boards/gr-sakura.html)

- [Prebuild Image File](https://github.com/ksekimoto/micropython/blob/renesas/releases/gr_sakura/latest/MPY-GR_SAKURA_DD.bin)

<p align="center">
  <img src="https://user-images.githubusercontent.com/3172869/82746220-51d2a200-9dc8-11ea-802f-53727c92029f.jpg" alt="GR-SAKURA"/>
</p>

Build
-----

RZ port
-------

The "rz" port requires arm none eabi gcc compiler.

To build:

    $ git submodule update --init --recursive

    #
    # need to patch source files
    # https://github.com/ksekimoto/micropython/blob/renesas/ports/rz/patch.txt
    #

    $ cd mpy-cross
    $ make
    $ cd ../ports/rz
    $ make V=1 DEBUG=1 BOARD=GR_MANGO MICROPY_PY_USSL=1 2>&1 | tee GR_MANGO_build.log

RX port
-------

The "rx" port requires an Renesas GCC for RX compiler, rx-elf-gcc, and 
associated bin-utils. :
https://gcc-renesas.com/rx-download-toolchains/ 

To build:

    $ git submodule update --init --recursive
    $ cd mpy-cross
    $ make
    $ cd ../ports/rx
    $ make BOARD=GR_CITURS

To build on Ubuntu 18.04

    $ sudo apt-get update -qq || true
    $ sudo apt-get install -y libc6:i386 libncurses5:i386 libstdc++6:i386
    $ sudo apt-get install -y bzip2
    $ sudo apt-get install -y wget
    $ sudo apt-get install -y build-essential        
    $ sudo apt-get install -y python3
    # $ wget "https://github.com/ksekimoto/cross-gcc-build_bin/raw/master/rx/4.9.4/rx-elf-gcc-4.9.4.tar.gz" -k -O rx-elf-gcc-4.9.4.tar.gz
    # $ tar xvf rx-elf-gcc-4.9.4.tar.gz
    # $ sudo mv ./rx-elf-gcc-4.9.4 /opt
    # $ sudo chmod 777 /opt/rx-elf-gcc-4.9.4
    # $ export PATH=/opt/rx-elf-gcc-4.9.4/bin:$PATH
    # please install Renesas GCC for RX compiler 4.8.4
    $ sudo apt-get install -y git
    $ git clone https://github.com/ksekimoto/micropython.git
    $ cd micropython
    $ git submodule update --init
    $ cd mpy-cross
    $ make
    $ cd ../ports/rx
    $ make V=1 DEBUG=1 MICROPY_PY_ESP8266=1 MICROPY_SSL_MBEDTLS=1 MICROPY_PY_USSL=1 BOARD=GR_CITRUS_DD clean 2>&1 | tee GR_CITRUS_DD_build.log
    $ make V=1 DEBUG=1 MICROPY_PY_ESP8266=1 MICROPY_SSL_MBEDTLS=1 MICROPY_PY_USSL=1 BOARD=GR_CITRUS_DD 2>&1 | tee GR_CITRUS_DD_build.log
    $ make V=1 DEBUG=1 MICROPY_PY_ESP8266=1 MICROPY_PY_LWIP=1 MICROPY_SSL_MBEDTLS=1 MICROPY_PY_USSL=1 BOARD=GR_ROSE_DD clean 2>&1 | tee GR_ROSE_DD_build.log
    $ make V=1 DEBUG=1 MICROPY_PY_ESP8266=1 MICROPY_PY_LWIP=1 MICROPY_SSL_MBEDTLS=1 MICROPY_PY_USSL=1 BOARD=GR_ROSE_DD 2>&1 | tee GR_ROSE_DD_build.log
    $ make V=1 DEBUG=1 MICROPY_PY_LWIP=1 MICROPY_SSL_MBEDTLS=1 MICROPY_PY_USSL=1 BOARD=GR_SAKURA_DD clean 2>&1 | tee GR_SAKURA_DD_build.log
    $ make V=1 DEBUG=1 MICROPY_PY_LWIP=1 MICROPY_SSL_MBEDTLS=1 MICROPY_PY_USSL=1 BOARD=GR_SAKURA_DD 2>&1 | tee GR_SAKURA_DD_build.log


The compiled elf/mot/bin binary files are created under 
build-'board name' folder

    firmware.elf
    firmware.mot
    firmware.bin

You can flash the mot file by using Renesas Flash Development Toolkit:
https://www.renesas.com/us/en/products/software-tools/tools/programmer/flash-development-toolkit-programming-gui.html#productInfo

Also you can flash the firmware.bin for USB MAS Storage firmware via
Drag and Drop on Windows Explorer.
To build the bin file, you need to specify "BOARD=BOARDNAME_DD", such as
"BOARD=GR_CITRUS_DD" or "BOARD=GR_SAKURA_DD". 

At default, 256KB of flash drive is allocated from internal flash ROM 
area (0xFFFA0000-0xFFFDFFFF).



[![Unix CI badge](https://github.com/micropython/micropython/actions/workflows/ports_unix.yml/badge.svg)](https://github.com/micropython/micropython/actions?query=branch%3Amaster+event%3Apush) [![STM32 CI badge](https://github.com/micropython/micropython/actions/workflows/ports_stm32.yml/badge.svg)](https://github.com/micropython/micropython/actions?query=branch%3Amaster+event%3Apush) [![Docs CI badge](https://github.com/micropython/micropython/actions/workflows/docs.yml/badge.svg)](https://docs.micropython.org/) [![codecov](https://codecov.io/gh/micropython/micropython/branch/master/graph/badge.svg?token=I92PfD05sD)](https://codecov.io/gh/micropython/micropython)

The MicroPython project
=======================
<p align="center">
  <img src="https://raw.githubusercontent.com/micropython/micropython/master/logo/upython-with-micro.jpg" alt="MicroPython Logo"/>
</p>

This is the MicroPython project, which aims to put an implementation
of Python 3.x on microcontrollers and small embedded systems.
You can find the official website at [micropython.org](http://www.micropython.org).

WARNING: this project is in beta stage and is subject to changes of the
code-base, including project-wide name changes and API changes.

MicroPython implements the entire Python 3.4 syntax (including exceptions,
`with`, `yield from`, etc., and additionally `async`/`await` keywords from
Python 3.5 and some select features from later versions). The following core
datatypes are provided: `str`(including basic Unicode support), `bytes`,
`bytearray`, `tuple`, `list`, `dict`, `set`, `frozenset`, `array.array`,
`collections.namedtuple`, classes and instances. Builtin modules include
`os`, `sys`, `time`, `re`, and `struct`, etc. Select ports have support for
`_thread` module (multithreading), `socket` and `ssl` for networking, and
`asyncio`. Note that only a subset of Python 3 functionality is implemented
for the data types and modules.

MicroPython can execute scripts in textual source form (.py files) or from
precompiled bytecode (.mpy files), in both cases either from an on-device
filesystem or "frozen" into the MicroPython executable.

MicroPython also provides a set of MicroPython-specific modules to access
hardware-specific functionality and peripherals such as GPIO, Timers, ADC,
DAC, PWM, SPI, I2C, CAN, Bluetooth, and USB.

Getting started
---------------

See the [online documentation](https://docs.micropython.org/) for API
references and information about using MicroPython and information about how
it is implemented.

We use [GitHub Discussions](https://github.com/micropython/micropython/discussions)
as our forum, and [Discord](https://discord.gg/RB8HZSAExQ) for chat. These
are great places to ask questions and advice from the community or to discuss your
MicroPython-based projects.

For bugs and feature requests, please [raise an issue](https://github.com/micropython/micropython/issues/new/choose)
and follow the templates there.

For information about the [MicroPython pyboard](https://store.micropython.org/pyb-features),
the officially supported board from the
[original Kickstarter campaign](https://www.kickstarter.com/projects/214379695/micro-python-python-for-microcontrollers),
see the [schematics and pinouts](http://github.com/micropython/pyboard) and
[documentation](https://docs.micropython.org/en/latest/pyboard/quickref.html).

Contributing
------------

MicroPython is an open-source project and welcomes contributions. To be
productive, please be sure to follow the
[Contributors' Guidelines](https://github.com/micropython/micropython/wiki/ContributorGuidelines)
and the [Code Conventions](https://github.com/micropython/micropython/blob/master/CODECONVENTIONS.md).
Note that MicroPython is licenced under the MIT license, and all contributions
should follow this license.

About this repository
---------------------

This repository contains the following components:
- [py/](py/) -- the core Python implementation, including compiler, runtime, and
  core library.
- [mpy-cross/](mpy-cross/) -- the MicroPython cross-compiler which is used to turn scripts
  into precompiled bytecode.
- [ports/](ports/) -- platform-specific code for the various ports and architectures that MicroPython runs on.
- [lib/](lib/) -- submodules for external dependencies.
- [tests/](tests/) -- test framework and test scripts.
- [docs/](docs/) -- user documentation in Sphinx reStructuredText format. This is used to generate the [online documentation](http://docs.micropython.org).
- [extmod/](extmod/) -- additional (non-core) modules implemented in C.
- [tools/](tools/) -- various tools, including the pyboard.py module.
- [examples/](examples/) -- a few example Python scripts.

"make" is used to build the components, or "gmake" on BSD-based systems.
You will also need bash, gcc, and Python 3.3+ available as the command `python3`
(if your system only has Python 2.7 then invoke make with the additional option
`PYTHON=python2`). Some ports (rp2 and esp32) additionally use CMake.

Supported platforms & architectures
-----------------------------------

MicroPython runs on a wide range of microcontrollers, as well as on Unix-like
(including Linux, BSD, macOS, WSL) and Windows systems.

Microcontroller targets can be as small as 256kiB flash + 16kiB RAM, although
devices with at least 512kiB flash + 128kiB RAM allow a much more
full-featured experience.

The [Unix](ports/unix) and [Windows](ports/windows) ports allow both
development and testing of MicroPython itself, as well as providing
lightweight alternative to CPython on these platforms (in particular on
embedded Linux systems).

The ["minimal"](ports/minimal) port provides an example of a very basic
MicroPython port and can be compiled as both a standalone Linux binary as
well as for ARM Cortex M4. Start with this if you want to port MicroPython to
another microcontroller. Additionally the ["bare-arm"](ports/bare-arm) port
is an example of the absolute minimum configuration, and is used to keep
track of the code size of the core runtime and VM.

In addition, the following ports are provided in this repository:
 - [cc3200](ports/cc3200) -- Texas Instruments CC3200 (including PyCom WiPy).
 - [esp32](ports/esp32) -- Espressif ESP32 SoC (including ESP32S2, ESP32S3, ESP32C3).
 - [esp8266](ports/esp8266) -- Espressif ESP8266 SoC.
 - [mimxrt](ports/mimxrt) -- NXP m.iMX RT (including Teensy 4.x).
 - [nrf](ports/nrf) -- Nordic Semiconductor nRF51 and nRF52.
 - [pic16bit](ports/pic16bit) -- Microchip PIC 16-bit.
 - [powerpc](ports/powerpc) -- IBM PowerPC (including Microwatt)
 - [qemu-arm](ports/qemu-arm) -- QEMU-based emulated target, for testing)
 - [renesas-ra](ports/renesas-ra) -- Renesas RA family.
 - [rp2](ports/rp2) -- Raspberry Pi RP2040 (including Pico and Pico W).
 - [samd](ports/samd) -- Microchip (formerly Atmel) SAMD21 and SAMD51.
 - [stm32](ports/stm32) -- STMicroelectronics STM32 family (including F0, F4, F7, G0, G4, H7, L0, L4, WB)
 - [teensy](ports/teensy) -- Teensy 3.x.
 - [webassembly](ports/webassembly) -- Emscripten port targeting browsers and NodeJS.
 - [zephyr](ports/zephyr) -- Zephyr RTOS.

The MicroPython cross-compiler, mpy-cross
-----------------------------------------

Most ports require the [MicroPython cross-compiler](mpy-cross) to be built
first.  This program, called mpy-cross, is used to pre-compile Python scripts
to .mpy files which can then be included (frozen) into the
firmware/executable for a port.  To build mpy-cross use:

    $ cd mpy-cross
    $ make

External dependencies
---------------------

The core MicroPython VM and runtime has no external dependencies, but a given
port might depend on third-party drivers or vendor HALs. This repository
includes [several submodules](lib/) linking to these external dependencies.
Before compiling a given port, use

    $ cd ports/name
    $ make submodules

to ensure that all required submodules are initialised.
