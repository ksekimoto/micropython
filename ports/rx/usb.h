/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014, 2015 Damien P. George
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

#ifndef PORTS_RX_USB_H_
#define PORTS_RX_USB_H_

#include "usb_common.h"
#include "usbdescriptors.h"

#define PYB_USB_FLAG_USB_MODE_CALLED    (0x0002)

// Windows needs a different PID to distinguish different device configurations
#ifdef GRSAKURA
#define USBD_VID            (0x045B)
#define USBD_PID_CDC_MSC    (0x0234)
#define USBD_PID_CDC_HID    (0x0234)
#define USBD_PID_CDC        (0x0234)
#define USBD_PID_MSC        (0x0234)
#define USBD_PID_CDC2_MSC   (0x0234)
#elif defined(GRCITRUS)
#define USBD_VID            (0x2A50)
#define USBD_PID_CDC_MSC    (0x0277)
#define USBD_PID_CDC_HID    (0x0277)
#define USBD_PID_CDC        (0x0277)
#define USBD_PID_MSC        (0x0277)
#define USBD_PID_CDC2_MSC   (0x0277)
#elif defined(GRROSE)
#define USBD_VID            (0x045B)
#define USBD_PID_CDC_MSC    (0x025A)
#define USBD_PID_CDC_HID    (0x025A)
#define USBD_PID_CDC        (0x025A)
#define USBD_PID_MSC        (0x025A)
#define USBD_PID_CDC2_MSC   (0x025A)
#endif

// These can be or'd together (but not all combinations may be available)
#define USBD_MODE_IFACE_MASK    (0x7f)
#define USBD_MODE_IFACE_CDC(i)  (0x01 << (i))
#define USBD_MODE_IFACE_HID     (0x10)
#define USBD_MODE_IFACE_MSC     (0x20)
#define USBD_MODE_HIGH_SPEED    (0x80)

// Convenience macros for supported mode combinations
// modified
#define USBD_MODE_CDC       1
#define USBD_MODE_CDC2      2
#define USBD_MODE_CDC3      3
#define USBD_MODE_CDC_HID   4
#define USBD_MODE_CDC_MSC   5
#define USBD_MODE_CDC2_MSC  6
#define USBD_MODE_CDC3_MSC  7
#define USBD_MODE_HID       8
#define USBD_MODE_MSC       9
#define USBD_MODE_MSC_HID   10

typedef enum {
    PYB_USB_STORAGE_MEDIUM_NONE = 0,
    PYB_USB_STORAGE_MEDIUM_FLASH,
    PYB_USB_STORAGE_MEDIUM_SDCARD,
} pyb_usb_storage_medium_t;
extern mp_uint_t pyb_usb_flags;
extern pyb_usb_storage_medium_t pyb_usb_storage_medium;
extern const struct _mp_rom_obj_tuple_t pyb_usb_hid_mouse_obj;
extern const struct _mp_rom_obj_tuple_t pyb_usb_hid_keyboard_obj;
extern const mp_obj_type_t pyb_usb_hid_type;

MP_DECLARE_CONST_FUN_OBJ_KW(pyb_usb_mode_obj);
void pyb_usb_init0(void);
bool pyb_usb_dev_init(uint16_t vid, uint16_t pid, uint8_t mode, char *hid_info);
void pyb_usb_dev_deinit(void);
void usb_init(void);

/* USB HID */
#define USBD_HID_MOUSE_MAX_PACKET          (4)
#define USBD_HID_MOUSE_REPORT_DESC_SIZE    (74)
#define USBD_HID_KEYBOARD_MAX_PACKET       (8)
#define USBD_HID_KEYBOARD_REPORT_DESC_SIZE (63)
#define HID_DATA_FS_MAX_PACKET_SIZE  64

typedef struct _usbd_hid_itf_t {
    //usbd_hid_state_t base; // state for the base HID layer
    uint8_t buffer[2][HID_DATA_FS_MAX_PACKET_SIZE]; // pair of buffers to read individual packets into
    int8_t current_read_buffer; // which buffer to read from
    uint32_t last_read_len; // length of last read
    int8_t current_write_buffer; // which buffer to write to
} usbd_hid_itf_t;

typedef struct _usb_device_t {
    uint32_t enabled;
    usbd_hid_itf_t usbd_hid_itf;
} usb_device_t;

#endif /* PORTS_RX_USB_H_ */
