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

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "xpt2046.h"

#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#ifdef PIN_NONE
#undef PIN_NONE
#endif
#define PIN_NONE 0xffff

// uint32_t disable_irq(void);
// void enable_irq(uint32_t);

static xpt2046_gpio_output_t m_gpio_output;
static xpt2046_gpio_input_t m_gpio_input;
static xpt2046_gpio_write_t m_gpio_write;
static xpt2046_gpio_read_t m_gpio_read;
static xpt2046_spi_init_t m_spi_init;
static xpt2046_spi_transfer_t m_spi_transfer;
static bool m_normalize = true;

static inline void xpt2046_gpio_set_input(uint32_t pin) {
    if (pin != PIN_NONE) {
        (*m_gpio_input)(pin);
    }
}

static inline void xpt2046_gpio_set_output(uint32_t pin) {
    if (pin != PIN_NONE) {
        (*m_gpio_output)(pin);
    }
}

static inline void xpt2046_gpio_write(uint32_t pin, bool level) {
    if (pin != PIN_NONE) {
        (*m_gpio_write)(pin, level);
    }
}

static inline bool xpt2046_gpio_read(uint32_t pin) {
    if (pin != PIN_NONE) {
        return (*m_gpio_read)(pin);
    } else {
        return false;
    }
}

static void xpt2046_set_pin_mode(xpt2046_t *xpt2046) {
    // set gpio mode for spi pins
    if (xpt2046->clk_id != PIN_NONE) {
        xpt2046_gpio_set_output(xpt2046->clk_id);
    }
    if (xpt2046->mosi_id != PIN_NONE) {
        xpt2046_gpio_set_output(xpt2046->mosi_id);
    }
    if (xpt2046->miso_id != PIN_NONE) {
        xpt2046_gpio_set_input(xpt2046->miso_id);
    }
}

void xpt2046_spi_init(xpt2046_t *lcdspi, bool spi_hw_mode) {
    if (spi_hw_mode) {
        m_spi_init();
    }
}

void xpt2046_init(xpt2046_t *xpt2046) {
    m_normalize = xpt2046->normalize;
    m_gpio_output = xpt2046->gpio_output;
    m_gpio_input = xpt2046->gpio_input;
    m_gpio_write = xpt2046->gpio_write;
    m_gpio_read = xpt2046->gpio_read;
    m_spi_init = xpt2046->spi_init;
    m_spi_transfer = xpt2046->spi_transfer;

    xpt2046_set_pin_mode(xpt2046);
    xpt2046_spi_init(xpt2046, true);
}

void xpt2046_deinit(xpt2046_t *xpt2046) {

}

void xpt2046_activate(xpt2046_t *xpt2046) {

}

static uint8_t tp_spi_xchg(xpt2046_t *xpt2046, uint8_t data_send) {
    uint8_t data_rec = 0;
    m_spi_transfer((size_t)1, (const uint8_t *)&data_send, (uint8_t *)&data_rec);
    return data_rec;
}

static void xpt2046_corr(xpt2046_t *xpt2046, int16_t *x, int16_t *y) {
    if ((*x) > xpt2046->x_min) {
        (*x) -= xpt2046->x_min;
    } else {
        (*x) = 0;
    }
    if ((*y) > xpt2046->y_min) {
        (*y) -= xpt2046->y_min;
    } else {
        (*y) = 0;
    }
    (*x) = (uint32_t)((uint32_t)(*x) * xpt2046->x_res) /
        (xpt2046->x_max - xpt2046->x_min);
    (*y) = (uint32_t)((uint32_t)(*y) * xpt2046->y_res) /
        (xpt2046->y_max - xpt2046->y_min);
    if (xpt2046->x_inv) {
        (*x) = xpt2046->x_res - (*x);
    }
    if (xpt2046->y_inv) {
        (*y) = xpt2046->y_res - (*y);
    }
}

static void xpt2046_avg(xpt2046_t *self, int16_t *x, int16_t *y) {
    /*Shift out the oldest data*/
    uint8_t i;
    for (i = XPT2046_AVG - 1; i > 0; i--) {
        self->avg_buf_x[i] = self->avg_buf_x[i - 1];
        self->avg_buf_y[i] = self->avg_buf_y[i - 1];
    }
    /*Insert the new point*/
    self->avg_buf_x[0] = *x;
    self->avg_buf_y[0] = *y;
    if (self->avg_last < XPT2046_AVG) {
        self->avg_last++;
    }
    /*Sum the x and y coordinates*/
    int32_t x_sum = 0;
    int32_t y_sum = 0;
    for (i = 0; i < self->avg_last; i++) {
        x_sum += self->avg_buf_x[i];
        y_sum += self->avg_buf_y[i];
    }
    /*Normalize the sums*/
    (*x) = (int32_t)x_sum / self->avg_last;
    (*y) = (int32_t)y_sum / self->avg_last;
}

void xpt2046_read(xpt2046_t *xpt2046, xpt2046_data_t *data) {
    static int16_t last_x = 0;
    static int16_t last_y = 0;
    bool valid = true;
    uint8_t buf;

    int16_t x = 0;
    int16_t y = 0;
    uint8_t irq = (uint8_t)m_gpio_read(xpt2046->irq_id);

    if (irq == 0) {
        uint32_t state = disable_irq();
        m_gpio_write(xpt2046->cs_id, 0);
        tp_spi_xchg(xpt2046, CMD_X_READ);       /*Start x read*/
        buf = tp_spi_xchg(xpt2046, 0);          /*Read x MSB*/
        x = buf << 8;
        buf = tp_spi_xchg(xpt2046, CMD_Y_READ); /*Until x LSB converted y command can be sent*/
        x += buf;
        buf = tp_spi_xchg(xpt2046, 0);  /*Read y MSB*/
        y = buf << 8;
        buf = tp_spi_xchg(xpt2046, 0);  /*Read y LSB*/
        y += buf;
        m_gpio_write(xpt2046->cs_id, 1);
        enable_irq(state);
        x = x >> 3; // 0 - 32767 -> 0 - 4095
        y = y >> 3; // 0 - 32767 -> 0 - 4095
        if (xpt2046->xy_swap) {
            int16_t swap_tmp;
            swap_tmp = x;
            x = y;
            y = swap_tmp;
        }
        if (m_normalize) {
            xpt2046_corr(xpt2046, &x, &y);
            xpt2046_avg(xpt2046, &x, &y);
        }
        last_x = x;
        last_y = y;
    } else {
        x = last_x;
        y = last_y;
        xpt2046->avg_last = 0;
        valid = false;
    }
    data->x = x;
    data->y = y;
    data->valid = valid;
}
