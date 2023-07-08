# From a book (MicroPython Projects by Jacob Beningo)
import uasyncio as asyncio

LED_GREEN = 1
LED_RED = 2
#LED_YELLOW = 3
#LED_BLUE = 4

async def task1():
     while True:
         pyb.LED(LED_GREEN).toggle()
         await asyncio.sleep_ms(150)

async def task2():
     while True:
         pyb.LED(LED_RED).toggle()
         await asyncio.sleep_ms(150)

pyb.LED(LED_GREEN).on()
pyb.LED(LED_RED).off()

loop = asyncio.get_event_loop()
loop.create_task(task1())
loop.create_task(task2())
loop.run_forever()