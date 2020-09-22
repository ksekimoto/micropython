import pyb

camera = pyb.CAMERA()
camera.StartLcdDisplay(pyb.CAMERA.V_YCBCR422)
camera.StartCamera(pyb.CAMERA.G_YCBCR422)
camera.JpegSave("test2.jpg")

# Power off and on
import pyb

camera = pyb.CAMERA()
camera.StartLcdDisplay(pyb.CAMERA.V_YCBCR422)
camera.JpegLoad("test2.jpg")

