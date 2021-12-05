import pyb
led = pyb.LED(1)
while True:
    led.toggle()
    pyb.delay(1000)
