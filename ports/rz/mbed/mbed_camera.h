/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Kentaro Sekimoto
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

#ifndef PORTS_RZ_MBED_MBED_CAMERA_H_
#define PORTS_RZ_MBED_MBED_CAMERA_H_

#ifdef __cplusplus
extern "C" {
#endif

#define CAMERA_BUF_ALIGN    0x80

#define DEF_CAMERA_WIDTH    (640)
#define DEF_CAMERA_HEIGHT   (480)

#define CAMERA_DV   0
#define CAMERA_MIPI 1

#define CAMERA_RAS_PI_DEF           0x2000
#define CAMERA_RAS_PI_WIDE_ANGLE    0x2001
#define CAMERA_RAS_PI_832X480       0x2002

#define VFORMAT_YCBCR422 0
#define VFORMAT_RGB565   1
#define VFORMAT_RGB888   2
#define VFORMAT_RAW8     3

#define CFORMAT_RGB888   0
#define CFORMAT_RGB666   1
#define CFORMAT_RGB565   2
#define CFORMAT_BT656    3
#define CFORMAT_BT601    4
#define CFORMAT_YCBCR422 5
#define CFORMAT_YCBCR444 6
#define CFORMAT_RAW8     7

typedef struct _camera_t {
    uint16_t camera_id;
    uint16_t module;
    uint16_t cformat;
    uint16_t hw;
    uint16_t vw;
    uint16_t depth;
    uint16_t stride;
    uint16_t vformat;
    uint16_t swa;
    uint16_t input_ch;
    uint8_t *buf;
    uint32_t reset_level;
} camera_t;

bool mbed_start_video_camera(camera_t *camera);
void mbed_camera_init_params(camera_t *camera);
bool mbed_camera_init(camera_t *camera);
uint16_t camera_get_ov_product_id(uint8_t addr, uint32_t reset_level);

bool camera_type1_reg_tbl_write(uint8_t addr, const uint8_t *tbl, size_t size);
bool camera_type1_reg_write(uint8_t addr, uint8_t reg, uint8_t v);
bool camera_type1_reg_read(uint8_t addr, uint8_t reg, uint8_t *v);
bool camera_type2_reg_tbl_write(uint8_t addr, const uint8_t *tbl, size_t size);
bool camera_type2_reg_write(uint8_t addr, uint16_t reg, uint8_t v);
bool camera_type2_reg_read(uint8_t addr, uint16_t reg, uint8_t *v);

#ifdef __cplusplus
};
#endif

#endif /* PORTS_RZ_MBED_MBED_CAMERA_H_ */
