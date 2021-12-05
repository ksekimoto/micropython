/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
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

#include <stdio.h>
#include <string.h>

#include "py/runtime.h"
#include "py/mphal.h"
#include "extmod/machine_spi.h"
#include "irq.h"
#include "pin.h"
#include "bufhelper.h"
#include "spi.h"
#include "mbed_spi.h"

// #define MBED_SPI_WRAPPER

#if defined(MBED_SPI_WRAPPER)
#include "mbed_spi.h"
#define MBED_SPI_SET_BITS       mbed_spi_set_bits
#define MBED_SPI_SET_CLK        mbed_spi_set_clk
#define MBED_SPI_SET_FIRSTBIT   mbed_spi_set_firstbit
#define MBED_SPI_SET_SPI_CH     mbed_spi_set_spi_ch
#define MBED_SPI_TRANSFER       mbed_spi_transfer
#define MBED_SPI_INIT           mbed_spi_init
#define MBED_SPI_DEINIT         mbed_spi_deinit
#else
#include "rz_spi.h"
#define MBED_SPI_SET_BITS       rz_spi_set_bits
#define MBED_SPI_SET_CLK        rz_spi_set_clk
#define MBED_SPI_SET_FIRSTBIT   rz_spi_set_firstbit
#define MBED_SPI_SET_SPI_CH     rz_spi_set_spi_ch
#define MBED_SPI_TRANSFER       rz_spi_transfer
#define MBED_SPI_INIT           rz_spi_init
#define MBED_SPI_DEINIT         rz_spi_deinit
#endif

/// \moduleref pyb
/// \class SPI - a master-driven serial protocol
///
/// SPI is a serial protocol that is driven by a master.  At the physical level
/// there are 3 lines: SCK, MOSI, MISO.
///
/// See usage model of I2C; SPI is very similar.  Main difference is
/// parameters to init the SPI bus:
///
///     from pyb import SPI
///     spi = SPI(1, SPI.MASTER, baudrate=600000, polarity=1, phase=0, crc=0x7)
///
/// Only required parameter is mode, SPI.MASTER or SPI.SLAVE.  Polarity can be
/// 0 or 1, and is the level the idle clock line sits at.  Phase can be 0 or 1
/// to sample data on the first or second clock edge respectively.  Crc can be
/// None for no CRC, or a polynomial specifier.
///
/// Additional method for SPI:
///
///     data = spi.send_recv(b'1234')        # send 4 bytes and receive 4 bytes
///     buf = bytearray(4)
///     spi.send_recv(b'1234', buf)          # send 4 bytes and receive 4 into buf
///     spi.send_recv(buf, buf)              # send/recv 4 bytes from/to buf

/* So far, only ch is used. */
mspi_t spi_obj[] = {
    {0, 8},
    {1, 8},
    {2, 8},
};

void _spi_init0(void) {
}

int spi_find_index(mp_obj_t id) {
    int spi_id;
    if (mp_obj_is_str(id)) {
        // given a string id
        const char *port = mp_obj_str_get_str(id);
        if (0) {
        #ifdef MICROPY_HW_SPI1_NAME
        } else if (strcmp(port, MICROPY_HW_SPI1_NAME) == 0) {
            spi_id = 1;
        #endif
        #ifdef MICROPY_HW_SPI2_NAME
        } else if (strcmp(port, MICROPY_HW_SPI2_NAME) == 0) {
            spi_id = 2;
        #endif
        #ifdef MICROPY_HW_SPI3_NAME
        } else if (strcmp(port, MICROPY_HW_SPI3_NAME) == 0) {
            spi_id = 3;
        #endif
        } else {
            mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("SPI(%s) doesn't exist"), port);
        }
    } else {
        // given an integer id
        spi_id = mp_obj_get_int(id);
        if (spi_id < 1 || spi_id > MP_ARRAY_SIZE(spi_obj)) {
            mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("SPI(%d) doesn't exist"), spi_id);
        }
    }

    // check if the SPI is reserved for system use or not
    if (MICROPY_HW_SPI_IS_RESERVED(spi_id)) {
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("SPI(%d) is reserved"), spi_id);
    }

    return spi_id;
}

// sets the parameters in the SPI_InitTypeDef struct
// if an argument is -1 then the corresponding parameter is not changed
void spi_set_params(const mspi_t *spi_obj, uint32_t prescale, int32_t baudrate,
    int32_t polarity, int32_t phase, int32_t bits, int32_t firstbit) {
    MBED_SPI_SET_SPI_CH(spi_obj->ch, polarity, phase);
    MBED_SPI_SET_CLK(spi_obj->ch, baudrate);
    MBED_SPI_SET_BITS(spi_obj->ch, bits);
    MBED_SPI_SET_FIRSTBIT(spi_obj->ch, firstbit);
}

// TODO allow to take a list of pins to use
void _spi_init(const mspi_t *self, bool enable_nss_pin) {
    // ToDo: implement using pins definition
    // const pin_obj_t *pins[4] = { NULL, NULL, NULL, NULL };

    if (0) {
    #if defined(MICROPY_HW_SPI1_SCK)
    } else if (self->ch == 0) {
        // #if defined(MICROPY_HW_SPI1_NSS)
        // pins[0] = MICROPY_HW_SPI1_NSS;
        // #endif
        // pins[1] = MICROPY_HW_SPI1_SCK;
        // #if defined(MICROPY_HW_SPI1_MISO)
        // pins[2] = MICROPY_HW_SPI1_MISO;
        // #endif
        // pins[3] = MICROPY_HW_SPI1_MOSI;
        MBED_SPI_INIT(self->ch, 0, 4000000, 8, 0);
    #endif
    #if defined(MICROPY_HW_SPI2_SCK)
    } else if (self->ch == 1) {
        // #if defined(MICROPY_HW_SPI2_NSS)
        // pins[0] = MICROPY_HW_SPI2_NSS;
        // #endif
        // pins[1] = MICROPY_HW_SPI2_SCK;
        // #if defined(MICROPY_HW_SPI2_MISO)
        // pins[2] = MICROPY_HW_SPI2_MISO;
        // #endif
        // pins[3] = MICROPY_HW_SPI2_MOSI;
        MBED_SPI_INIT(self->ch, 0, 4000000, 8, 0);
    #endif
    #if defined(MICROPY_HW_SPI3_SCK)
    } else if (self->ch == 2) {
        // #if defined(MICROPY_HW_SPI0_NSS)
        // pins[0] = MICROPY_HW_SPI3_NSS;
        // #endif
        // pins[1] = MICROPY_HW_SPI3_SCK;
        // #if defined(MICROPY_HW_SPI3_MISO)
        // pins[2] = MICROPY_HW_SPI3_MISO;
        // #endif
        // pins[3] = MICROPY_HW_SPI3_MOSI;
        MBED_SPI_INIT(self->ch, 0, 4000000, 8, 0);
    #endif
    } else {
        // SPI does not exist for this board (shouldn't get here, should be checked by caller)
        return;
    }

}

void spi_deinit(const mspi_t *spi_obj) {
    if (0) {
    #if defined(MICROPY_HW_SPI1_SCK)
    } else if (spi_obj->ch == 0) {
    #endif
    #if defined(MICROPY_HW_SPI2_SCK)
    } else if (spi_obj->ch == 1) {
    #endif
    #if defined(MICROPY_HW_SPI3_SCK)
    } else if (spi_obj->ch == 2) {
    #endif
    }
    MBED_SPI_DEINIT(spi_obj->ch, 0);
}

// A transfer of "len" bytes should take len*8*1000/baudrate milliseconds.
// To simplify the calculation we assume the baudrate is never less than 8kHz
// and use that value for the baudrate in the formula, plus a small constant.
#define SPI_TRANSFER_TIMEOUT(len) ((len) + 100)

void spi_transfer(const mspi_t *self, size_t len, const uint8_t *src, uint8_t *dest, uint32_t timeout) {
    MBED_SPI_TRANSFER(self->ch, self->bits, dest, (uint8_t *)src, (uint32_t)len, timeout);
}

void spi_print(const mp_print_t *print, const mspi_t *spi_obj, bool legacy) {
    uint spi_num = 1; // default to SPI1
    mp_printf(print, "SPI(%u", spi_num);
    mp_print_str(print, ")");
}

const mspi_t *spi_from_mp_obj(mp_obj_t o) {
    if (mp_obj_is_type(o, &pyb_spi_type)) {
        pyb_spi_obj_t *self = MP_OBJ_TO_PTR(o);
        return self->spi;
    } else if (mp_obj_is_type(o, &machine_hard_spi_type)) {
        machine_hard_spi_obj_t *self = MP_OBJ_TO_PTR(o);
        return self->spi;
    } else {
        mp_raise_TypeError(MP_ERROR_TEXT("expecting an SPI object"));
    }
}

/******************************************************************************/
// Implementation of low-level SPI C protocol

STATIC int spi_proto_ioctl(void *self_in, uint32_t cmd) {
    spi_proto_cfg_t *self = (spi_proto_cfg_t *)self_in;

    switch (cmd) {
        case MP_SPI_IOCTL_INIT:
            _spi_init(self->spi, false);
            spi_set_params(self->spi, 0xffffffff, self->baudrate,
                self->polarity, self->phase, self->bits, self->firstbit);
            break;

        case MP_SPI_IOCTL_DEINIT:
            spi_deinit(self->spi);
            break;
    }

    return 0;
}

STATIC void spi_proto_transfer(void *self_in, size_t len, const uint8_t *src, uint8_t *dest) {
    spi_proto_cfg_t *self = (spi_proto_cfg_t *)self_in;
    spi_transfer(self->spi, len, src, dest, SPI_TRANSFER_TIMEOUT(len));
}

const mp_spi_proto_t spi_proto = {
    .ioctl = spi_proto_ioctl,
    .transfer = spi_proto_transfer,
};
