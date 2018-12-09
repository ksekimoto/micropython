General information about the rxboard port
==========================================

The rxboard are as follows

GR-CITRUS by Akiduki Densi


Technical specifications and SoC datasheets
-------------------------------------------

The datasheets and other reference material for rxboard chip are available
from the vendor site: http://gadget.renesas.com/ja/product/citrus.html .
They are the primary reference for the chip technical specifications, capabilities,
operating modes, internal functioning, etc.

For your convenience, some of technical specifications are provided below:

* Architecture: RX631(R5F5631FDDFP 100pin QFP)
* CPU frequency: 96MHz
* Total RAM available: 256KB (part of it reserved for system)
* Internal FlashROM: 2MB
* USB
* UART: 12 RX/TX UART
* SPI: 4 SPI interfaces.
* I2C: 4 I2C interfaces.


Scarcity of runtime resources
-----------------------------

rxboard has very modest resources (first of all, RAM memory). So, please
avoid allocating too big container objects (lists, dictionaries) and
buffers. There is also no full-fledged OS to keep track of resources
and automatically clean them up, so that's the task of a user/user
application: please be sure to close open files, sockets, etc. as soon
as possible after use.


Boot process
------------

On boot, MicroPython rxboard port executes ``_boot.py`` script from internal
frozen modules. It mounts filesystem in FlashROM, or if it's not available,
performs first-time setup of the module and creates the filesystem. This
part of the boot process is considered fixed, and not available for customization
for end users (even if you build from source, please refrain from changes to
it; customization of early boot process is available only to advanced users
and developers, who can diagnose themselves any issues arising from
modifying the standard process).

Once the filesystem is mounted, ``boot.py`` is executed from it. The standard
version of this file is created during first-time module set up and has
commands to start a WebREPL daemon (disabled by default, configurable
with ``webrepl_setup`` module), etc. This
file is customizable by end users (for example, you may want to set some
parameters or add other services which should be run on
a module start-up). But keep in mind that incorrect modifications to boot.py
may still lead to boot loops or lock ups, requiring to reflash a module
from scratch. (In particular, it's recommended that you use either
``webrepl_setup`` module or manual editing to configure WebREPL, but not
both).

As a final step of boot procedure, ``main.py`` is executed from filesystem,
if exists. This file is a hook to start up a user application each time
on boot (instead of going to REPL). For small test applications, you may
name them directly as ``main.py``, and upload to module, but instead it's
recommended to keep your application(s) in separate files, and have just
the following in ``main.py``::

    import my_app
    my_app.main()

This will allow to keep the structure of your application clear, as well as
allow to install multiple applications on a board, and switch among them.


Known Issues
------------

To be listed soon.