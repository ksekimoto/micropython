from omron2smpb import OMRON2SMPB

sensor = OMRON2SMPB(scl_pin=machine.Pin.cpu.PD2, sda_pin=machine.Pin.cpu.PD3, i2c_address=0x56)

sensor.init()
temperature = sensor.get_temperature()
pressure = sensor.get_pressure()

print('Temperature:', temperature, 'ºC, Pressure:', pressure, 'Pa')