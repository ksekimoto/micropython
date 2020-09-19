import lvgl as lv
lv.init()

import lvrz
lvrz.init()

from pyb import Pin
import ILI9341 as ili
# GR-MANGO Beta version
#disp = ili.display(spihost=0, cs=Pin.cpu.P84, dc=Pin.cpu.PH6, rst=Pin.cpu.P45)
# GR-MANGO production version
disp = ili.display(spihost=1, cs=Pin.cpu.PG7, dc=Pin.cpu.PE0, rst=Pin.cpu.P44)
disp.init()

disp_buf1 = lv.disp_buf_t()
buf1_1 = bytearray(480*10)
disp_buf1.init(buf1_1, None, len(buf1_1) // lv.color_t.SIZE)
disp_drv = lv.disp_drv_t()
disp_drv.init()
disp_drv.buffer = disp_buf1
disp_drv.flush_cb = disp.flush
disp_drv.hor_res = 240
disp_drv.ver_res = 320
disp_drv.register()

# Animation helper class

class Anim(lv.anim_t):
    def __init__(self, obj, val, size, exec_cb, path_cb, time=500, playback=False, ready_cb=None):
        super().__init__()
        self.init()
        self.set_time(time)
        self.set_values(val, val + size)
        if callable(exec_cb):
            self.set_custom_exec_cb(exec_cb)
        else:
            self.set_exec_cb(obj, exec_cb)
        path = lv.anim_path_t({"cb": path_cb})
        self.set_path(path)
        if playback:
            self.set_playback(0)
        if ready_cb:
            self.set_ready_cb(ready_cb)
        self.start()

# An animated chart

class AnimatedChart(lv.chart):
    def __init__(self, parent, val, size):
        super().__init__(parent)
        self.val = val
        self.size = size
        self.max = 2000
        self.min = 500
        self.factor = 100
        self.anim_phase1()

    def anim_phase1(self):
        Anim(
            self, 
            self.val, 
            self.size, 
            lambda a, val: self.set_y_range(self.AXIS.PRIMARY_Y,0, val),
            lv.anim_path_t.ease_in, 
            ready_cb=lambda a:self.anim_phase2(),
            time=(self.max * self.factor) // 100,
        )

    def anim_phase2(self):
        Anim(
            self, 
            self.val+self.size, 
            -self.size, 
            lambda a, val: self.set_y_range(self.AXIS.PRIMARY_Y,0, val),
            lv.anim_path_t.ease_out, 
            ready_cb=lambda a:self.anim_phase1(),
            time=(self.min * self.factor) // 100,
        )

# Create the chart

scr = lv.obj()
chart = AnimatedChart(scr, 100, 1000)
chart.set_width(scr.get_width() - 100)
chart.align(scr, lv.ALIGN.CENTER, 0, 0)
series1 = chart.add_series(lv.color_hex(0xFF0000))
chart.set_type(chart.TYPE.LINE)
chart.set_y_range(chart.AXIS.PRIMARY_Y, 0,100)
chart.init_points(series1, 10)
chart.set_points(series1, [10, 20, 30, 20, 10, 40, 50, 90, 95, 90])
chart.set_x_tick_texts("a\nb\nc\nd\ne", 2, lv.chart.AXIS.DRAW_LAST_TICK)
chart.set_x_tick_length(10, 5)
chart.set_y_tick_texts("1\n2\n3\n4\n5", 2, lv.chart.AXIS.DRAW_LAST_TICK)
chart.set_y_tick_length(10, 5)
chart.set_div_line_count(3, 3)
# chart.set_margin(30)

# Create a slider that controls the chart animation speed

def on_slider_changed(self, obj=None, event=-1):
    chart.factor = slider.get_value()


slider = lv.slider(scr)
slider.align(chart, lv.ALIGN.OUT_RIGHT_TOP, 10, 0)
slider.set_width(10)
slider.set_height(chart.get_height())
slider.set_range(10, 200)
slider.set_value(chart.factor, 0)
slider.set_event_cb(on_slider_changed)

# Load the screen

lv.scr_load(scr)
