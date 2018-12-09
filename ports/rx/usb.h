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
#endif

typedef enum {
    PYB_USB_STORAGE_MEDIUM_NONE = 0,
    PYB_USB_STORAGE_MEDIUM_FLASH,
    PYB_USB_STORAGE_MEDIUM_SDCARD,
} pyb_usb_storage_medium_t;
extern pyb_usb_storage_medium_t pyb_usb_storage_medium;
void pyb_usb_init0(void);

#endif /* PORTS_RX_USB_H_ */
