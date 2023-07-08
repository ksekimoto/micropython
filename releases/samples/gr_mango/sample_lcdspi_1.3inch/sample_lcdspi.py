from pyb import LCDSPI, FONT, Pin

c=LCDSPI(lcd_id=LCDSPI.M_RASPI13LCDSPI,font_id=FONT.MISAKIA_12,spi_id=1,baud=24000000,cs=Pin.cpu.P84,clk=Pin.cpu.P87,dout=Pin.cpu.P86, rs=Pin.cpu.PH6, reset=Pin.cpu.P45, din=Pin.cpu.P85)
c.puts("============================\r\n")
c.puts("===== 240x240 LCD Demo =====\r\n")
c.puts("============================\r\n")


import pyb
import os
sd=pyb.SDCard()
os.mount(sd, "/sd")
os.listdir("/sd")

from pyb import LCDSPI, FONT, Pin
c=LCDSPI(lcd_id=LCDSPI.M_RASPI28LCDSPI,font_id=FONT.MISAKIA_12,spi_id=1,baud=12000000,cs=Pin.cpu.P84,clk=Pin.cpu.P87,dout=Pin.cpu.P86, rs=Pin.cpu.PH6, reset=Pin.cpu.P30, din=Pin.cpu.P85)
c.disp_jpeg_sd(0,0,'/sd/GR_MANGO.jpg')
c.disp_bmp_sd(0,0,'/sd/GR_MANGO.bmp')

c.puts("============================\r\n")
c.puts("===== 320x240 LCD Demo =====\r\n")
c.puts("============================\r\n")
c.disp_jpeg_sd(0,0,'/sd/GR_MANGO.jpg')
c.disp_bmp_sd(0,0,'/sd/GR_MANGO.bmp')
c.disp_bmp_sd(0,0,'/sd/GR_MANGO.bmp')

c.disp_jpeg_sd(0,0,'/sd/MIF1204.jpg')