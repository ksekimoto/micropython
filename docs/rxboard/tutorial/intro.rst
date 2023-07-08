.. _intro:

Getting started with MicroPython on the rxboard
===============================================

Using MicroPython is a great way to get the most of your rxboard board.  And
vice versa, the rxboard chip is a great platform for using MicroPython.  This
tutorial will guide you through setting up MicroPython, getting a prompt, using
WebREPL, connecting to the network and communicating with the Internet, using
the hardware peripherals, and controlling some external components.

Let's get started!

Requirements
------------

The first thing you need is a board with an rxboard chip.  The MicroPython
software supports the rxboard chip itself and any board should work.  The main
characteristic of a board is how much flash it has, how the GPIO pins are
connected to the outside world, and whether it includes a built-in USB-serial
convertor to make the UART available to your PC.

Powering the board
------------------

If your board has a USB connector on it then most likely it is powered through
this when connected to your PC.  Otherwise you will need to power it directly.
Please refer to the documentation for your board for further details.

Getting the firmware
--------------------

The first thing you need to do is download the most recent MicroPython firmware 
.bin file to load onto your rxboard device. You can download it from the  
`MicroPython downloads page <https://github.com/ksekimoto/micropython>`_.

Deploying the firmware
----------------------

Connect GR-CITRUS board to your PC and copy the firmware on RXBOARD drive by 
drag and drop.