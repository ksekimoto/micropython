from ws2812 import WS2812
chain = WS2812(spi_bus=1, led_count=12)
data = [
(255, 0, 0),    # red
(0, 255, 0),    # green
(0, 0, 255),    # blue
(85, 85, 85),   # white
(255, 0, 0),    # red
(0, 255, 0),    # green
(0, 0, 255),    # blue
(85, 85, 85),   # white
(255, 0, 0),    # red
(0, 255, 0),    # green
(0, 0, 255),    # blue
(85, 85, 85),   # white
]
chain.show(data)
