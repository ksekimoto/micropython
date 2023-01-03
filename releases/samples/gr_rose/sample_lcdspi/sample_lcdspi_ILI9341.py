from machine import Pin
from pyb import LCDSPI, FONT
import time

cs=Pin("SPI_SS")
dout=Pin("SPI_MO")
din=Pin("SPI_MI")
clk=Pin("SPI_CK")
reset=Pin(Pin.cpu.P26)
rs=Pin(Pin.cpu.P30)

lcd_id=LCDSPI.M_AIDEEPEN22SPI
lcd=LCDSPI(lcd_id=lcd_id, font_id=4, spi_id=2, baud=15000000, cs=cs, clk=clk, dout=dout, rs=rs, reset=reset, din=din, dir=LCDSPI.ROTATE_0)
lcd.clear(lcd.Pink)
font=FONT(4)
print(font)
lcd.box(50, 50, 100, 100, lcd.Green)
lcd.box_fill(30, 60, 90, 120, lcd.Cyan)
lcd.line(50, 50, 100, 100, lcd.Red)
lcd.circle(100, 100, 40, lcd.Blue)
lcd.circle_fill(80, 80, 15, lcd.Yellow)

lcd.puts_xy(50, 50, "Hello")
lcd.pututf8_xy(20, 80, "Helloこんにちは。")

lcd.scroll(10)
lcd.puts("Hello\r\n")

lcd.clear()
loop = 0
while True:
    lcd.puts("Hello-" + str(loop) + "\r\n")
    time.sleep(1)
    loop += 1
    if loop > 100:
        break