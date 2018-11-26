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

                    Func    Jtag    Pin Int PWM(MTU)TPU(Servo)
    PIN0    P20     SCI0-TX         ExtInt  MTIOC1A TIOCB3
    PIN1    P21     SCI0-RX         ExtInt          TIOCA3
    PIN2    PC0(P31)SD-SW   (TMS)   ExtInt  MTIOC3C 
    PIN3    PC1(P30)SD-CS   (TDI)   ExtInt  MTIOC3C 
    PIN4    PC2                     
    PIN5    P50(P34)WIFI-EN TRST
    PIN6    P52(P55)                     
    PIN7    P32     SCI6-TX         ExtInt  MTIOC0C 
    PIN8    P33     SCI6-RX TDO     ExtInt
    PIN9    P05(P26)DA              ExtInt (MTIOC2A)
    PIN10   PC4                                     TIOCC6(2)
    PIN11   PC6     SPI-MOSI        ExtInt  MTIOC3C TIOCA6
    PIN12   PC7     SPI-MISO        ExtInt  MTIOC3A TIOCB6
    PIN13   PC5     SPI-CLK                         TIOCD6
    PIN14   P40(P27)AD0     (TCK)   ExtInt
    PIN15   P41(PB3)AD1             ExtInt (MTIOC0A)(TICOD3)(3)
    PIN16   P42(PB5)AD2             ExtInt          (TICOB4)(4)
    PIN17   P43(PE1)AD3             ExtInt (MTIOC4C)(TIOCA9)
    PIN18   P12                     ExtInt
    PIN19   P13(P15)                ExtInt          TIOCA5(1)
    NMI P35
    LED PA0                 MTIOC4A


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


PWM (pulse width modulation)
----------------------------

Sample::

    pin7=pyb.Pin(pyb.Pin('PIN7'))
    pwm=pyb.PWM(pin7)

    led=pyb.Pin(pyb.Pin('LED'))
    pwm=pyb.PWM(led)
    pwm.freq(50)
    pwm.duty(10)


DAC (digital to analog conversion)
----------------------------------

Sample::

    dac = pyb.DAC(pyb.Pin('PIN9'))
    dac.write(512)
    dac.write(256)


RTC (real time clock)
---------------------

Sample::

    rtc = pyb.RTC()
    rtc.init()
    rtc.datetime((2018,11,8,5,10,10,10,0))
    rtc.datetime()
    (2018, 11, 8, 5, 10, 10, 14, 0)


Servo control
-------------

Servo(1): PIN19
Servo(2): PIN10

Sample::

    servo1 = pyb.Servo(1)
    servo1.angle(45)


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


CAN bus (controller area network)
---------------------------------

Not planned.

