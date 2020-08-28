from sht30 import SHT30

sensor = SHT30(scl_pin=machine.Pin.cpu.PD2, sda_pin=machine.Pin.cpu.PD3, i2c_address=0x44)

temperature, humidity = sensor.measure()

print('Temperature:', temperature, 'ºC, RH:', humidity, '%')