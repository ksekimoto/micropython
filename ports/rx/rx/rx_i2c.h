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

#include <stdint.h>
#include <stdbool.h>
#include "iodefine.h"

typedef volatile struct st_riic riic_t;

#define RX_I2C_DEF_TIMEOUT              1000    // 1000 ms
#define RX_I2C_TIMEOUT_STOP_CONDITION   100000  // counts
#define RX_I2C_TIMEOUT_BUS_BUSY         100000  // counts
#define RX_I2C_CLOCK_MAX                1000000 // counts

typedef enum
{
    RX_I2C_STATUS_Idle = 1,
    RX_I2C_STATUS_Started = 2,
    RX_I2C_STATUS_AddrWriteCompleted = 3,
    RX_I2C_STATUS_DataWriteCompleted = 4,
    RX_I2C_STATUS_DataSendCompleted = 5,
    RX_I2C_STATUS_FirstReceiveCompleted = 5,
    RX_I2C_STATUS_LastReceiveCompleted = 6,
    RX_I2C_STATUS_Stopped = 7,
} xaction_status_t;

typedef enum
{
    RX_I2C_ERROR_OK = 0,
    RX_I2C_ERROR_TMOF = 1,
    RX_I2C_ERROR_AL = 2,
    RX_I2C_ERROR_NACK = 3,
    RX_I2C_ERROR_BUSY = 4,
} xaction_error_t;

typedef struct {
    volatile uint32_t m_bytes_transferred;
    volatile uint32_t m_bytes_transfer;
    bool m_fread;
    uint8_t *buf;
    void *next;
} xaction_unit_t;

typedef struct {
    xaction_unit_t *units;
    uint32_t m_num_of_units;
    uint32_t m_current;
    uint32_t m_ch;
    uint32_t m_baudrate;
    uint32_t m_address;
    volatile xaction_status_t m_status;
    xaction_error_t m_error;
    bool m_stop;
} xaction_t;

void rx_i2c_set_baudrate(uint32_t ch, uint32_t baudrate);
void rx_i2c_init(uint32_t ch, uint32_t scl, uint32_t sda, uint32_t baudrate, uint32_t timeout);
void rx_i2c_deinit(uint32_t ch);
void rx_i2c_read_last_byte(uint32_t ch);
void rx_i2c_stop_condition(uint32_t ch);
void rx_i2c_abort(uint32_t ch);
void rx_i2c_xaction_start(xaction_t *xaction, bool repeated_start);
void rx_i2c_xaction_stop(void);
void rx_i2c_unit_write_byte(uint32_t ch, xaction_unit_t *unit);
void rx_i2c_unit_read_byte(uint32_t ch, xaction_unit_t *unit);
void rx_i2c_unit_init(xaction_unit_t *unit, uint8_t *buf, uint32_t size, bool fread, void *next);
void rx_i2c_xaction_init(xaction_t *action, xaction_unit_t *units, uint32_t size, uint32_t ch, uint32_t baudrate, uint32_t address, bool stop);
bool rx_i2c_action_execute(xaction_t *action, bool repeated_start, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* RX_RX_I2C_H_ */
