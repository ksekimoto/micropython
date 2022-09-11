/*
 * Copyright (c) 2021, Kentaro Sekimoto
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  -Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *  -Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _XPT2046_H_
#define _XPT2046_H_

#ifdef __cplusplus
extern "C" {
#endif

#define XPT2046_BAUDRATE   1000000
#define XPT2046_AVG 4
#define CMD_X_READ  0b10010000
#define CMD_Y_READ  0b11010000

typedef void (*xpt2046_gpio_output_t)(uint32_t pin_id);
typedef void (*xpt2046_gpio_input_t)(uint32_t pin_id);
typedef void (*xpt2046_gpio_write_t)(uint32_t pin_id, bool v);
typedef bool (*xpt2046_gpio_read_t)(uint32_t pin_id);
typedef void (*xpt2046_spi_init_t)(void);
typedef void (*xpt2046_spi_transfer_t)(size_t len, const uint8_t *src, uint8_t *dest);

/*
 * XPT2046 information
 */
typedef struct _xpt2046_t {
    uint32_t baudrate;
    uint32_t spi_id;
    uint32_t mode;
    uint32_t mosi_id;
    uint32_t miso_id;
    uint32_t clk_id;
    uint32_t cs_id;
    uint32_t irq_id;

    xpt2046_gpio_output_t gpio_output;
    xpt2046_gpio_input_t gpio_input;
    xpt2046_gpio_write_t gpio_write;
    xpt2046_gpio_read_t gpio_read;
    xpt2046_spi_init_t spi_init;
    xpt2046_spi_transfer_t spi_transfer;

    int16_t x_res;
    int16_t y_res;
    int16_t x_min;
    int16_t y_min;
    int16_t x_max;
    int16_t y_max;
    bool x_inv;
    bool y_inv;
    bool xy_swap;
    bool normalize;
    int16_t avg_buf_x[XPT2046_AVG];
    int16_t avg_buf_y[XPT2046_AVG];
    uint8_t avg_last;
} xpt2046_t;

typedef struct _xpt2046_data_t {
    int16_t x;
    int16_t y;
    bool valid;
} xpt2046_data_t;

void xpt2046_init(xpt2046_t *xpt2046);
void xpt2046_deinit(xpt2046_t *xpt2046);
void xpt2046_activate(xpt2046_t *xpt2046);
void xpt2046_read(xpt2046_t *xpt2046, xpt2046_data_t *data);

#ifdef __cplusplus
}
#endif

#endif /* _XPT2046_H_ */
