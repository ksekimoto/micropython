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

#include <stdarg.h>
#include <string.h>

#include "py/objstr.h"
#include "py/runtime.h"
#include "py/stream.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "bufhelper.h"
#include "usb.h"
#include "usb_hid.h"

const uint8_t USBD_HID_MOUSE_ReportDesc[USBD_HID_MOUSE_REPORT_DESC_SIZE];
const uint8_t USBD_HID_KEYBOARD_ReportDesc[USBD_HID_KEYBOARD_REPORT_DESC_SIZE];
// this will be persistent across a soft-reset
mp_uint_t pyb_usb_flags = 0;

usb_device_t usb_device = {0};
pyb_usb_storage_medium_t pyb_usb_storage_medium = PYB_USB_STORAGE_MEDIUM_NONE;

// predefined hid mouse data
STATIC const mp_obj_str_t pyb_usb_hid_mouse_desc_obj = {
    {&mp_type_bytes},
    0, // hash not valid
    USBD_HID_MOUSE_REPORT_DESC_SIZE,
    USBD_HID_MOUSE_ReportDesc,
};
const mp_rom_obj_tuple_t pyb_usb_hid_mouse_obj = {
    {&mp_type_tuple},
    5,
    {
        MP_ROM_INT(1), // subclass: boot
        MP_ROM_INT(2), // protocol: mouse
        MP_ROM_INT(USBD_HID_MOUSE_MAX_PACKET),
        MP_ROM_INT(8), // polling interval: 8ms
        MP_ROM_PTR(&pyb_usb_hid_mouse_desc_obj),
    },
};

// predefined hid keyboard data
STATIC const mp_obj_str_t pyb_usb_hid_keyboard_desc_obj = {
    {&mp_type_bytes},
    0, // hash not valid
    USBD_HID_KEYBOARD_REPORT_DESC_SIZE,
    USBD_HID_KEYBOARD_ReportDesc,
};
const mp_rom_obj_tuple_t pyb_usb_hid_keyboard_obj = {
    {&mp_type_tuple},
    5,
    {
        MP_ROM_INT(1), // subclass: boot
        MP_ROM_INT(1), // protocol: keyboard
        MP_ROM_INT(USBD_HID_KEYBOARD_MAX_PACKET),
        MP_ROM_INT(8), // polling interval: 8ms
        MP_ROM_PTR(&pyb_usb_hid_keyboard_desc_obj),
    },
};

void pyb_usb_init0(void) {
    mp_hal_set_interrupt_char(-1);
}

bool pyb_usb_dev_init(uint16_t vid, uint16_t pid, uint8_t mode, char *hid_info) {
    usb_device_t *usb_dev = &usb_device;
    if (!usb_dev->enabled) {
        SetPIDVID(pid, vid);
        SetHIDMode(mode);
        if (hid_info == NULL) {
            SetDefaultHIDReportDescriptor(mode);
        }
        usb_init();
        // only init USB once in the device's power-lifetime

        // set up the USBD state
        //USBD_HandleTypeDef *usbd = &usb_dev->hUSBDDevice;
        //usbd->id = MICROPY_HW_USB_MAIN_DEV;
        //usbd->dev_state  = USBD_STATE_DEFAULT;
        //usbd->pDesc = (USBD_DescriptorsTypeDef*)&USBD_Descriptors;
        //usbd->pClass = &USBD_CDC_MSC_HID;
        //usb_dev->usbd_cdc_msc_hid_state.pdev = usbd;
        //for (int i = 0; i < MICROPY_HW_USB_CDC_NUM; ++i) {
        //    usb_dev->usbd_cdc_msc_hid_state.cdc[i] = &usb_dev->usbd_cdc_itf[i].base;
        //}
        //usb_dev->usbd_cdc_msc_hid_state.hid = &usb_dev->usbd_hid_itf.base;
        //usbd->pClassData = &usb_dev->usbd_cdc_msc_hid_state;

        // configure the VID, PID and the USBD mode (interfaces it will expose)
        //int cdc_only = (mode & USBD_MODE_IFACE_MASK) == USBD_MODE_CDC;
        //USBD_SetVIDPIDRelease(&usb_dev->usbd_cdc_msc_hid_state, vid, pid, 0x0200, cdc_only);
        //if (USBD_SelectMode(&usb_dev->usbd_cdc_msc_hid_state, mode, hid_info) != 0) {
        //    return false;
        //}

        // Configure the MSC interface
        //const void *msc_unit_default[1];
        //if (msc_n == 0) {
        //    msc_n = 1;
        //    msc_unit = msc_unit_default;
        //    switch (pyb_usb_storage_medium) {
        //        #if MICROPY_HW_ENABLE_SDCARD
        //        case PYB_USB_STORAGE_MEDIUM_SDCARD:
        //            msc_unit_default[0] = &pyb_sdcard_type;
        //            break;
        //        #endif
        //        default:
        //            msc_unit_default[0] = &pyb_flash_type;
        //            break;
        //    }
        //}
        //usbd_msc_init_lu(msc_n, msc_unit);
        //USBD_MSC_RegisterStorage(&usb_dev->usbd_cdc_msc_hid_state, (USBD_StorageTypeDef*)&usbd_msc_fops);

        // start the USB device
        //USBD_LL_Init(usbd, (mode & USBD_MODE_HIGH_SPEED) != 0);
        //USBD_LL_Start(usbd);
        usb_dev->enabled = true;
    }

    return true;
}

void pyb_usb_dev_deinit(void) {
    usb_device_t *usb_dev = &usb_device;
    if (usb_dev->enabled) {
        //USBD_Stop(&usb_dev->hUSBDDevice);
        //USBD_DeInit(&usb_dev->hUSBDDevice);
        usb_dev->enabled = false;
    }
}

static uint8_t USBD_HID_SendReport(uint8_t *report, uint16_t len) {
    uint8_t ret = 0;
#if defined(USB_HID)
    USBHID_ReportIN(report, (int)len);
#endif
    return ret;
}

// Dummy
static uint8_t USBD_HID_ReceiveReport(uint8_t *report, uint16_t len) {
    uint8_t ret = 0;
#if defined(USB_HID)
    //USBHID_ReportOut(report, (int)len);
#endif
    return ret; // return read length
}

// Dummy
static int usbd_hid_rx_num(void) {
    return 1;
}


// Dummy
static int USBD_HID_CanSendReport(void) {
    return 1;
}

/******************************************************************************/
// MicroPython bindings for USB

/*
  Philosophy of USB driver and Python API: pyb.usb_mode(...) configures the USB
  on the board.  The USB itself is not an entity, rather the interfaces are, and
  can be accessed by creating objects, such as pyb.USB_VCP() and pyb.USB_HID().

  We have:

    pyb.usb_mode()          # return the current usb mode
    pyb.usb_mode(None)      # disable USB
    pyb.usb_mode('VCP')     # enable with VCP interface
    pyb.usb_mode('VCP+MSC') # enable with VCP and MSC interfaces
    pyb.usb_mode('VCP+HID') # enable with VCP and HID, defaulting to mouse protocol
    pyb.usb_mode('VCP+HID', vid=0xf055, pid=0x9800) # specify VID and PID
    pyb.usb_mode('VCP+HID', hid=pyb.hid_mouse)
    pyb.usb_mode('VCP+HID', hid=pyb.hid_keyboard)
    pyb.usb_mode('VCP+HID', pid=0x1234, hid=(subclass, protocol, max_packet_len, polling_interval, report_desc))

    vcp = pyb.USB_VCP() # get the VCP device for read/write
    hid = pyb.USB_HID() # get the HID device for write/poll

  Possible extensions:
    pyb.usb_mode('host', ...)
    pyb.usb_mode('OTG', ...)
    pyb.usb_mode(..., port=2) # for second USB port
*/

STATIC mp_obj_t pyb_usb_mode(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_mode, ARG_vid, ARG_pid, ARG_msc, ARG_hid, ARG_high_speed };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_mode, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_vid, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = USBD_VID} },
        { MP_QSTR_pid, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_msc, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = MP_ROM_PTR(&mp_const_empty_tuple_obj)} },
        { MP_QSTR_hid, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = MP_ROM_PTR(&pyb_usb_hid_mouse_obj)} },
        #if USBD_SUPPORT_HS_MODE
        { MP_QSTR_high_speed, MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = false} },
        #endif
    };

    // fetch the current usb mode -> pyb.usb_mode()
    if (n_args == 0) {
        //uint8_t mode = USBD_GetMode(&usb_device.usbd_cdc_msc_hid_state);
        uint8_t mode = USBD_MODE_MSC_HID;
        switch (mode & USBD_MODE_IFACE_MASK) {
            case USBD_MODE_CDC:
                return MP_OBJ_NEW_QSTR(MP_QSTR_VCP);
            case USBD_MODE_MSC:
                return MP_OBJ_NEW_QSTR(MP_QSTR_MSC);
            case USBD_MODE_HID:
                return MP_OBJ_NEW_QSTR(MP_QSTR_HID);
            case USBD_MODE_CDC_MSC:
                return MP_OBJ_NEW_QSTR(MP_QSTR_VCP_plus_MSC);
            case USBD_MODE_CDC_HID:
                return MP_OBJ_NEW_QSTR(MP_QSTR_VCP_plus_HID);
            case USBD_MODE_MSC_HID:
                return MP_OBJ_NEW_QSTR(MP_QSTR_MSC_plus_HID);
            default:
                return mp_const_none;
        }
    }

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // record the fact that the usb has been explicitly configured
    pyb_usb_flags |= PYB_USB_FLAG_USB_MODE_CALLED;

    // check if user wants to disable the USB
    if (args[ARG_mode].u_obj == mp_const_none) {
        // disable usb
        pyb_usb_dev_deinit();
        return mp_const_none;
    }

    // get mode string
    const char *mode_str = mp_obj_str_get_str(args[ARG_mode].u_obj);

    // hardware configured for USB device mode

    // get the VID, PID and USB mode
    // note: PID=-1 means select PID based on mode
    // note: we support CDC as a synonym for VCP for backward compatibility
    uint16_t vid = args[ARG_vid].u_int;
    mp_int_t pid = args[ARG_pid].u_int;
    uint8_t mode;
    if (strcmp(mode_str, "CDC+MSC") == 0 || strcmp(mode_str, "VCP+MSC") == 0) {
        if (pid == -1) {
            pid = USBD_PID_CDC_MSC;
        }
        mode = USBD_MODE_CDC_MSC;
    } else if (strcmp(mode_str, "CDC+HID") == 0 || strcmp(mode_str, "VCP+HID") == 0) {
        if (pid == -1) {
            pid = USBD_PID_CDC_HID;
        }
        mode = USBD_MODE_CDC_HID;
    } else if (strcmp(mode_str, "CDC") == 0 || strcmp(mode_str, "VCP") == 0) {
        if (pid == -1) {
            pid = USBD_PID_CDC;
        }
        mode = USBD_MODE_CDC;
    } else if (strcmp(mode_str, "MSC") == 0) {
        if (pid == -1) {
            pid = USBD_PID_MSC;
        }
        mode = USBD_MODE_MSC;
    } else {
        goto bad_mode;
    }

    // Get MSC logical units
    //size_t msc_n = 0;
    //const void *msc_unit[USBD_MSC_MAX_LUN];
    //if (mode & USBD_MODE_IFACE_MSC) {
    //    mp_obj_t *items;
    //    mp_obj_get_array(args[ARG_msc].u_obj, &msc_n, &items);
    //    if (msc_n > USBD_MSC_MAX_LUN) {
    //        mp_raise_ValueError("too many logical units");
    //    }
    //    for (size_t i = 0; i < msc_n; ++i) {
    //        mp_obj_type_t *type = mp_obj_get_type(items[i]);
    //        if (type == &pyb_flash_type
    //            #if MICROPY_HW_ENABLE_SDCARD
    //            || type == &pyb_sdcard_type
    //            #endif
    //            #if MICROPY_HW_ENABLE_MMCARD
    //            || type == &pyb_mmcard_type
    //            #endif
    //            ) {
    //            msc_unit[i] = type;
    //        } else {
    //            mp_raise_ValueError("unsupported logical unit");
    //        }
    //    }
    //}

    char *hid_info = (char *)NULL;
    // get hid info if user selected such a mode
    //USBD_HID_ModeInfoTypeDef hid_info;
    //if (mode & USBD_MODE_IFACE_HID) {
    //    mp_obj_t *items;
    //    mp_obj_get_array_fixed_n(args[ARG_hid].u_obj, 5, &items);
    //    hid_info.subclass = mp_obj_get_int(items[0]);
    //    hid_info.protocol = mp_obj_get_int(items[1]);
    //    hid_info.max_packet_len = mp_obj_get_int(items[2]);
    //    hid_info.polling_interval = mp_obj_get_int(items[3]);
    //    mp_buffer_info_t bufinfo;
    //    mp_get_buffer_raise(items[4], &bufinfo, MP_BUFFER_READ);
    //    hid_info.report_desc = bufinfo.buf;
    //    hid_info.report_desc_len = bufinfo.len;

        // need to keep a copy of this so report_desc does not get GC'd
    //    MP_STATE_PORT(pyb_hid_report_desc) = items[4];
    //}

    // init the USB device
    if (!pyb_usb_dev_init(vid, pid, mode, hid_info)) {
        goto bad_mode;
    }

    return mp_const_none;

bad_mode:
    mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("bad USB mode"));
}
MP_DEFINE_CONST_FUN_OBJ_KW(pyb_usb_mode_obj, 0, pyb_usb_mode);

/******************************************************************************/
// MicroPython bindings for USB HID

typedef struct _pyb_usb_hid_obj_t {
    mp_obj_base_t base;
    usb_device_t *usb_dev;
} pyb_usb_hid_obj_t;

STATIC const pyb_usb_hid_obj_t pyb_usb_hid_obj = {{&pyb_usb_hid_type}, &usb_device};

STATIC mp_obj_t pyb_usb_hid_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    // TODO raise exception if USB is not configured for HID

    // return the USB HID object
    return MP_OBJ_FROM_PTR(&pyb_usb_hid_obj);
}

/// \method recv(data, *, timeout=5000)
///
/// Receive data on the bus:
///
///   - `data` can be an integer, which is the number of bytes to receive,
///     or a mutable buffer, which will be filled with received bytes.
///   - `timeout` is the timeout in milliseconds to wait for the receive.
///
/// Return value: if `data` is an integer then a new buffer of the bytes received,
/// otherwise the number of bytes read into `data` is returned.
STATIC mp_obj_t pyb_usb_hid_recv(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_data,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 5000} },
    };

    // parse args
    //pyb_usb_hid_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_arg_val_t vals[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, vals);

    // get the buffer to receive into
    vstr_t vstr;
    mp_obj_t o_ret = pyb_buf_get_for_recv(vals[0].u_obj, &vstr);

    // receive the data
    // Dummy
    // ToDo: Implement USBD_HID_SendReport()
    int ret = USBD_HID_ReceiveReport((uint8_t*)vstr.buf, vstr.len);

    // return the received data
    if (o_ret != MP_OBJ_NULL) {
        return mp_obj_new_int(ret); // number of bytes read into given buffer
    } else {
        vstr.len = ret; // set actual number of bytes read
        return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr); // create a new buffer
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_usb_hid_recv_obj, 1, pyb_usb_hid_recv);

STATIC mp_obj_t pyb_usb_hid_send(mp_obj_t self_in, mp_obj_t report_in) {
    //pyb_usb_hid_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_buffer_info_t bufinfo;
    byte temp_buf[8];
    // get the buffer to send from
    // we accept either a byte array, or a tuple/list of integers
    if (!mp_get_buffer(report_in, &bufinfo, MP_BUFFER_READ)) {
        mp_obj_t *items;
        mp_obj_get_array(report_in, &bufinfo.len, &items);
        if (bufinfo.len > sizeof(temp_buf)) {
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("tuple/list too large for HID report; use bytearray instead"));
        }
        for (int i = 0; i < bufinfo.len; i++) {
            temp_buf[i] = mp_obj_get_int(items[i]);
        }
        bufinfo.buf = temp_buf;
    }

    // send the data
    if (0 == USBD_HID_SendReport(bufinfo.buf, bufinfo.len)) {
        return mp_obj_new_int(bufinfo.len);
    } else {
        return mp_obj_new_int(0);
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_usb_hid_send_obj, pyb_usb_hid_send);

// deprecated in favour of USB_HID.send
STATIC mp_obj_t pyb_hid_send_report(mp_obj_t arg) {
    return pyb_usb_hid_send(MP_OBJ_FROM_PTR(&pyb_usb_hid_obj), arg);
}
MP_DEFINE_CONST_FUN_OBJ_1(pyb_hid_send_report_obj, pyb_hid_send_report);

STATIC const mp_rom_map_elem_t pyb_usb_hid_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_send), MP_ROM_PTR(&pyb_usb_hid_send_obj) },
    { MP_ROM_QSTR(MP_QSTR_recv), MP_ROM_PTR(&pyb_usb_hid_recv_obj) },
};

STATIC MP_DEFINE_CONST_DICT(pyb_usb_hid_locals_dict, pyb_usb_hid_locals_dict_table);

STATIC mp_uint_t pyb_usb_hid_ioctl(mp_obj_t self_in, mp_uint_t request, uintptr_t arg, int *errcode) {
    //pyb_usb_hid_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_uint_t ret;
    if (request == MP_STREAM_POLL) {
        uintptr_t flags = arg;
        ret = 0;

        if ((flags & MP_STREAM_POLL_RD) && usbd_hid_rx_num() > 0) {
            ret |= MP_STREAM_POLL_RD;
        }
        if ((flags & MP_STREAM_POLL_WR) && USBD_HID_CanSendReport()) {
            ret |= MP_STREAM_POLL_WR;
        }

    } else {
        *errcode = MP_EINVAL;
        ret = MP_STREAM_ERROR;
    }
    return ret;
}

STATIC const mp_stream_p_t pyb_usb_hid_stream_p = {
    .ioctl = pyb_usb_hid_ioctl,
};

const mp_obj_type_t pyb_usb_hid_type = {
    { &mp_type_type },
    .name = MP_QSTR_USB_HID,
    .make_new = pyb_usb_hid_make_new,
    .protocol = &pyb_usb_hid_stream_p,
    .locals_dict = (mp_obj_dict_t*)&pyb_usb_hid_locals_dict,
};
