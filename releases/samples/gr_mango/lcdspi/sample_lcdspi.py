from pyb import Pin
from rz import LCDSPI, FONT

# lcd_id=LCDSPI.M_NOKIA6100_0
# lcd_id=LCDSPI.M_NOKIA6100_1
# lcd_id=LCDSPI.M_T180
# lcd_id=LCDSPI.M_M022C9340SPI
lcd_id=LCDSPI.M_RASPI13LCDSPI
# lcd_id=LCDSPI.M_RASPI28LCDSPI
# lcd_id=LCDSPI.M_KMRTM24024SPI
# lcd_id=LCDSPI.M_KMR18SPI
# lcd_id=LCDSPI.M_ST7735R_G128x160
# lcd_id=LCDSPI.M_ST7735R_R128x160
# lcd_id=LCDSPI.M_ST7735R_G128x128
# lcd_id=LCDSPI.M_ST7735R_G160x80
# lcd_id=LCDSPI.M_ROBOT_LCD
# lcd_id=LCDSPI.M_AIDEEPEN22SPI

# Beta
# spi_id = 0
# cs=Pin.cpu.P84    # pin 24 - GPIO8 
# clk=Pin.cpu.P87   # pin 23 - GPIO11
# dout=Pin.cpu.P86  # pin 19 - GPIO10
# rs=Pin.cpu.PH6    # pin 22 - GPIO25
# reset=Pin.cpu.P45 # pin 13 - GPIO27
# din=Pin.cpu.P85   # pin 21 - GPIO9

spi_id = 2
# spi_id = -1
cs=Pin.cpu.PG7      # pin 24
clk=Pin.cpu.PG4     # pin 23
dout=Pin.cpu.PG5    # pin 19
rs=Pin.cpu.PE0      # pin 22
reset=Pin.cpu.P44   # pin 13   
din=Pin.cpu.PG6     # pin 21

if lcd_id == LCDSPI.M_ROBOT_LCD:
    reset=Pin(20)
    rs=Pin(21)

if lcd_id == LCDSPI.M_T180:
    reset=Pin(20)
    rs=Pin(4)
    din=Pin(16) # assign other than Pin(4)

lcd=LCDSPI(lcd_id=lcd_id, font_id=4, spi_id=spi_id, baud=20000000, cs=cs, clk=clk, dout=dout, rs=rs, reset=reset, din=din)
lcd.clear(lcd.Pink)
font=FONT(4)
print(font)
lcd.box(50, 50, 100, 100, lcd.Green)
lcd.box_fill(30, 60, 90, 120, lcd.Cyan)
lcd.line(50, 50, 100, 100, lcd.Red)
lcd.circle(100, 100, 40, lcd.Blue)
lcd.circle_fill(80, 80, 15, lcd.Yellow)
# lcd.disp_bmp_file(0, 0, "citrus16.bmp")
# lcd.disp_jpeg_file(0, 0, "citrus24.jpg")

lcd.fcol(lcd.Green)
lcd.pututf8("インターフェース誌\r\n")
lcd.fcol(lcd.Blue)
lcd.pututf8("掲載予定\r\n")