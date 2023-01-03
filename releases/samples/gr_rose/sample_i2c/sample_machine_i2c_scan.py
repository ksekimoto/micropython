# GR-ROSE
import machine
i2c = machine.I2C(scl=machine.Pin.cpu.P52, sda=machine.Pin.cpu.P50)
print('Scan i2c bus...')
devices = i2c.scan()
if len(devices) == 0:
    print("No i2c device !")
else:
    print('i2c devices found:',len(devices))
for device in devices:
    print("Decimal address: ", device, " | Hexaaddress: ", hex(device))