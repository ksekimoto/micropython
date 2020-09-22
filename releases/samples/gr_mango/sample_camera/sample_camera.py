import pyb

camera = pyb.CAMERA()
camera.StartLcdYcbcr()
camera.StartCameraYcbcr()
camera.JpegSave("test1.jpg")

