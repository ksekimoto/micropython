from pyb import Pin

# LED1
pa0_out = Pin('PA0', Pin.OUT_PP)
pa0_out.high()

# LED2
pa1_out = Pin('PA1', Pin.OUT_PP)
pa1_out.high()