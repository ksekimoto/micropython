import rz
import rzreg
import time
from machine import Pin

led1 = Pin.cpu.P01.pin()    # 1
led2 = Pin.cpu.P03.pin()    # 3
led3 = Pin.cpu.P05.pin()    # 5
led4 = Pin.cpu.P82.pin()    # 8*8 + 2

leds = [led1, led2, led3, led4]

def gpio_output(pin):
    port = pin >> 4
    mask1 = 1 << (pin & 7)
    maskd = 3 << ((pin & 7) << 1)
    masko = 3 << ((pin & 7) << 1)
    ppmr = rzreg.PORT0 + 128 + port
    # print("ppmr:" + hex(ppmr))
    ppdr = rzreg.PORT0 + 0 + port * 2
    # print("ppdr:" + hex(ppdr))
    rz.mem8[ppmr] &= ~mask1
    rz.mem16[ppdr] &= ~maskd
    rz.mem16[ppdr] |= masko

def gpio_write(pin, v):
    port = pin >> 4
    mask1 = 1 << (pin & 7)
    ppodr = rzreg.PORT0 + 64 + port
    # print("ppodr:" + hex(ppodr))
    if v != 0:
        rz.mem8[ppodr] |= mask1
    else:
        rz.mem8[ppodr] &= ~mask1

def gpio_toggle(pin):
    port = pin >> 4
    mask1 = 1 << (pin & 7)
    ppodr = rzreg.PORT0 + 64 + port
    rz.mem8[ppodr] ^= mask1

def led_on(pin):
    gpio_output(pin)
    gpio_write(pin, 1)

def led_off(pin):
    gpio_output(pin)
    gpio_write(pin, 0)

def led_toggle(pin):
    gpio_output(pin)
    gpio_toggle(pin)

n = 0
while True:
    n = (n + 1) % 4
    led_toggle(leds[n])
    time.sleep_ms(50)