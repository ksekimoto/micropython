#====================================================================
# LCD 800x480 display sample
#====================================================================

from rz import DISPLAY
from rz import LCD
from rz import CAMERA
from rz import FONT

lcd = LCD(lcd_id=LCD.LCD_800x480)
width = lcd.width()
height = lcd.height()
hs = 10
vs = 10
hw = width - 2 * hs
vw = height - 2 * vs

display0=DISPLAY(font_id=4, format=DISPLAY.G_RGB565, layer_id=0, width=width, height=height, hs=hs, vs=vs, hw=int(hw/2), vw=vw)
display0.start_display()

display0.clear(DISPLAY.Cyan)
display0.box_fill(50,60,220,300,DISPLAY.Yellow)
display0.circle_fill(100,100,50,DISPLAY.Green)
display0.circle_fill(200,200,50,DISPLAY.Red)
display0.line(0,0,100,200,DISPLAY.Blue)
display0.circle(150,200,50,DISPLAY.Blue)

hs2 = hs + int(width/2)
vs2 = vs
hw2 = int(width/4)
vw2 = int(height/4)
display2=DISPLAY(font_id=4, format=DISPLAY.G_RGB565, layer_id=2, width=int(width/4), height=int(height/4), hs=hs2, vs=vs2, hw=hw2, vw=vw2)
display2.start_display()
display2.clear(DISPLAY.Green)
display2.fcol(DISPLAY.Red)
display2.bcol(DISPLAY.White)
display2.pututf8("インターフェース誌\r\n")
display2.pututf8("掲載予定\r\n")

#====================================================================
# LCD 480x272 display sample
#====================================================================

from rz import DISPLAY
from rz import LCD
from rz import CAMERA
from rz import FONT

lcd=LCD(lcd_id=LCD.ATM0430D25)
display0=DISPLAY(font_id=4, format=DISPLAY.G_RGB565, layer_id=0, hs=30, vs=30, hw=400, vw=200)
display0.start_display()

display0.clear(DISPLAY.Red)
display0.line(30,30,200,200,DISPLAY.Blue)
display0.box_fill(50,60,400,480,DISPLAY.Yellow)
display0.circle(150,200,50,DISPLAY.Blue)
display0.circle_fill(100,100,50,DISPLAY.Green)

display0.fcol(DISPLAY.Green)
display0.pututf8("インターフェース誌\r\n")
display0.fcol(DISPLAY.Blue)
display0.pututf8("掲載予定\r\n")

#====================================================================
# Raspi (MIPI) Camera RAW8
#====================================================================

from rz import LCD
from rz import CAMERA
from rz import FONT

camera=CAMERA(camera_id=CAMERA.RASPBERRY_PI_832X480, format=CAMERA.V_RAW8, input_ch=0)
buf_ptr=camera.get_fb_ptr()
lcd1=LCD(lcd_id=DISPLAY.LCD_800x480, font_id=4, format=DISPLAY.G_CLUT8, layer=0, buf_ptr=buf_ptr, stride=832)
camera.StartCamera()

lcd1.fcol(lcd1.Green)
lcd1.pututf8("インターフェース誌\r\n")
lcd1.fcol(lcd1.Blue)
lcd1.pututf8("掲載予定\r\n")

#====================================================================
# OV7725 Camera (GR-LYCHEE) YCBCR422
#====================================================================

from rz import LCD
from rz import CAMERA
from rz import FONT

camera=CAMERA(camera_id=CAMERA.OV7725, format=CAMERA.V_YCBCR422, input_ch=0)
buf_ptr=camera.get_fb_ptr()
lcd1=LCD(lcd_id=DISPLAY.LCD_800x480, font_id=4, format=DISPLAY.G_YCBCR422, layer=0, buf_ptr=buf_ptr, stride=1280)
camera.StartCamera()

lcd1.fcol(lcd1.Green)
lcd1.pututf8("インターフェース誌\r\n")
lcd1.fcol(lcd1.Blue)
lcd1.pututf8("掲載予定\r\n")

#====================================================================
# OV7725 Camera (GR-LYCHEE) 
# DISPLAY.LCD_800x480
#====================================================================

from rz import LCD
from rz import CAMERA
from rz import FONT

camera=CAMERA(camera_id=CAMERA.OV7725, format=CAMERA.V_YCBCR422, input_ch=0)
buf_ptr=camera.get_fb_ptr()
lcd1=LCD(lcd_id=DISPLAY.LCD_800x480, font_id=4, format=DISPLAY.G_YCBCR422, layer=0, buf_ptr=buf_ptr, stride=1280)
camera.StartCamera()

#====================================================================
# OV7725 Camera (GR-LYCHEE)
# DISPLAY.ATM0430D25
#====================================================================

from rz import DISPLAY
from rz import LCD
from rz import CAMERA
from rz import FONT

lcd0=LCD(lcd_id=LCD.ATM0430D25)

display1=DISPLAY(font_id=4, format=DISPLAY.G_YCBCR422, layer_id=0, stride=1280, width=640, height=480, hs=240, vs=0, hw=240, vw=272)
display1.start_display()
buf_ptr=display1.get_fb_ptr()
camera=CAMERA(camera_id=CAMERA.OV7725, format=CAMERA.V_YCBCR422, input_ch=0, buf_ptr=buf_ptr, stride=1280, reset_level=0)
camera.StartCamera()

display0=DISPLAY(font_id=4, format=DISPLAY.G_RGB565, layer_id=3, width=480, height=272, hs=0, vs=0, hw=240, vw=272)
display0.start_display()
display0.clear(DISPLAY.Red)
display0.line(10,10,200,200,DISPLAY.Blue)
display0.box_fill(50,60,200,180,DISPLAY.Yellow)
display0.circle(30,50,50,DISPLAY.Blue)
display0.circle_fill(50,50,50,DISPLAY.Green)

display0.fcol(display0.Green)
display0.pututf8("インターフェース誌\r\n")
display0.fcol(display0.Blue)
display0.pututf8("掲載予定\r\n")

#====================================================================
# OV7725 Camera (GR-LYCHEE)
# DISPLAY.ATM0430D25
#====================================================================

from rz import DISPLAY
from rz import LCD
from rz import CAMERA
from rz import FONT

lcd0=LCD(lcd_id=LCD.LCD_800x480)
width=lcd0.width()
height=lcd0.height()

hs2 = 0
vs2 = 0
hw2 = int(width/2)
vw2 = height

hs3 = int(width/2)
vs3 = 0
hw3 = int(width/2)
vw3 = height

display3=DISPLAY(font_id=4, format=DISPLAY.G_YCBCR422, layer_id=0, stride=1280, width=640, height=480, hs=hs3, vs=vs3, hw=hw3, vw=vw3)
display3.start_display()
buf_ptr3=display3.get_fb_ptr()
camera3=CAMERA(camera_id=CAMERA.OV7725, format=CAMERA.V_YCBCR422, input_ch=0, buf_ptr=buf_ptr3, stride=1280, reset_level=0)
camera3.StartCamera()

display2=DISPLAY(font_id=4, format=DISPLAY.G_CLUT8, layer_id=2, width=832, height=480, hs=hs2, vs=vs2, hw=hw2, vw=vw2)
display2.start_display()
buf_ptr2=display2.get_fb_ptr()
camera2=CAMERA(camera_id=CAMERA.RASPBERRY_PI_832X480, format=CAMERA.V_RAW8, input_ch=0, buf_ptr=buf_ptr2, stride=832, reset_level=0)
camera2.StartCamera()

#====================================================================
# OV7670 Camera
#====================================================================

from rz import DISPLAY
from rz import LCD
from rz import CAMERA
from rz import FONT

lcd0=LCD(lcd_id=LCD.LCD_800x480)
width=lcd0.width()
height=lcd0.height()

hs2 = 0
vs2 = 0
hw2 = int(width/2)
vw2 = height

hs3 = int(width/2)
vs3 = 0
hw3 = int(width/2)
vw3 = height

display3=DISPLAY(font_id=4, format=DISPLAY.G_YCBCR422, layer_id=0, stride=1280, width=640, height=480, hs=hs3, vs=vs3, hw=hw3, vw=vw3)
display3.start_display()
buf_ptr3=display3.get_fb_ptr()
camera3=CAMERA(camera_id=CAMERA.OV7670, format=CAMERA.C_YCBCR422, input_ch=0, buf_ptr=buf_ptr3, stride=1280, reset_level=0)
camera3.start_camera()

display2=DISPLAY(font_id=4, format=DISPLAY.G_CLUT8, layer_id=2, width=832, height=480, hs=hs2, vs=vs2, hw=hw2, vw=vw2)
display2.start_display()
buf_ptr2=display2.get_fb_ptr()
camera2=CAMERA(camera_id=CAMERA.RASPBERRY_PI_832X480, format=CAMERA.C_RAW8, input_ch=0, buf_ptr=buf_ptr2, stride=832, reset_level=0)
camera2.start_camera()
camera2.camera_type2_reg_read(0x20, 0x00)
camera2.camera_type2_reg_read(0x20, 0x01)