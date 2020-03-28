/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Kentaro Sekimoto
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "mbed.h"
#include "dcache-control.h"
#include "EasyAttach_CameraAndLCD.h"
#include "mbed_camera_lcd.h"

#define FRAME_BUFFER_STRIDE    (((LCD_PIXEL_WIDTH * DATA_SIZE_PER_PIC) + 31u) & ~31u)
#define FRAME_BUFFER_HEIGHT    (LCD_PIXEL_HEIGHT)

static uint8_t user_frame_buffer0[FRAME_BUFFER_STRIDE * FRAME_BUFFER_HEIGHT]__attribute((section("NC_BSS"),aligned(32)));
static uint16_t lcd_hw = LCD_PIXEL_WIDTH;
static uint16_t lcd_vw = LCD_PIXEL_HEIGHT;
static uint16_t video_hw = VIDEO_PIXEL_HW;
static uint16_t video_vw = VIDEO_PIXEL_VW;
static uint16_t video_pic_size = DATA_SIZE_PER_PIC;

//static DisplayBase::video_input_channel_t channel;
static DisplayBase *mbed_display;

void mbed_ticker_thread(void *thread, uint32_t ms) {
    static Ticker ticker;
    ticker.attach_us((void (*)())thread, ms);
}

void mbed_set_pixel(int x, int y, uint16_t color) {
    int i = (((LCD_PIXEL_WIDTH) + 31u) & ~31u) * y + x;
    uint16_t *p = (uint16_t*)user_frame_buffer0;
    p[i] = color;
}

uint8_t *mbed_get_fb_ptr(void) {
    return (uint8_t *)&user_frame_buffer0;
}

uint32_t mbed_get_fb_size(void) {
    return (uint32_t)(FRAME_BUFFER_STRIDE * FRAME_BUFFER_HEIGHT);
}

int mbed_get_lcd_hw(void) {
    return lcd_hw;
}

int mbed_get_lcd_vw(void) {
    return lcd_vw;
}

int mbed_get_video_hw(void) {
    return video_hw;
}

int mbed_get_video_vw(void) {
    return video_vw;
}

int mbed_get_lcd_pic_size(void) {
    return video_pic_size;
}

void mbed_start_video_camera(uint8_t *buf) {
    mbed_display->Video_Write_Setting(
        DisplayBase::VIDEO_INPUT_CHANNEL_0,
        DisplayBase::COL_SYS_NTSC_358,
        (void *)buf,
        FRAME_BUFFER_STRIDE,
        DisplayBase::VIDEO_FORMAT_YCBCR422,
        DisplayBase::WR_RD_WRSWA_32_16BIT,
        VIDEO_PIXEL_VW,
        VIDEO_PIXEL_HW
    );
    EasyAttach_CameraStart(*mbed_display, DisplayBase::VIDEO_INPUT_CHANNEL_0);
}

void mbed_start_lcd_ycbcr_display(uint8_t *buf) {
   DisplayBase::rect_t rect;

   rect.vs = 0;
   rect.vw = LCD_PIXEL_HEIGHT;
   rect.hs = 0;
   rect.hw = LCD_PIXEL_WIDTH;
   mbed_display->Graphics_Read_Setting(
       DisplayBase::GRAPHICS_LAYER_0,
       (void *)buf,
       FRAME_BUFFER_STRIDE,
       DisplayBase::GRAPHICS_FORMAT_YCBCR422,
       DisplayBase::WR_RD_WRSWA_32_16BIT,
       &rect
   );
   mbed_display->Graphics_Start(DisplayBase::GRAPHICS_LAYER_0);

   //ThisThread::sleep_for(50);
   wait_ms(50);
   EasyAttach_LcdBacklight(true);
}

void mbed_start_lcd_rgb_display(uint8_t *buf) {
   DisplayBase::rect_t rect;

   for (uint32_t i = 0; i < sizeof(user_frame_buffer0); i += 2) {
       user_frame_buffer0[i + 0] = 0x00;
       user_frame_buffer0[i + 1] = 0x00;
   }
   dcache_clean(user_frame_buffer0, sizeof(user_frame_buffer0));
   rect.vs = 0;
   rect.vw = LCD_PIXEL_HEIGHT;
   rect.hs = 0;
   rect.hw = LCD_PIXEL_WIDTH;
   mbed_display->Graphics_Read_Setting(
       DisplayBase::GRAPHICS_LAYER_0,
       (void *)buf,
       FRAME_BUFFER_STRIDE,
       DisplayBase::GRAPHICS_FORMAT_RGB565,
       DisplayBase::WR_RD_WRSWA_32_16BIT,
       &rect
   );
   mbed_display->Graphics_Start(DisplayBase::GRAPHICS_LAYER_0);
   wait_ms(50);
   EasyAttach_LcdBacklight(true);
}

void mbed_lcd_init(void) {
    mbed_display = new DisplayBase();
    EasyAttach_Init(*mbed_display);
    mbed_start_lcd_rgb_display((uint8_t *)&user_frame_buffer0);
}

void mbed_lcd_deinit(void) {
    if (mbed_display) {
        delete mbed_display;
    }
}

void mbed_camera_lcd_init(void) {
    mbed_display = new DisplayBase();
    EasyAttach_Init(*mbed_display);
}

void mbed_camera_lcd_deinit(void) {
    if (mbed_display) {
        delete mbed_display;
    }
}

