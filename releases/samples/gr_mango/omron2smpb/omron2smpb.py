from machine import I2C, Pin
import time

__version__ = '0.0.1'
__author__ = 'Kentaro Sekimoto'
__license__ = "Apache License 2.0. https://www.apache.org/licenses/LICENSE-2.0"

DEFAULT_I2C_ADDRESS = 0x56
# COE parameter index
_b00_1 = 0
_b00_0 = 1
_bt1_1 = 2
_bt1_0 = 3
_bt2_1 = 4
_bt2_0 = 5
_bp1_1 = 6
_bp1_0 = 7
_b11_1 = 8
_b11_0 = 9
_bp2_1 = 10
_bp2_0 = 11
_b12_1 = 12
_b12_0 = 13
_b21_0 = 14
_b21_1 = 15
_bp3_1 = 16
_bp3_0 = 17
_a0_1 = 18
_a0_0 = 19
_a1_1 = 20
_a1_0 = 21
_a2_1 = 22
_a2_0 = 23
_b00_a0_ex = 24

class OMRON2SMPB():
    """
    OMRON 2SMPB sensor driver in pure python based on I2C bus
    """
    MODE_SLEEP = 0
    MODE_FORCE = 1
    MODE_NORMAL = 3
    COE_ADDR_S = 0xa0
    COE_ADDR_E = 0xb8

    def __init__(self, scl_pin=5, sda_pin=4, i2c_address=DEFAULT_I2C_ADDRESS):
        self.i2c = I2C(scl=Pin(scl_pin), sda=Pin(sda_pin))
        self.i2c_addr = i2c_address
        self.coe = bytearray(self.COE_ADDR_E - self.COE_ADDR_S + 1)
        self.a0 = 0.0
        self.a1 = 0.0
        self.a2 = 0.0
        self.b00 = 0.0
        self.bt1 = 0.0
        self.bpl = 0.0
        self.b11 = 0.0
        self.bt2 = 0.0
        self.bp2 = 0.0
        self.b12 = 0.0
        self.b21 = 0.0
        self.bp3 = 0.0

    def init(self, standby_time=4, temp_average=3, press_average=3, power_mode=MODE_NORMAL):
        self.reset()
        time.sleep_ms(50)
        print("id: ", self.get_id())
        self.get_coe_params()
        print("coe: ", self.coe)
        self.set_standby_time(standby_time)
        print("standby time: ", self.get_standby_time())
        self.set_temp_average(temp_average)
        print("temp_average: ", self.get_temp_average())
        self.set_press_average(press_average)
        print("press_average: ", self.get_press_average())
        self.set_power_mode(power_mode)
        print("power_mode: ", self.get_power_mode())
        time.sleep_ms(2000)

    def get_id(self):
        return self.i2c.readfrom_mem(self.i2c_addr, 0xd1, 1)[0]
    
    def reset(self):
        self.i2c.writeto_mem(self.i2c_addr, 0xe0, b'\xe6')
    
    def get_standby_time(self):
        return self.i2c.readfrom_mem(self.i2c_addr, 0xf5, 1)[0] >> 5
    
    def set_standby_time(self, t):
        # 000 1ms
        # 001 5ms
        # 010 50ms
        # 011 250ms
        # 100 500ms
        # 101 1s
        # 110 2s
        # 111 4s
        ba = bytearray(1)
        b = self.i2c.readfrom_mem(self.i2c_addr, 0xf5, 1)
        ba[0] = (b[0] & ~0xe0) | (t << 5)
        self.i2c.writeto_mem(self.i2c_addr, 0xf5, ba)
    
    def get_ctrl_meas(self):
        return self.i2c.readfrom_mem(self.i2c_addr, 0xf4, 1)[0]
    
    def get_temp_average(self):
        return self.i2c.readfrom_mem(self.i2c_addr, 0xf4, 1)[0] >> 5
    
    def set_temp_average(self, t):
        ba = bytearray(1)
        b = self.i2c.readfrom_mem(self.i2c_addr, 0xf4, 1)
        ba[0] = (b[0] & ~0xe0) | (t << 5)
        self.i2c.writeto_mem(self.i2c_addr, 0xf4, ba)
    
    def get_press_average(self):
        b = self.i2c.readfrom_mem(self.i2c_addr, 0xf4, 1)
        return (b[0] >> 2) & 0x7
    
    def set_press_average(self, t):
        ba = bytearray(1)
        b = self.i2c.readfrom_mem(self.i2c_addr, 0xf4, 1)
        ba[0] = (b[0] & ~0x1c) | (t << 2)
        self.i2c.writeto_mem(self.i2c_addr, 0xf4, ba)
    
    def get_power_mode(self):
        b = self.i2c.readfrom_mem(self.i2c_addr, 0xf4, 1)
        return b[0] & 0x3
    
    def set_power_mode(self, t):
        ba = bytearray(1)
        b = self.i2c.readfrom_mem(self.i2c_addr, 0xf4, 1)
        ba[0] = (b[0] & ~0x03) | t
        self.i2c.writeto_mem(self.i2c_addr, 0xf4, ba)
    
    def to_f16(self, b0, b1):
        i = b0 | (b1 << 8)
        i = (i & 0x7fff) - (i & 0x8000) 
        #print("{0}:{1} i={2}".format(hex(b1), hex(b0), i))
        return float(i)
    
    def to_f20(self, b0, b1, b2):
        i = b0 | (b1 << 4) | (b2 << 12)
        i = (i & 0x7ffff) - (i & 0x80000) 
        #print("{0}:{1}:{2} i={3}".format(hex(b2), hex(b1), hex(b0), i))
        return float(i)
    
    def to_f24(self, b0, b1, b2):
        i = b0 | (b1 << 8) | (b2 << 16)
        i = (i & 0x7fffff) - (i & 0x800000) 
        #print("{0}:{1}:{2} i={3}".format(hex(b2), hex(b1), hex(b0), i))
        return float(i)
    
    def get_raw_temperature(self):
        t_txd0 = self.i2c.readfrom_mem(self.i2c_addr, 0xfc, 1)
        t_txd1 = self.i2c.readfrom_mem(self.i2c_addr, 0xfb, 1)
        t_txd2 = self.i2c.readfrom_mem(self.i2c_addr, 0xfa, 1)
        i = ((t_txd2[0] << 16) | (t_txd1[0] << 8) | t_txd0[0]) - (2**23)
        return float(i)
    
    def get_raw_pressure(self):
        p_txd0 = self.i2c.readfrom_mem(self.i2c_addr, 0xf9, 1)
        p_txd1 = self.i2c.readfrom_mem(self.i2c_addr, 0xf8, 1)
        p_txd2 = self.i2c.readfrom_mem(self.i2c_addr, 0xf7, 1)
        i = ((p_txd2[0] << 16) | (p_txd1[0] << 8) | p_txd0[0]) - (2**23)
        return float(i)
    
    def get_device_stat(self):
        return self.i2c.readfrom_mem(self.i2c_addr, 0xf3, 1)[0]
    
    def get_coe_params(self):
        addr = self.COE_ADDR_S
        i = 0
        while (addr <= self.COE_ADDR_E):
            self.coe[i] = self.i2c.readfrom_mem(self.i2c_addr, addr, 1)[0]
            addr += 1
            i += 1
        self.a0 = self.to_f20(self.coe[_b00_a0_ex] & 0xf, self.coe[_a0_0], self.coe[_a0_1]) / 16.0
        self.a1 = float('-6.3E-03') + ((float('4.3E-04') * self.to_f16(self.coe[_a1_0], self.coe[_a1_1])) / 32767.0)
        self.a2 = float('-1.9E-11') + ((float('1.2E-10') * self.to_f16(self.coe[_a2_0], self.coe[_a2_1])) / 32767.0)
        self.b00 = self.to_f20(self.coe[_b00_a0_ex] >> 4, self.coe[_b00_0], self.coe[_b00_1]) / 16.0
        self.bt1 = float('1.0E-01') + ((float('9.1E-02') * self.to_f16(self.coe[_bt1_0], self.coe[_bt1_1])) / 32767.0)
        self.bt2 = float('1.2E-08') + ((float('1.2E-06') * self.to_f16(self.coe[_bt2_0], self.coe[_bt2_1])) / 32767.0)
        self.bp1 = float('3.3E-02') + ((float('1.9E-02') * self.to_f16(self.coe[_bp1_0], self.coe[_bp1_1])) / 32767.0)
        self.b11 = float('2.1E-07') + ((float('1.4E-07') * self.to_f16(self.coe[_b11_0], self.coe[_b11_1])) / 32767.0)
        self.bp2 = float('-6.3E-10') + ((float('3.5E-10') * self.to_f16(self.coe[_bp2_0], self.coe[_bp2_1])) / 32767.0)
        self.b12 = float('2.9E-13') + ((float('7.6E-13') * self.to_f16(self.coe[_b12_0], self.coe[_b12_1])) / 32767.0)
        self.b21 = float('2.1E-15') + ((float('1.2E-14') * self.to_f16(self.coe[_b21_0], self.coe[_b21_1])) / 32767.0)
        self.bp3 = float('1.3E-16') + ((float('7.9E-17') * self.to_f16(self.coe[_bp3_0], self.coe[_bp3_1])) / 32767.0)
        return
    
    def get_temperature(self):
        dt = self.get_raw_temperature()
        tr = self.a0 + self.a1 * dt + self.a2 * dt * dt
        #print("tr/256.0: ", tr/256.0)
        return tr/256.0
    
    def get_pressure(self):
        dt = self.get_raw_temperature()
        #print("dt: ", dt)
        tr = self.a0 + self.a1 * dt + self.a2 * dt * dt
        dp = self.get_raw_pressure()
        #print("dp: ", dp)
        p = self.b00 + self.bt1*tr + self.bp1*dp + self.b11*dp*tr + self.bt2*tr*tr + self.bp2*dp*dp + self.b12*dp*tr*tr + self.b21*dp*dp*tr + self.bp3*dp*dp*dp
        #print("p: ", p)
        return p

