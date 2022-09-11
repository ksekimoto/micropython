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

#ifndef RX_RX_I2C_H_
#define RX_RX_I2C_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "iodefine.h"

typedef volatile struct st_riic riic_t;

typedef struct {
    uint32_t m_bytes_transferred;
    uint32_t m_bytes_transfer;
    bool m_fread;
    uint8_t *buf;
} xaction_unit_t;

typedef struct {
    xaction_unit_t *units;
    uint32_t m_num_of_units;
    uint32_t m_current;
    uint32_t m_clock;
    uint32_t m_address;
    uint32_t m_status;
} xaction_t;

#ifdef __cplusplus
}
#endif

#endif /* RX_RX_I2C_H_ */
