class OPT3001:
    def __init__(self, i2c, addr=69):
        self.i2c = i2c
        self.addr = addr

    def is_ready(self):
        return bool(self.i2c.readfrom_mem(self.addr, 0x01, 2)[1] & 0x80)

    def measure(self):
        self.i2c.writeto_mem(self.addr, 0x01, b'\xca\x10')

    def lux(self):
        data = self.i2c.readfrom_mem(self.addr, 0, 2)
        return 0.01 * 2 ** (data[0] >> 4) * ((data[0] & 0x0f) << 8 | data[1])
