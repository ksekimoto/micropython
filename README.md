[![CI badge](https://github.com/micropython/micropython/workflows/unix%20port/badge.svg)](https://github.com/micropython/micropython/actions?query=branch%3Amaster+event%3Apush) [![codecov](https://codecov.io/gh/micropython/micropython/branch/master/graph/badge.svg?token=I92PfD05sD)](https://codecov.io/gh/micropython/micropython)

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
Python 3.5). The following core datatypes are provided: `str` (including
basic Unicode support), `bytes`, `bytearray`, `tuple`, `list`, `dict`, `set`,
`frozenset`, `array.array`, `collections.namedtuple`, classes and instances.
Builtin modules include `sys`, `time`, and `struct`, etc. Select ports have
support for `_thread` module (multithreading). Note that only a subset of
Python 3 functionality is implemented for the data types and modules.

MicroPython can execute scripts in textual source form or from precompiled
bytecode, in both cases either from an on-device filesystem or "frozen" into
the MicroPython executable.

See the repository http://github.com/micropython/pyboard for the MicroPython
board (PyBoard), the officially supported reference electronic circuit board.

Major components in this repository:
- py/ -- the core Python implementation, including compiler, runtime, and
  core library.
- mpy-cross/ -- the MicroPython cross-compiler which is used to turn scripts
  into precompiled bytecode.
- ports/unix/ -- a version of MicroPython that runs on Unix.
- ports/stm32/ -- a version of MicroPython that runs on the PyBoard and similar
  STM32 boards (using ST's Cube HAL drivers).
- ports/minimal/ -- a minimal MicroPython port. Start with this if you want
  to port MicroPython to another microcontroller.
- tests/ -- test framework and test scripts.
- docs/ -- user documentation in Sphinx reStructuredText format. Rendered
  HTML documentation is available at http://docs.micropython.org.

Additional components:
- ports/bare-arm/ -- a bare minimum version of MicroPython for ARM MCUs. Used
  mostly to control code size.
- ports/teensy/ -- a version of MicroPython that runs on the Teensy 3.1
  (preliminary but functional).
- ports/pic16bit/ -- a version of MicroPython for 16-bit PIC microcontrollers.
- ports/cc3200/ -- a version of MicroPython that runs on the CC3200 from TI.
- ports/esp8266/ -- a version of MicroPython that runs on Espressif's ESP8266 SoC.
- ports/esp32/ -- a version of MicroPython that runs on Espressif's ESP32 SoC.
- ports/nrf/ -- a version of MicroPython that runs on Nordic's nRF51 and nRF52 MCUs.
- extmod/ -- additional (non-core) modules implemented in C.
- tools/ -- various tools, including the pyboard.py module.
- examples/ -- a few example Python scripts.

The subdirectories above may include READMEs with additional info.

"make" is used to build the components, or "gmake" on BSD-based systems.
You will also need bash, gcc, and Python 3.3+ available as the command `python3`
(if your system only has Python 2.7 then invoke make with the additional option
`PYTHON=python2`).

The MicroPython cross-compiler, mpy-cross
-----------------------------------------

Most ports require the MicroPython cross-compiler to be built first.  This
program, called mpy-cross, is used to pre-compile Python scripts to .mpy
files which can then be included (frozen) into the firmware/executable for
a port.  To build mpy-cross use:

    $ cd mpy-cross
    $ make

The Unix version
----------------

The "unix" port requires a standard Unix environment with gcc and GNU make.
x86 and x64 architectures are supported (i.e. x86 32- and 64-bit), as well
as ARM and MIPS. Making full-featured port to another architecture requires
writing some assembly code for the exception handling and garbage collection.
Alternatively, fallback implementation based on setjmp/longjmp can be used.

To build (see section below for required dependencies):

    $ cd ports/unix
    $ make submodules
    $ make

Then to give it a try:

    $ ./micropython
    >>> list(5 * x + y for x in range(10) for y in [4, 2, 1])

Use `CTRL-D` (i.e. EOF) to exit the shell.
Learn about command-line options (in particular, how to increase heap size
which may be needed for larger applications):

    $ ./micropython -h

Run complete testsuite:

    $ make test

Unix version comes with a builtin package manager called upip, e.g.:

    $ ./micropython -m upip install micropython-pystone
    $ ./micropython -m pystone

Browse available modules on
[PyPI](https://pypi.python.org/pypi?%3Aaction=search&term=micropython).
Standard library modules come from
[micropython-lib](https://github.com/micropython/micropython-lib) project.

External dependencies
---------------------

Building MicroPython ports may require some dependencies installed.

For Unix port, `libffi` library and `pkg-config` tool are required. On
Debian/Ubuntu/Mint derivative Linux distros, install `build-essential`
(includes toolchain and make), `libffi-dev`, and `pkg-config` packages.

Other dependencies can be built together with MicroPython. This may
be required to enable extra features or capabilities, and in recent
versions of MicroPython, these may be enabled by default. To build
these additional dependencies, in the port directory you're
interested in (e.g. `ports/unix/`) first execute:

    $ make submodules

This will fetch all the relevant git submodules (sub repositories) that
the port needs.  Use the same command to get the latest versions of
submodules as they are updated from time to time. After that execute:

    $ make deplibs

This will build all available dependencies (regardless whether they
are used or not). If you intend to build MicroPython with additional
options (like cross-compiling), the same set of options should be passed
to `make deplibs`. To actually enable/disable use of dependencies, edit
`ports/unix/mpconfigport.mk` file, which has inline descriptions of the options.
For example, to build SSL module (required for `upip` tool described above,
and so enabled by default), `MICROPY_PY_USSL` should be set to 1.

For some ports, building required dependences is transparent, and happens
automatically.  But they still need to be fetched with the `make submodules`
command.

The STM32 version
-----------------

The "stm32" port requires an ARM compiler, arm-none-eabi-gcc, and associated
bin-utils.  For those using Arch Linux, you need arm-none-eabi-binutils,
arm-none-eabi-gcc and arm-none-eabi-newlib packages.  Otherwise, try here:
https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm

To build:

    $ cd ports/stm32
    $ make submodules
    $ make

You then need to get your board into DFU mode.  On the pyboard, connect the
3V3 pin to the P1/DFU pin with a wire (on PYBv1.0 they are next to each other
on the bottom left of the board, second row from the bottom).

Then to flash the code via USB DFU to your device:

    $ make deploy

This will use the included `tools/pydfu.py` script.  If flashing the firmware
does not work it may be because you don't have the correct permissions, and
need to use `sudo make deploy`.
See the README.md file in the ports/stm32/ directory for further details.

Contributing
------------

MicroPython is an open-source project and welcomes contributions. To be
productive, please be sure to follow the
[Contributors' Guidelines](https://github.com/micropython/micropython/wiki/ContributorGuidelines)
and the [Code Conventions](https://github.com/micropython/micropython/blob/master/CODECONVENTIONS.md).
Note that MicroPython is licenced under the MIT license, and all contributions
should follow this license.
