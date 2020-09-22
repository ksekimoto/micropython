import pyb

camera = pyb.CAMERA()
camera.StartLcdDisplay(pyb.CAMERA.G_YCBCR422)
camera.StartCamera(pyb.CAMERA.V_YCBCR422)
camera.JpegSave("test2.jpg", pyb.CAMERA.J_YCBCR422)

# Power off and on
import pyb

camera = pyb.CAMERA()
camera.StartLcdDisplay(pyb.CAMERA.G_YCBCR422)
camera.JpegLoad("test2.jpg", pyb.CAMERA.J_YCBCR422)

camera.StartLcdDisplay(pyb.CAMERA.G_RGB565)
camera.JpegLoad("test2.jpg", pyb.CAMERA.J_RGB565)
