.. _quickref:

Quick reference for the rxboard
===============================

GR-CITRUS board
---------------

.. image:: img/grcitrus_pic.jpg
    :alt: GR-CITRUS board
    :width: 640px

Installing MicroPython
----------------------

See the corresponding section of tutorial: :ref:`intro`. It also includes
a troubleshooting subsection.

General board control
---------------------

The MicroPython REPL is on USB at baudrate 115200.
Tab-completion is useful to find out what methods an object has.
Paste mode (ctrl-E) is useful to paste a large slab of Python code into
the REPL.

Internal Flash
--------------

The part of the internal flash memory is configured as flash file system.

Display flash file system::

    import uos

    uos.listdir('/flash')


Delay and timing
----------------

Use the :mod:`time <utime>` module::

    import time

    time.sleep(1)           # sleep for 1 second
    time.sleep_ms(500)      # sleep for 500 milliseconds
    time.sleep_us(10)       # sleep for 10 microseconds
    start = time.ticks_ms() # get millisecond counter
    delta = time.ticks_diff(time.ticks_ms(), start) # compute time difference


Internal LED
------------

See :ref:`pyb.LED <pyb.LED>`. ::

    from pyb import LED

    led = LED(1)    # PA0 pin
    led.toggle()
    led.on()
    led.off()


Timers
------

See :ref:`pyb.Timer <pyb.Timer>`. ::

    from pyb import Timer

    tim = Timer(1, freq=1000)
    tim.counter() # get counter value
    tim.freq(5) # 5 Hz
    tim.callback(lambda t: pyb.LED(1).toggle())

    freq: only integer is supported.


Pins and GPIO
-------------

See :ref:`pyb.Pin <pyb.Pin>`. ::

    from pyb import Pin

    p_out = Pin('PIN19', Pin.OUT_PP)
    p_out.high()
    p_out.low()

    p_in = Pin('PIN19', Pin.IN, Pin.PULL_UP)
    p_in.value() # get value, 0 or 1


.. image:: img/grcitrus_pins.jpg
    :alt: GR-CITRUS board
    :width: 640px

GR-CITRUS Pin Information ::

    PIN0    P20     ExtInt
    PIN1    P21     ExtInt
    PIN2    PC0     ExtInt
    PIN3    PC1     ExtInt
    PIN4    PC2
    PIN5    P50
    PIN6    P52
    PIN7    P32     ExtInt
    PIN8    P33     ExtInt
    PIN9    P05     ExtInt
    PIN10   PC4
    PIN11   PC6     ExtInt
    PIN12   PC7     ExtInt
    PIN13   PC5
    PIN14   P40     ExtInt
    PIN15   P41     ExtInt
    PIN16   P42     ExtInt
    PIN17   P43     ExtInt
    PIN18   P12     ExtInt
    PIN19   P13     ExtInt
    NMI P35
    LED PA0


External interrupts
-------------------

See :ref:`pyb.ExtInt <pyb.ExtInt>`. ::

    from pyb import Pin, ExtInt

    callback = lambda e: print("intr")
    ext = ExtInt(Pin('P14'), ExtInt.IRQ_RISING, Pin.PULL_NONE, callback)

Limitation
^^^^^^^^^^

The debounce protection is not supported. The chattering might happen.


UART (serial bus)
-----------------

See :ref:`pyb.UART <pyb.UART>`. ::

    from pyb import UART

    uart = UART(1, 9600)
    uart.write('hello')
    uart.read(5) # read up to 5 bytes

Limitation
^^^^^^^^^^

Only 8 channels (SCI0 - SCI7) are supported


Hardware SPI bus
----------------

The hardware SPI is faster (up to 48Mhz), but only works on following pins:
For SPI0, ``MISO`` is PIN12, ``MOSI`` is PIN11, and ``SCK`` is PIN1. It has the same
methods as the bitbanging SPI class above, except for the pin parameters for the
constructor and init (as those are fixed)::

    from machine import Pin, SPI

    hspi = SPI(0, baudrate=80000000, polarity=0, phase=0)

I2C bus
-------

The I2C driver is implemented in software and works on all pins,
and is accessed via the :ref:`machine.I2C <machine.I2C>` class::

    from machine import Pin, I2C

    # construct an I2C bus
    i2c = I2C(scl=Pin('PIN16'), sda=Pin('PIN17'), freq=100000)

    i2c.readfrom(0x48, 2)   # read 2 bytes from slave device with address 0x48
    i2c.writeto(0x3a, '12') # write '12' to slave device with address 0x3a

    buf = bytearray(10)     # create a buffer with 10 bytes
    i2c.writeto(0x3a, buf)  # write the given buffer to the slave


ADC (analog to digital conversion)
----------------------------------

Sample::

    adc = pyb.ADC(pyb.Pin.board.PIN17)
    val = adc.read()
    print(val)


WA-MIKAN board
--------------

.. image:: img/wamikan_pic.jpg
    :alt: WA-MIKAN board
    :width: 640px


* `slide <https://www.slideshare.net/MinaoYamamoto/wamikan>`_


SDCARD
-------

If the WA-MIKAN board is attached and the SDCARD is inserted, it should be detetced.

Display SD file system::

    import uos
    uos.listdir('/sd')


WIFI
----

Sample::

    import wifi
    a=wifi.init()
    print(a)
    a=wifi.disconnect()
    print(a)
    a=wifi.version()
    print(a)
    a=wifi.set_mode(3)
    print(a)
    a=wifi.connect("ssid", "password")
    print(a)
    a=wifi.ipconfig()
    print(a)
    a=wifi.multiconnect(1)
    print(a)
    a=wifi.https_get_sd("www.google.com", "yahoo.txt")
    print(a)
    header=["User-Agent: gr-citrus", "Accept: application/json", "Content-type: application/x-www-form-urlencoded"]
    data="test"
    a=wifi.http_post("httpbin.org/post", header, data, "httpbin.txt")
    print(a)
    f = open('httpbin.txt')
    f.read()
    f.close()


Limitations
===========


DAC (digital to analog conversion)
----------------------------------

Not implemented yet.


PWM (pulse width modulation)
----------------------------

Not implemented yet.


RTC (real time clock)
---------------------

Not implemented yet.


Servo control
-------------

Not implemented yet.


CAN bus (controller area network)
---------------------------------

Not planned.

