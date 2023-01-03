/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Damien P. George
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

#include <string.h>

#include "py/runtime.h"
#include "py/stream.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "extmod/misc.h"
#include "usb.h"
#include "uart.h"
#include "modmachine.h"

typedef enum
{
    HAL_OK       = 0x00,
    HAL_ERROR    = 0x01,
    HAL_BUSY     = 0x02,
    HAL_TIMEOUT  = 0x03
} HAL_StatusTypeDef;

// this table converts from HAL_StatusTypeDef to POSIX errno
const byte mp_hal_status_to_errno_table[4] = {
    [HAL_OK] = 0,
    [HAL_ERROR] = MP_EIO,
    [HAL_BUSY] = MP_EBUSY,
    [HAL_TIMEOUT] = MP_ETIMEDOUT,
};

NORETURN void mp_hal_raise(HAL_StatusTypeDef status) {
    mp_raise_OSError(mp_hal_status_to_errno_table[status]);
}

MP_WEAK uintptr_t mp_hal_stdio_poll(uintptr_t poll_flags) {
    uintptr_t ret = 0;
    if (MP_STATE_PORT(pyb_stdio_uart) != NULL) {
        mp_obj_t pyb_stdio_uart = MP_OBJ_FROM_PTR(MP_STATE_PORT(pyb_stdio_uart));
        int errcode;
        const mp_stream_p_t *stream_p = mp_get_stream(pyb_stdio_uart);
        ret = stream_p->ioctl(pyb_stdio_uart, MP_STREAM_POLL, poll_flags, &errcode);
    }
    return ret | mp_uos_dupterm_poll(poll_flags);
}

void flash_cache_commit(void);

void _mp_hal_stdout_tx_chr(int c);
int _mp_hal_stdin_rx_chr(void);
void _mp_hal_stdout_tx_strn(const char *str, int len);

MP_WEAK int mp_hal_stdin_rx_chr(void) {
    for (;;) {
        flash_cache_commit();
        #if 0
        #ifdef USE_HOST_MODE
        pyb_usb_host_process();
        int c = pyb_usb_host_get_keyboard();
        if (c != 0) {
            return c;
        }
        #endif
        #endif
        #if MICROPY_HW_ENABLE_RZ_USB
        // byte c;
        // if ((c = usbcdc_read()) != 0) {
        //    return c;
        // }
        #endif
        if (MP_STATE_PORT(pyb_stdio_uart) != NULL && uart_rx_any(MP_STATE_PORT(pyb_stdio_uart))) {
            return uart_rx_char(MP_STATE_PORT(pyb_stdio_uart));
        }
        int dupterm_c = mp_uos_dupterm_rx_chr();
        if (dupterm_c >= 0) {
            return dupterm_c;
        }
        MICROPY_EVENT_POLL_HOOK
    }
}

MP_WEAK void mp_hal_stdout_tx_strn(const char *str, size_t len) {
    if (MP_STATE_PORT(pyb_stdio_uart) != NULL) {
        uart_tx_strn(MP_STATE_PORT(pyb_stdio_uart), str, len);
    }
    #if 0 && defined(USE_HOST_MODE) && MICROPY_HW_HAS_LCD
    lcd_print_strn(str, len);
    #endif
    // #if MICROPY_HW_ENABLE_RZ_USB
    // uint8_t *p = (uint8_t *)str;
    // while (len--) {
    //    usbcdc_write(*p++);
    // }
    // #endif

    mp_uos_dupterm_tx_strn(str, len);
}


void mp_hal_ticks_cpu_enable(void) {
}

void mp_hal_pin_config(mp_hal_pin_obj_t pin_obj, uint32_t mode, uint32_t pull, uint32_t alt) {
    rz_gpio_config(pin_obj->id, mode, pull, alt);
}

bool mp_hal_pin_config_alt(mp_hal_pin_obj_t pin, uint32_t mode, uint32_t pull, uint8_t fn, uint8_t unit) {
    const pin_af_obj_t *af = pin_find_af(pin, fn, unit);
    if (af == NULL) {
        return false;
    }
    mp_hal_pin_config(pin, mode, pull, af->idx);
    return true;
}

void mp_hal_pin_config_speed(mp_hal_pin_obj_t pin_obj, uint32_t speed) {
}

/*******************************************************************************/
// MAC address

// Generate a random locally administered MAC address (LAA)
void mp_hal_generate_laa_mac(int idx, uint8_t buf[6]) {
    #if 0
    // STM32
    uint8_t *id = (uint8_t *)MP_HAL_UNIQUE_ID_ADDRESS;
    buf[0] = 0x02; // LAA range
    buf[1] = (id[11] << 4) | (id[10] & 0xf);
    buf[2] = (id[9] << 4) | (id[8] & 0xf);
    buf[3] = (id[7] << 4) | (id[6] & 0xf);
    buf[4] = id[2];
    buf[5] = (id[0] << 2) | idx;
    #else
    #if defined(RX65N)
    uint8_t id[16];
    get_unique_id((uint8_t *)&id);
    buf[0] = 0x02;
    buf[1] = id[11];
    buf[2] = id[12];
    buf[3] = id[13];
    buf[4] = id[14];
    buf[5] = id[15];
    #else
    #if defined(MICROPY_HW_ETH_MAC_ADDRESS_0)
    buf[0] = MICROPY_HW_ETH_MAC_ADDRESS_0;
    buf[1] = MICROPY_HW_ETH_MAC_ADDRESS_1;
    buf[2] = MICROPY_HW_ETH_MAC_ADDRESS_2;
    buf[3] = MICROPY_HW_ETH_MAC_ADDRESS_3;
    buf[4] = MICROPY_HW_ETH_MAC_ADDRESS_4;
    buf[5] = MICROPY_HW_ETH_MAC_ADDRESS_5;
    #else
    uint32_t tick = utick();
    buf[0] = 0;
    buf[1] = 0;
    buf[2] = (uint8_t)(tick >> 24);
    buf[3] = (uint8_t)(tick >> 16);
    buf[4] = (uint8_t)(tick >> 8);
    buf[5] = (uint8_t)(tick);
    #endif
    #endif
    #endif
}

// A board can override this if needed
MP_WEAK void mp_hal_get_mac(int idx, uint8_t buf[6]) {
    mp_hal_generate_laa_mac(idx, buf);
}

void mp_hal_get_mac_ascii(int idx, size_t chr_off, size_t chr_len, char *dest) {
    static const char hexchr[16] = "0123456789ABCDEF";
    uint8_t mac[6];
    mp_hal_get_mac(idx, mac);
    for (; chr_len; ++chr_off, --chr_len) {
        *dest++ = hexchr[mac[chr_off >> 1] >> (4 * (1 - (chr_off & 1))) & 0xf];
    }
}

MP_REGISTER_ROOT_POINTER(struct _pyb_uart_obj_t *pyb_stdio_uart);
