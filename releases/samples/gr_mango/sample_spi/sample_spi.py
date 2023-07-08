import pyb
import max7219
from machine import Pin, SPI
spi= SPI(1)
cs= Pin.cpu.P84
cs.init(cs.OUT, True)
display = max7219.Matrix8x8(spi, cs, 1)
# “1”を座標(0,0)でカラー1で描画
display.text("1",0,0,1)
display.show()
# 消去後、”A”を座標(0,0)にカラー1で描画
pyb.delay(1000)
display.fill(0)
display.show()
display.text("A",0,0,1)
display.show()
#消去後、縦線、横線をカラー1で描画
pyb.delay(1000)
display.fill(0)
display.hline(0,4,8,1)
display.vline(4,0,8,1)
display.show()