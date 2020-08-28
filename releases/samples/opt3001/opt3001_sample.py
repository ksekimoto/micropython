import machine
import time
from OPT3001 import OPT3001
i2c = machine.I2C(scl=machine.Pin.cpu.PD2, sda=machine.Pin.cpu.PD3)
opt = OPT3001(i2c)
while True:
    opt.measure()
    while not opt.is_ready():
        time.sleep_ms(100)
    print(opt.lux())
