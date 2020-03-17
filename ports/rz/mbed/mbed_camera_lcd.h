/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
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

#ifndef PORTS_RZ_MBED_MBED_CAMERA_LCD_H_
#define PORTS_RZ_MBED_MBED_CAMERA_LCD_H_

#ifdef __cplusplus
extern "C" {
#endif

/* LCD Parameter */
#define LCD_INPUT_CLOCK                     (66.67)  /* not use */

void mbed_camera_lcd_init(void);
void mbed_camera_lcd_deinit(void);
void mbed_start_video_camera(uint8_t *buf);
void mbed_start_lcd_ycbcr_display(uint8_t *buf);
void mbed_start_lcd_rgb_display(uint8_t *buf);
uint8_t *mbed_get_frame_buffer(void);
int mbed_get_frame_hw(void);
int mbed_get_frame_vw(void);
int mbed_get_frame_pic_size(void);

#ifdef __cplusplus
};
#endif

#endif /* PORTS_RZ_MBED_MBED_CAMERA_LCD_H_ */
