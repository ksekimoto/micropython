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

#ifndef PORTS_RZ_CAMERA_LCD_H_
#define PORTS_RZ_CAMERA_LCD_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(CAMERA_CVBS)
#undef CAMERA_CVBS
#endif
#if defined(CAMERA_MT9V111)
#undef CAMERA_MT9V111
#endif
#if defined(CAMERA_OV7725)
#undef CAMERA_OV7725
#endif
#if defined(CAMERA_OV5642)
#undef CAMERA_OV5642
#endif
#if defined(CAMERA_OV2640)
#undef CAMERA_OV2640
#endif
#if defined(CAMERA_OV7670)
#undef CAMERA_OV7670
#endif
#if defined(CAMERA_WIRELESS_CAMERA)
#undef CAMERA_WIRELESS_CAMERA
#endif
#if defined(CAMERA_RASPBERRY_PI)
#undef CAMERA_RASPBERRY_PI
#endif
#if defined(CAMERA_RASPBERRY_PI_WIDE_ANGLE)
#undef CAMERA_RASPBERRY_PI_WIDE_ANGLE
#endif
#if defined(CAMERA_RASPBERRY_PI_832X480)
#undef CAMERA_RASPBERRY_PI_832X480
#endif
#define CAMERA_CVBS                     0x0001
#define CAMERA_MT9V111                  0x0002
#define CAMERA_OV7725                   0x0003
#define CAMERA_OV5642                   0x0004
#define CAMERA_OV2640                   0x0005
#define CAMERA_OV7670                   0x0006
#define CAMERA_WIRELESS_CAMERA          0x0083
#define CAMERA_RASPBERRY_PI             0x2000
#define CAMERA_RASPBERRY_PI_WIDE_ANGLE  0x2001
#define CAMERA_RASPBERRY_PI_832X480     0x2002

#define CAMERA_RESET_ACTIVE_LOW     0
#define CAMERA_RESET_ACTIVE_HIGH    1

extern const mp_obj_type_t rz_camera_type;

#ifdef __cplusplus
};
#endif

#endif /* PORTS_RZ_CAMERA_LCD_H_ */
