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

#ifndef PORTS_RZ_CAMERA_LCD_H_
#define PORTS_RZ_CAMERA_LCD_H_

#ifndef __cplusplus
extern const mp_obj_type_t pyb_camera_type;
#endif

#define CAMERA_DV   0
#define CAMERA_MIPI 1

#define GFORMAT_YCBCR422 0
#define GFORMAT_RGB565   1
#define GFORMAT_RGB888   2
#define GFORMAT_ARGB8888 3
#define GFORMAT_ARGB4444 4
#define GFORMAT_CLUT8    5
#define GFORMAT_CLUT4    6
#define GFORMAT_CLUT1    7

#define VFORMAT_YCBCR422 0
#define VFORMAT_RGB565   1
#define VFORMAT_RGB888   2
#define VFORMAT_RAW8     3

#define JFORMAT_YCBCR422 0
#define JFORMAT_ARGB8888 1
#define JFORMAT_RGB565   2

#endif /* PORTS_RZ_CAMERA_LCD_H_ */
