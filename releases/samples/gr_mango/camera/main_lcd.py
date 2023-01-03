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