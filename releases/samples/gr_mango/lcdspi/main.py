from machine import Pin
from rp2 import LCDSPI, FONT

cs=Pin(5)
reset=Pin(21)
rs=Pin(20)
dout=Pin(3)
din=Pin(4)
clk=Pin(2)
lcd_id=8
lcd_id=lcd.M_KMRTM24024SPI
lcd=LCDSPI(lcd_id=lcd_id, font_id=4, spi_id=0, baud=8000000, cs=cs, clk=clk, dout=dout, rs=rs, reset=reset, din=din)
lcd.clear(lcd.Pink)
font=FONT(4)
print(font)
lcd.box(50, 50, 100, 100, lcd.Green)
lcd.box_fill(30, 60, 90, 120, lcd.Cyan)
lcd.line(50, 50, 100, 100, lcd.Red)
lcd.circle(100, 100, 40, lcd.Blue)
lcd.circle_fill(80, 80, 15, lcd.Yellow)
# lcd.disp_bmp_file(0, 0, "citrus16.bmp")
lcd.disp_jpeg_file(0, 0, "citrus24.jpg")

lcd.fcol(lcd.Green)
lcd.pututf8("インターフェース誌\r\n")
lcd.fcol(lcd.Blue)
lcd.pututf8("掲載予定\r\n")

# lcd_id=LCDSPI.M_T180
# lcd_id=LCDSPI.M_M022C9340SPI
# lcd_id=lcd.M_KMRTM24024SPI