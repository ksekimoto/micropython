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

#include <stdint.h>

#include "mbed.h"
#include "rtos.h"
#include "dcache-control.h"
#include "EasyAttach_CameraAndLCD.h"
#include "mbed_camera.h"
#include "r_jcu_api.h"
#include "JPEG_Converter.h"
#include "converter_wrapper.h"
#include "display.h"
#include "mbed_jpeg.h"

#define JPEG_BUF_SIZE   (128 * 1024)

static __attribute((section("NC_BSS"),aligned(32))) uint8_t JpegBuffer[JPEG_BUF_SIZE];

int mbed_jpeg_encode(const char *vbuf, uint16_t wx, uint16_t wy, char **jpeg_buf, uint32_t *encode_size, uint16_t format) {
    JPEG_Converter mbed_jpeg;
    JPEG_Converter::bitmap_buff_info_t buff_info;
    JPEG_Converter::encode_options_t encode_options;
    JPEG_Converter::jpeg_conv_error_t err = JPEG_Converter::JPEG_CONV_JCU_ERR;
    *encode_size = (uint32_t)sizeof(JpegBuffer);

    switch (format) {
        case JFORMAT_ARGB8888:
            buff_info.format = JPEG_Converter::WR_RD_ARGB8888;
            encode_options.input_swapsetting = JPEG_Converter::WR_RD_WRSWA_NON;
            break;
        case JFORMAT_RGB565:
            buff_info.format = JPEG_Converter::WR_RD_RGB565;
            encode_options.input_swapsetting = JPEG_Converter::WR_RD_WRSWA_NON;
            break;
        case JFORMAT_YCBCR422:
        default:
            buff_info.format = JPEG_Converter::WR_RD_YCbCr422;
            encode_options.input_swapsetting = JPEG_Converter::WR_RD_WRSWA_32_16_8BIT;
            break;
    }
    buff_info.buffer_address = (void *)vbuf;
    buff_info.height = (int32_t)wy;
    buff_info.width = (int32_t)wx;
    encode_options.encode_buff_size = sizeof(JpegBuffer);
    encode_options.p_EncodeCallBackFunc = NULL;

    dcache_invalid((void *)&JpegBuffer, (uint32_t)sizeof(JpegBuffer));
    err = mbed_jpeg.encode(&buff_info, (void *)&JpegBuffer, (size_t *)encode_size, &encode_options);
    if (err == JPEG_Converter::JPEG_CONV_OK) {
        *jpeg_buf = (char *)&JpegBuffer;
    }
    return (int)err;
}

int mbed_jpeg_decode(const char *vbuf, uint16_t wx, uint16_t wy, char *jpeg_buf, uint32_t decode_size, uint16_t format) {
    JPEG_Converter mbed_jpeg;
    JPEG_Converter::bitmap_buff_info_t buff_info;
    JPEG_Converter::decode_options_t decode_options;
    JPEG_Converter::jpeg_conv_error_t err = JPEG_Converter::JPEG_CONV_JCU_ERR;

    switch (format) {
        case JFORMAT_ARGB8888:
            buff_info.format = JPEG_Converter::WR_RD_ARGB8888;
            decode_options.output_swapsetting = JPEG_Converter::WR_RD_WRSWA_NON;
            break;
        case JFORMAT_RGB565:
            buff_info.format = JPEG_Converter::WR_RD_RGB565;
            decode_options.output_swapsetting = JPEG_Converter::WR_RD_WRSWA_NON;
            break;
        case JFORMAT_YCBCR422:
        default:
            buff_info.format = JPEG_Converter::WR_RD_YCbCr422;
            decode_options.output_swapsetting = JPEG_Converter::WR_RD_WRSWA_32_16_8BIT;
            break;
    }
    buff_info.buffer_address = (void *)vbuf;
    buff_info.height = (int32_t)wy;
    buff_info.width = (int32_t)wx;
    decode_options.output_cb_cr_offset = JPEG_Converter::CBCR_OFFSET_128;

    if (decode_size <= sizeof(JpegBuffer)) {
        memcpy((void *)&JpegBuffer, (const void *)jpeg_buf, (size_t)decode_size);
        dcache_clean((void *)&JpegBuffer, sizeof(JpegBuffer));
        err = mbed_jpeg.decode((void *)&JpegBuffer, &buff_info, &decode_options);
    }
    return (int)err;
}
