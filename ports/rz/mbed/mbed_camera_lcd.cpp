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
#include  "r_jcu_api.h"
#include  "JPEG_Converter.h"
#include  "converter_wrapper.h"

/* Video Parameter */
#define VIDEO_PIXEL_HW      (640)
#define VIDEO_PIXEL_VW      (480)
#define DATA_SIZE_PER_PIC      (2u)

#define CAMERA_PIXEL_HW 640
#define CAMERA_PIXEL_VW 480
#define CAMERA_BUFFER_STRIDE    (((CAMERA_PIXEL_HW * DATA_SIZE_PER_PIC) + 31u) & ~31u)
#define CAMERA_BUFFER_HEIGHT    (CAMERA_PIXEL_VW)
static uint8_t camera_frame_buf[CAMERA_BUFFER_STRIDE * CAMERA_BUFFER_HEIGHT]__attribute((section("NC_BSS"),aligned(32)));

#define LCD_BUFFER_STRIDE    (((LCD_PIXEL_WIDTH * DATA_SIZE_PER_PIC) + 31u) & ~31u)
#define LCD_BUFFER_HEIGHT    (LCD_PIXEL_HEIGHT)

static uint8_t lcd_frame_buf[LCD_BUFFER_STRIDE * LCD_BUFFER_HEIGHT]__attribute((section("NC_BSS"),aligned(32)));

static uint16_t lcd_hw = LCD_PIXEL_WIDTH;
static uint16_t lcd_vw = LCD_PIXEL_HEIGHT;
static uint16_t camera_hw = VIDEO_PIXEL_HW;
static uint16_t camera_vw = VIDEO_PIXEL_VW;
static uint16_t video_pic_size = DATA_SIZE_PER_PIC;

static DisplayBase *mbed_display;
static JPEG_Converter *mbed_jpeg;

void mbed_jpeg_encode(const char *camera, uint32_t wx, uint32_t wy, char *jpeg_buf, uint32_t *encode_size) {
    JPEG_Converter::bitmap_buff_info_t buff_info;
    JPEG_Converter::jpeg_conv_error_t err;

    buff_info.buffer_address = (void *)camera;
    buff_info.height = (int32_t)wy;
    buff_info.width = (int32_t)wx;
    buff_info.format = JPEG_Converter::WR_RD_YCbCr422;
    err = mbed_jpeg->encode(&buff_info, (void*)jpeg_buf, (size_t*)encode_size);
}

void mbed_ticker_thread(void *thread, uint32_t us) {
    static Ticker ticker;
    ticker.attach_us((void (*)())thread, us);
}

void mbed_set_pixel(int x, int y, uint16_t color) {
    int i = (((lcd_hw) + 31u) & ~31u) * y + x;
    uint16_t *p = (uint16_t*)lcd_frame_buf;
    p[i] = color;
}

uint8_t *mbed_get_camera_fb_ptr(void) {
    return (uint8_t *)&camera_frame_buf;
}

uint32_t mbed_get_camera_fb_size(void) {
    return (uint32_t)(CAMERA_BUFFER_STRIDE * CAMERA_BUFFER_HEIGHT);
}

uint8_t *mbed_get_lcd_fb_ptr(void) {
    return (uint8_t *)&lcd_frame_buf;
}

uint32_t mbed_get_lcd_fb_size(void) {
    return (uint32_t)(LCD_BUFFER_STRIDE * LCD_BUFFER_HEIGHT);
}

int mbed_get_lcd_hw(void) {
    return lcd_hw;
}

int mbed_get_lcd_vw(void) {
    return lcd_vw;
}

int mbed_get_camera_hw(void) {
    return camera_hw;
}

int mbed_get_camera_vw(void) {
    return camera_vw;
}

int mbed_get_lcd_pic_size(void) {
    return video_pic_size;
}

void mbed_start_video_camera(uint8_t *buf) {
    mbed_display->Video_Write_Setting(
        DisplayBase::VIDEO_INPUT_CHANNEL_0,
        DisplayBase::COL_SYS_NTSC_358,
        (void *)buf,
        CAMERA_BUFFER_STRIDE,
        DisplayBase::VIDEO_FORMAT_YCBCR422,
        DisplayBase::WR_RD_WRSWA_32_16BIT,
        camera_vw,
        camera_hw
    );
    EasyAttach_CameraStart(*mbed_display, DisplayBase::VIDEO_INPUT_CHANNEL_0);
}

void mbed_start_lcd_ycbcr_display(uint8_t *buf) {
   DisplayBase::rect_t rect;

   for (uint32_t i = 0; i < sizeof(camera_frame_buf); i += 2) {
       camera_frame_buf[i + 0] = 0x00;
       camera_frame_buf[i + 1] = 0x00;
   }
   dcache_clean(camera_frame_buf, sizeof(camera_frame_buf));
   rect.vs = 0;
   rect.vw = camera_vw;
   rect.hs = 0;
   rect.hw = camera_hw;
   mbed_display->Graphics_Read_Setting(
       DisplayBase::GRAPHICS_LAYER_0,
       (void *)buf,
       CAMERA_BUFFER_STRIDE,
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

   for (uint32_t i = 0; i < sizeof(lcd_frame_buf); i += 2) {
       lcd_frame_buf[i + 0] = 0x00;
       lcd_frame_buf[i + 1] = 0x00;
   }
   dcache_clean(lcd_frame_buf, sizeof(lcd_frame_buf));
   rect.vs = 0;
   rect.vw = lcd_vw;
   rect.hs = 0;
   rect.hw = lcd_hw;
   mbed_display->Graphics_Read_Setting(
       DisplayBase::GRAPHICS_LAYER_0,
       (void *)buf,
       LCD_BUFFER_STRIDE,
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
    mbed_jpeg = new JPEG_Converter();

    EasyAttach_Init(*mbed_display);
    mbed_start_lcd_rgb_display((uint8_t *)lcd_frame_buf);
}

void mbed_lcd_deinit(void) {
    if (mbed_display) {
        delete mbed_display;
    }
    if (mbed_jpeg) {
        delete mbed_jpeg;
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

