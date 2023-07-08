# GR-ROSE
# I2C
# channel 1: SCL:P12, SDA:P13
# channel 2: SCL:P21, SDA:P20
# channel 3: SCL:P16, SDA:P17
# channel 4: SCL:PC0, SDA:PC1
#
from pyb import I2C

i2c = I2C(2)
i2c = I2C(2, I2C.CONTROLLER)
i2c.scan()

# AXCL345
i2c.is_ready(0x1d)