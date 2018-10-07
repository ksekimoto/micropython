/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 * Copyright (c) 2018 Kentaro Sekimoto
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

/*------------------------------------------------------------------------*/
/* STM32F100: MMCv3/SDv1/SDv2 (SPI mode) control module                   */
/*------------------------------------------------------------------------*/
/*
/  Copyright (C) 2018, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-------------------------------------------------------------------------*/

#include <string.h>
#include <stdbool.h>

#include "py/runtime.h"
#include "py/mphal.h"
#include "lib/oofatfs/ff.h"
#include "extmod/vfs_fat.h"

#include "sdcard.h"
#include "pin.h"
#include "pin_static_af.h"
#include "bufhelper.h"
#include "common.h"

#if MICROPY_HW_HAS_SDCARD

/* MMC card type flags (MMC_GET_TYPE) */
#define CT_MMC      0x01        /* MMC ver 3 */
#define CT_SD1      0x02        /* SD ver 1 */
#define CT_SD2      0x04        /* SD ver 2 */
#define CT_SDC      (CT_SD1|CT_SD2) /* SD */
#define CT_BLOCK    0x08        /* Block addressing */

/* MMC/SD command */
#define CMD0    (0)         /* GO_IDLE_STATE */
#define CMD1    (1)         /* SEND_OP_COND (MMC) */
#define	ACMD41  (0x80+41)   /* SEND_OP_COND (SDC) */
#define CMD8    (8)         /* SEND_IF_COND */
#define CMD9    (9)         /* SEND_CSD */
#define CMD10   (10)        /* SEND_CID */
#define CMD12   (12)        /* STOP_TRANSMISSION */
#define ACMD13  (0x80+13)   /* SD_STATUS (SDC) */
#define CMD16   (16)        /* SET_BLOCKLEN */
#define CMD17   (17)        /* READ_SINGLE_BLOCK */
#define CMD18   (18)        /* READ_MULTIPLE_BLOCK */
#define CMD23   (23)        /* SET_BLOCK_COUNT (MMC) */
#define	ACMD23  (0x80+23)   /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24   (24)        /* WRITE_BLOCK */
#define CMD25   (25)        /* WRITE_MULTIPLE_BLOCK */
#define CMD32   (32)        /* ERASE_ER_BLK_START */
#define CMD33   (33)        /* ERASE_ER_BLK_END */
#define CMD38   (38)        /* ERASE */
#define CMD55   (55)        /* APP_CMD */
#define CMD58   (58)        /* READ_OCR */

#if defined(GRSAKURA)
#define MICROPY_HW_SDCARD_SPI_CH    0       /* channel 0 */
#define MICROPY_HW_SDCARD_SPI_CS    96      /* pin PC0 */
#define MICROPY_HW_SDCARD_CHK       13      /* pin P15 */
#endif
#if defined(GRCITRUS)
#define MICROPY_HW_SDCARD_SPI_CH    0       /* channel 0 */
#define MICROPY_HW_SDCARD_SPI_CS    98      /* pin PC2 */
#define MICROPY_HW_SDCARD_CHK       13      /* pin P15 */
#endif

#define _CMD_TIMEOUT        100
#define _TOKEN_CMD25        0xfc
#define _TOKEN_STOP_TRAN    0xfd
#define _TOKEN_DATA         0xfe
#define SD_OK               0
#define SD_ERR_NO_SDCARD    2
#define SD_ERR_EIO          5
#define SD_ERR              -1

static uint32_t sd_handle = NULL;
static uint8_t sd_ch = MICROPY_HW_SDCARD_SPI_CH;
static uint8_t sd_cs = MICROPY_HW_SDCARD_SPI_CS;
static uint8_t sd_csd[16];
static uint32_t sd_cdv;
static uint32_t sd_sectors;
static uint32_t sd_blocksize = 512;
static uint8_t sd_card_type;

#define SD_TIMEOUT  1000000
#define CS_HIGH()	gpio_write(sd_cs, 1)
#define CS_LOW()	gpio_write(sd_cs, 0)

inline static uint8_t xchg_spi(uint8_t dat) {
    return rx_spi_write_byte(sd_ch, dat);
}

/* 1:Ready, 0:Timeout */
/* Timeout [ms] */
static int wait_ready(UINT wt) {
    uint8_t d;
    uint32_t timeout = wt * 1000;
    do {
        d = xchg_spi(0xFF);
    } while (d != 0xFF && timeout-- > 0);
    return (d == 0xFF) ? 1 : 0;
}

/*-----------------------------------------------------------------------*/
/* Deselect card and release SPI                                         */
/*-----------------------------------------------------------------------*/

static void deselect(void) {
    CS_HIGH();      /* Set CS# high */
    xchg_spi(0xFF); /* Dummy clock (force DO hi-z for multiple slave SPI) */
}

/*-----------------------------------------------------------------------*/
/* Select card and wait for ready                                        */
/*-----------------------------------------------------------------------*/

/* 1:OK, 0:Timeout */
static int select(void) {
    CS_LOW();       /* Set CS# low */
    xchg_spi(0xFF); /* Dummy clock (force DO enabled) */
    if (wait_ready(500)) {
        return 1;   /* Wait for card ready */
    }
    deselect();
    return 0;       /* Timeout */
}

/*-----------------------------------------------------------------------*/
/* Send a command packet to the MMC                                      */
/*-----------------------------------------------------------------------*/
/* Return value: R1 resp (bit7==1:Failed to send) */
/* Command index */
/* Argument */
static BYTE sd_cmd(uint8_t cmd, uint32_t arg) {
    uint8_t n, res;

    if (cmd & 0x80) {   /* Send a CMD55 prior to ACMD<n> */
        cmd &= 0x7F;
        res = sd_cmd(CMD55, 0);
        if (res > 1) {
            return res;
        }
    }
    /* Select the card and wait for ready except to stop multiple block read */
    if (cmd != CMD12) {
        deselect();
        if (!select()) {
            return 0xFF;
        }
    }
    /* Send command packet */
    xchg_spi(0x40 | cmd);               /* Start + command index */
    xchg_spi((BYTE)(arg >> 24));        /* Argument[31..24] */
    xchg_spi((BYTE)(arg >> 16));        /* Argument[23..16] */
    xchg_spi((BYTE)(arg >> 8));         /* Argument[15..8] */
    xchg_spi((BYTE)arg);                /* Argument[7..0] */
    n = 0x01;                           /* Dummy CRC + Stop */
    if (cmd == CMD0) n = 0x95;          /* Valid CRC for CMD0(0) */
    if (cmd == CMD8) n = 0x87;          /* Valid CRC for CMD8(0x1AA) */
    xchg_spi(n);
    /* Receive command resp */
    /* Discard following one byte when CMD12 */
    if (cmd == CMD12) {
        xchg_spi(0xFF);
    }
    n = 10;
    /* Wait for response (10 bytes max) */
    do {
        res = xchg_spi(0xFF);
    } while ((res & 0x80) && --n);
    return res;                         /* Return received response */
}

static void sd_readinto(uint8_t *buf, uint32_t len) {
    gpio_write(sd_cs ,0);
    while (true) {
        if (rx_spi_write_byte(sd_ch, 0xff) == _TOKEN_DATA)
            break;
    }
    rx_spi_transfer8(sd_ch, buf, buf, len);
    rx_spi_write_byte(sd_ch, 0xff);   /* read checksum */
    rx_spi_write_byte(sd_ch, 0xff);   /* read checksum */
    gpio_write(sd_cs ,1);
    rx_spi_write_byte(sd_ch, 0xff);
}

static void sd_write_buf(uint8_t token, uint8_t *buf, uint32_t len) {
    uint8_t res;
    gpio_write(sd_cs ,0);
    res = rx_spi_write_byte(sd_ch, token);
    rx_spi_write_bytes(sd_ch, buf, len);
    rx_spi_write_byte(sd_ch, 0xff);
    rx_spi_write_byte(sd_ch, 0xff);
    if ((rx_spi_write_byte(sd_ch, 0xff) & 0x1f) != 0x05) {
        gpio_write(sd_cs ,1);
        rx_spi_write_byte(sd_ch, 0xff);
        return;
    }
    while (rx_spi_write_byte(sd_ch, 0xff) == 0) {
        ;
    }
    gpio_write(sd_cs ,1);
    rx_spi_write_byte(sd_ch, 0xff);
}

static void sd_write_token(uint8_t token) {
    uint8_t res;
    gpio_write(sd_cs ,0);
    res = rx_spi_write_byte(sd_ch, token);
    rx_spi_write_byte(sd_ch, 0xff);
    while (rx_spi_write_byte(sd_ch, 0xff) == 0) {
        ;
    }
    gpio_write(sd_cs ,1);
    rx_spi_write_byte(sd_ch, 0xff);
}

#if 0
static void sd_init_card_v1(void) {
    int count = _CMD_TIMEOUT;
    while (count != 0) {
        sd_cmd(CMD55, 0);
        if (sd_cmd(ACMD41, 0) == 0) {
            sd_cdv = 512;
            /* "[SDCard] v1 card" */
            return;
        }
        count--;
    }
    return; /* raise OSError("timeout waiting for v1 card") */
}

static void sd_init_card_v2(void) {
    int count = _CMD_TIMEOUT;
    while (count != 0) {
        udelay(50000);
        sd_cmd(CMD58, 0);
        sd_cmd(CMD55, 0);
        if (sd_cmd(ACMD41, 0x40000000) == 0) {
            sd_cmd(CMD58, 0);
            sd_cdv = 1;
            /* "[SDCard] v2 card" */
            return;
        }
        count--;
    }
    return; /* raise OSError("timeout waiting for v2 card") */
}
#endif

static void sd_get_sector_count(void) {
    uint32_t csize;
    uint32_t n;

    if (sd_cmd(CMD9, 0) == 0) {
        sd_readinto(sd_csd, sizeof sd_csd);
        if ((sd_csd[0] >> 6) == 1) {
            /* SDC ver 2.00 */
            csize = sd_csd[9] + ((uint32_t)sd_csd[8] << 8) + ((uint32_t)(sd_csd[7] & 63) << 16) + 1;
            csize = (csize << 10);
        } else {
            /* SDC ver 1.XX or MMC ver 3 */
            n = (sd_csd[5] & 15) + ((sd_csd[10] & 128) >> 7) + ((sd_csd[9] & 3) << 1) + 2;
            csize = (sd_csd[8] >> 6) + ((uint32_t)sd_csd[7] << 2) + ((uint32_t)(sd_csd[6] & 3) << 10) + 1;
            csize = (csize << (n - 9));
        }
    }
    sd_sectors = csize;
}

static void sd_get_sector_block_size(void) {
    uint32_t n;
    if (sd_card_type & CT_SD2) {
        /* SDC ver 2.00 */
        if (sd_cmd(ACMD13, 0) == 0) { /* Read SD status */
            xchg_spi(0xFF);
            sd_readinto(sd_csd, sizeof sd_csd); /* Read partial block */
            for (n = 64 - 16; n; n--) {
                xchg_spi(0xFF);   /* Purge trailing data */
            }
            sd_blocksize = 16UL << (sd_csd[10] >> 4);
        }
    } else {
        /* SDC ver 1.XX or MMC */
        if (sd_cmd(CMD9, 0) == 0) {  /* Read CSD */
            sd_readinto(sd_csd, sizeof sd_csd);
            if (sd_card_type & CT_SD1) {
                /* SDC ver 1.XX */
                sd_blocksize = (((sd_csd[10] & 63) << 1) + ((uint32_t)(sd_csd[11] & 128) >> 7) + 1) << ((sd_csd[13] >> 6) - 1);
            } else {                    /* MMC */
                sd_blocksize = ((uint32_t)((sd_csd[10] & 124) >> 2) + 1) * (((sd_csd[11] & 3) << 3) + ((sd_csd[11] & 224) >> 5) + 1);
            }
        }
    }
}

uint8_t sd_init_card(void) {
    uint8_t n, cmd, ty, ocr[4];
    uint32_t timeout;
    uint32_t c_size;
    uint32_t c_size_mult;

    gpio_write(sd_cs, 1);
    rx_spi_init(sd_ch, sd_cs, 100000, 8, 0);
    /* Send 80 dummy clocks */
    for (int i = 0; i < 10; i++) {
        xchg_spi(0xFF);
    }
    ty = 0;
    if (sd_cmd(CMD0, 0) == 1) {         /* Put the card SPI/Idle state */
        timeout = 1000000;              /* Initialization timeout = 1 sec */
        if (sd_cmd(CMD8, 0x1AA) == 1) { /* SDv2? */
            sd_cdv = 512;
            for (n = 0; n < 4; n++) {
                ocr[n] = xchg_spi(0xFF); /* Get 32 bit return value of R7 resp */
            }
            if (ocr[2] == 0x01 && ocr[3] == 0xAA) {     /* Is the card supports vcc of 2.7-3.6V? */
                /* Wait for end of initialization with ACMD41(HCS) */
                while ((timeout-- > 0) && sd_cmd(ACMD41, 1UL << 30)) {
                    ;
                }
                if ((timeout != 0) && sd_cmd(CMD58, 0) == 0) {  /* Check CCS bit in the OCR */
                    for (n = 0; n < 4; n++) {
                        ocr[n] = xchg_spi(0xFF);
                    }
                    ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2; /* Card id SDv2 */
                }
            }
        } else {
            /* Not SDv2 card */
            sd_cdv = 512;
            if (sd_cmd(ACMD41, 0) <= 1) {   /* SDv1 or MMC? */
                ty = CT_SD1;
                cmd = ACMD41;               /* SDv1 (ACMD41(0)) */
            } else {
                ty = CT_MMC;
                cmd = CMD1;                 /* MMCv3 (CMD1(0)) */
            }
            while ((timeout-- > 0) && sd_cmd(cmd, 0))
                ;                           /* Wait for end of initialization */
            if ((timeout != 0) || sd_cmd(CMD16, 512) != 0) {
                /* Set block length: 512 */
                ty = 0;
            }
        }
    }
    sd_card_type = ty;      /* Card type */
    sd_get_sector_count();
    sd_get_sector_block_size();
    deselect();

    rx_spi_init(sd_ch, sd_cs, 4000000, 8, 0);
    return 0;
}

void sdcard_init(void) {
    sd_handle = NULL;
    sd_ch = MICROPY_HW_SDCARD_SPI_CH;
    sd_cs = MICROPY_HW_SDCARD_SPI_CS;
    gpio_mode_input(MICROPY_HW_SDCARD_CHK);
    gpio_mode_output(sd_cs);
    sd_init_card();
    sd_handle = 1;
}

bool sdcard_is_present(void) {
    gpio_mode_input(MICROPY_HW_SDCARD_CHK);
    return (gpio_read(MICROPY_HW_SDCARD_CHK) == MICROPY_HW_SDCARD_DETECT_PRESENT);
}

bool sdcard_power_on(void) {
    if (!sdcard_is_present()) {
        return false;
    }
    if (sd_handle) {
        return true;
    }
    sd_handle = 1;
    return true;
}

void sdcard_power_off(void) {
    if (!sd_handle) {
        return;
    }
    sd_handle = NULL;
}

uint64_t sdcard_get_capacity_in_bytes(void) {
    if (sd_handle == NULL) {
        return (uint64_t)0;
    }
    return (uint64_t)sd_sectors * (uint64_t)sd_blocksize;
}

static uint32_t sd_readblocks(uint32_t block_num, uint8_t *buf, uint32_t len) {
    uint32_t offset;
    uint32_t nblocks = len;
    if (nblocks == 1) {
        if (sd_cmd(CMD17, block_num * sd_cdv) != 0) {
            gpio_write(sd_cs, 1);
            /* raise OSError(5) */
            return SD_ERR_EIO;
        }
        sd_readinto(buf, 512);
    } else {
        if (sd_cmd(CMD18, block_num * sd_cdv) != 0) {
            gpio_write(sd_cs, 1);
            /* raise OSError(5) */
            return SD_ERR_EIO;
        }
        offset = 0;
        while (nblocks) {
            sd_readinto(&buf[offset], 512);
            offset += 512;
            nblocks--;
        }
        if (sd_cmd(CMD12, 0)) {
            /* raise OSError(5) */
            return SD_ERR_EIO;
        }
    }
    return SD_OK;
}

mp_uint_t sdcard_read_blocks(uint8_t *dest, uint32_t block_num, uint32_t num_blocks) {
    // check that SD card is initialised
    if (sd_handle == NULL) {
        return SD_ERR;
    }
    uint32_t err = SD_OK;
    err = sd_readblocks(block_num, (uint8_t *)dest, num_blocks);
    return err;
}

static uint32_t sd_writeblocks(uint32_t block_num, uint8_t *buf, uint32_t len) {
    uint32_t offset;
    uint32_t nblocks = len;

    if (nblocks == 1) {
        if (sd_cmd(CMD24, block_num * sd_cdv) != 0) {
            /* raise OSError(5) */
            return SD_ERR_EIO;
        }
        sd_write_buf(_TOKEN_DATA, buf, 512);
    } else {
        if (sd_cmd(CMD25, block_num * sd_cdv) != 0) {
            /* raise OSError(5) */
            return SD_ERR_EIO;
        }
        offset = 0;
        while (nblocks) {
            sd_write_buf(_TOKEN_CMD25, &buf[offset], 512);
            offset += 512;
            nblocks--;
        }
        sd_write_token(_TOKEN_STOP_TRAN);
    }
    return SD_OK;
}

mp_uint_t sdcard_write_blocks(const uint8_t *src, uint32_t block_num, uint32_t num_blocks) {
    // check that SD card is initialised
    if (sd_handle == NULL) {
        return SD_ERR;
    }

    uint32_t err = SD_OK;
    err = sd_writeblocks(block_num, (uint8_t *)src, num_blocks);
    return err;
}

/******************************************************************************/
// MicroPython bindings
//
// Expose the SD card as an object with the block protocol.

// there is a singleton SDCard object
const mp_obj_base_t pyb_sdcard_obj = {&pyb_sdcard_type};

STATIC mp_obj_t pyb_sdcard_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    // return singleton object
    return MP_OBJ_FROM_PTR(&pyb_sdcard_obj);
}

STATIC mp_obj_t sd_present(mp_obj_t self) {
    return mp_obj_new_bool(sdcard_is_present());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(sd_present_obj, sd_present);

STATIC mp_obj_t sd_power(mp_obj_t self, mp_obj_t state) {
    bool result;
    if (mp_obj_is_true(state)) {
        result = sdcard_power_on();
    } else {
        sdcard_power_off();
        result = true;
    }
    return mp_obj_new_bool(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(sd_power_obj, sd_power);

STATIC mp_obj_t sd_info(mp_obj_t self) {
    if (sd_handle == NULL) {
        return mp_const_none;
    }
    mp_obj_t tuple[3] = {
        mp_obj_new_int_from_ull((uint64_t)sd_sectors * (uint64_t)sd_blocksize),
        mp_obj_new_int_from_uint(sd_blocksize),
        mp_obj_new_int(0L),
    };
    return mp_obj_new_tuple(3, tuple);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(sd_info_obj, sd_info);

// now obsolete, kept for backwards compatibility
STATIC mp_obj_t sd_read(mp_obj_t self, mp_obj_t block_num) {
    uint8_t *dest = m_new(uint8_t, SDCARD_BLOCK_SIZE);
    mp_uint_t ret = sdcard_read_blocks(dest, mp_obj_get_int(block_num), 1);

    if (ret != 0) {
        m_del(uint8_t, dest, SDCARD_BLOCK_SIZE);
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_Exception, "sdcard_read_blocks failed [%u]", ret));
    }

    return mp_obj_new_bytearray_by_ref(SDCARD_BLOCK_SIZE, dest);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(sd_read_obj, sd_read);

// now obsolete, kept for backwards compatibility
STATIC mp_obj_t sd_write(mp_obj_t self, mp_obj_t block_num, mp_obj_t data) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(data, &bufinfo, MP_BUFFER_READ);
    if (bufinfo.len % SDCARD_BLOCK_SIZE != 0) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "writes must be a multiple of %d bytes", SDCARD_BLOCK_SIZE));
    }

    mp_uint_t ret = sdcard_write_blocks(bufinfo.buf, mp_obj_get_int(block_num), bufinfo.len / SDCARD_BLOCK_SIZE);

    if (ret != 0) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_Exception, "sdcard_write_blocks failed [%u]", ret));
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(sd_write_obj, sd_write);

STATIC mp_obj_t pyb_sdcard_readblocks(mp_obj_t self, mp_obj_t block_num, mp_obj_t buf) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf, &bufinfo, MP_BUFFER_WRITE);
    mp_uint_t ret = sdcard_read_blocks(bufinfo.buf, mp_obj_get_int(block_num), bufinfo.len / SDCARD_BLOCK_SIZE);
    return mp_obj_new_bool(ret == 0);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(pyb_sdcard_readblocks_obj, pyb_sdcard_readblocks);

STATIC mp_obj_t pyb_sdcard_writeblocks(mp_obj_t self, mp_obj_t block_num, mp_obj_t buf) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf, &bufinfo, MP_BUFFER_READ);
    mp_uint_t ret = sdcard_write_blocks(bufinfo.buf, mp_obj_get_int(block_num), bufinfo.len / SDCARD_BLOCK_SIZE);
    return mp_obj_new_bool(ret == 0);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(pyb_sdcard_writeblocks_obj, pyb_sdcard_writeblocks);

STATIC mp_obj_t pyb_sdcard_ioctl(mp_obj_t self, mp_obj_t cmd_in, mp_obj_t arg_in) {
    mp_int_t cmd = mp_obj_get_int(cmd_in);
    switch (cmd) {
        case BP_IOCTL_INIT:
            //if (!sdcard_power_on()) {
            //    return MP_OBJ_NEW_SMALL_INT(-1); // error
            //}
            return MP_OBJ_NEW_SMALL_INT(0); // success

        case BP_IOCTL_DEINIT:
            sdcard_power_off();
            return MP_OBJ_NEW_SMALL_INT(0); // success

        case BP_IOCTL_SYNC:
            // nothing to do
            return MP_OBJ_NEW_SMALL_INT(0); // success

        case BP_IOCTL_SEC_COUNT:
            return MP_OBJ_NEW_SMALL_INT(sdcard_get_capacity_in_bytes() / SDCARD_BLOCK_SIZE);

        case BP_IOCTL_SEC_SIZE:
            return MP_OBJ_NEW_SMALL_INT(SDCARD_BLOCK_SIZE);

        default: // unknown command
            return MP_OBJ_NEW_SMALL_INT(-1); // error
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(pyb_sdcard_ioctl_obj, pyb_sdcard_ioctl);

STATIC const mp_rom_map_elem_t pyb_sdcard_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_present), MP_ROM_PTR(&sd_present_obj) },
    { MP_ROM_QSTR(MP_QSTR_power), MP_ROM_PTR(&sd_power_obj) },
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&sd_info_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&sd_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&sd_write_obj) },
    // block device protocol
    { MP_ROM_QSTR(MP_QSTR_readblocks), MP_ROM_PTR(&pyb_sdcard_readblocks_obj) },
    { MP_ROM_QSTR(MP_QSTR_writeblocks), MP_ROM_PTR(&pyb_sdcard_writeblocks_obj) },
    { MP_ROM_QSTR(MP_QSTR_ioctl), MP_ROM_PTR(&pyb_sdcard_ioctl_obj) },
};

STATIC MP_DEFINE_CONST_DICT(pyb_sdcard_locals_dict, pyb_sdcard_locals_dict_table);

const mp_obj_type_t pyb_sdcard_type = {
    { &mp_type_type },
    .name = MP_QSTR_SDCard,
    .make_new = pyb_sdcard_make_new,
    .locals_dict = (mp_obj_dict_t *)&pyb_sdcard_locals_dict,
};

void sdcard_init_vfs(fs_user_mount_t *vfs, int part) {
    vfs->base.type = &mp_fat_vfs_type;
    vfs->flags |= FSUSER_NATIVE | FSUSER_HAVE_IOCTL;
    vfs->fatfs.drv = vfs;
    vfs->fatfs.part = part;
    vfs->readblocks[0] = MP_OBJ_FROM_PTR(&pyb_sdcard_readblocks_obj);
    vfs->readblocks[1] = MP_OBJ_FROM_PTR(&pyb_sdcard_obj);
    vfs->readblocks[2] = MP_OBJ_FROM_PTR(sdcard_read_blocks); // native version
    vfs->writeblocks[0] = MP_OBJ_FROM_PTR(&pyb_sdcard_writeblocks_obj);
    vfs->writeblocks[1] = MP_OBJ_FROM_PTR(&pyb_sdcard_obj);
    vfs->writeblocks[2] = MP_OBJ_FROM_PTR(sdcard_write_blocks); // native version
    vfs->u.ioctl[0] = MP_OBJ_FROM_PTR(&pyb_sdcard_ioctl_obj);
    vfs->u.ioctl[1] = MP_OBJ_FROM_PTR(&pyb_sdcard_obj);
}

#endif // MICROPY_HW_HAS_SDCARD
