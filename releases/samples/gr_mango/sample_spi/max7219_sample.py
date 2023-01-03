import max7219
from machine import Pin, SPI
spi = SPI(1)
cs = Pin.cpu.P84
cs.init(cs.OUT, True)
display = max7219.Matrix8x8(spi, cs, 1)
display.text("1",0,0,1)
display.show()

pyb.delay(1000)
display.fill(0)
display.show()
display.text("A",0,0,1)
display.show()

pyb.delay(1000)
display.fill(0)
display.hline(0,4,8,1)
display.vline(4,0,8,1)
display.show()