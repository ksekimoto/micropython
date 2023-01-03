#====================================================================
# OV7725 Camera (640x480)
# RasPi Camera (832x480)
# LCD_800x480
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
camera3.start_camera()

display2=DISPLAY(font_id=4, format=DISPLAY.G_CLUT8, layer_id=2, width=832, height=480, hs=hs2, vs=vs2, hw=hw2, vw=vw2)
display2.start_display()
buf_ptr2=display2.get_fb_ptr()
camera2=CAMERA(camera_id=CAMERA.RASPBERRY_PI_832X480, format=CAMERA.V_RAW8, input_ch=0, buf_ptr=buf_ptr2, stride=832, reset_level=0)
camera2.start_camera()