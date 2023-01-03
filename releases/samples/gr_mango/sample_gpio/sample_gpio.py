from pyb import Pin

# LED1
p01_out = Pin('P01', Pin.OUT_PP)
p01_out.high()

# LED2
p03_out = Pin('P03', Pin.OUT_PP)
p03_out.high()

# LED3
p05_out = Pin('P05', Pin.OUT_PP)
p05_out.high()

# LED4
p82_out = Pin('P82', Pin.OUT_PP)
p82_out.high()

# SW1
pd6_in = Pin('PD6', Pin.IN, Pin.PULL_UP)
pd6_in.value() # get value, 0 or 1

# SW2
pd7_in = Pin('PD7', Pin.IN, Pin.PULL_UP)
pd7_in.value() # get value, 0 or 1