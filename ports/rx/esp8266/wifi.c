/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
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

#include <stdio.h>
#include <string.h>

#include "py/nlr.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/binary.h"
#include "portmodules.h"

#include "common.h"
#include "esp8266.h"
#include "ntp.h"
#include "wifi.h"

#if MICROPY_HW_HAS_ESP8266

#ifdef DEBUG_WIFI
#  define DEBUG_PRINT(m,v)     { debug_printf("%s:%d\r\n", m, v); }
#  define DEBUG_PRINT1(a)      { debug_printf("%s", a); }
#  define DEBUG_PRINTLN1(a)    { debug_printf("%s\r\n", a); }
#else
#  define DEBUG_PRINT(m,v)      // do nothing
#  define DEBUG_PRINT1(s)       // do nothing
#  define DEBUG_PRINTLN1(s)     // do nothing
#endif

// ===== micropyhton function in module definition

STATIC mp_obj_t wifi_init(void) {
    int ret;
    ret = esp8266_init();
    return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(wifi_init_obj, wifi_init);

STATIC mp_obj_t wifi_version(void) {
    esp8266_version();
    return mp_obj_new_str(esp8266_data_ptr(), esp8266_data_len());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(wifi_version_obj, wifi_version);

STATIC mp_obj_t wifi_set_mode(mp_obj_t mode) {
    esp8266_cwmode(mp_obj_get_int(mode));
    return mp_obj_new_str(esp8266_data_ptr(), esp8266_data_len());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(wifi_set_mode_obj, wifi_set_mode);

STATIC mp_obj_t wifi_serialout(mp_obj_t mode, mp_obj_t num) {
    esp8266_serialout(mp_obj_get_int(mode), mp_obj_get_int(num));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(wifi_serialout_obj, wifi_serialout);

STATIC mp_obj_t wifi_at(mp_obj_t mode, mp_obj_t num, mp_obj_t s) {
    esp8266_at(mp_obj_get_int(mode), mp_obj_get_int(num), mp_obj_str_get_str(s));
    return mp_obj_new_str(esp8266_data_ptr(), esp8266_data_len());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(wifi_at_obj, wifi_at);

STATIC mp_obj_t wifi_connect(mp_obj_t ssid, mp_obj_t pass) {
    esp8266_cwjap(mp_obj_str_get_str(ssid), mp_obj_str_get_str(pass));
    return mp_obj_new_str(esp8266_data_ptr(), esp8266_data_len());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(wifi_connect_obj, wifi_connect);

#if 0
STATIC mp_obj_t wifi_softap(mp_obj_t ssid, mp_obj_t pass, mp_obj_t ch, mp_obj_t enc) {
    esp8266_softap(mp_obj_str_get_str(ssid), mp_obj_str_get_str(pass), mp_obj_get_int(ch), mp_obj_get_int(enc));
    return mp_const_none;
}
//STATIC MP_DEFINE_CONST_FUN_OBJ_4(wifi_softap_obj, wifi_softap);
#endif

STATIC mp_obj_t wifi_connectedip(void) {
    esp8266_connectedip();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(wifi_connectedip_obj, wifi_connectedip);

STATIC mp_obj_t wifi_dhcp(mp_obj_t mode, mp_obj_t bl) {
    esp8266_dhcp(mp_obj_get_int(mode), mp_obj_get_int(bl));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(wifi_dhcp_obj, wifi_dhcp);

STATIC mp_obj_t wifi_ipconfig(void) {
    esp8266_cifsr();
    return mp_obj_new_str(esp8266_data_ptr(), esp8266_data_len());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(wifi_ipconfig_obj, wifi_ipconfig);

#if 0
STATIC mp_obj_t wifi_bypass(void) {
    esp8266_bypass();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(wifi_bypass_obj, wifi_bypass);
#endif

STATIC mp_obj_t wifi_disconnect(void) {
    esp8266_disconnect();
    return mp_obj_new_str(esp8266_data_ptr(), esp8266_data_len());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(wifi_disconnect_obj, wifi_disconnect);

STATIC mp_obj_t wifi_multiconnect(mp_obj_t mode) {
    esp8266_multiconnect(mp_obj_get_int(mode));
    return mp_obj_new_str(esp8266_data_ptr(), esp8266_data_len());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(wifi_multiconnect_obj, wifi_multiconnect);

// esp8266_get(unsigned char *strURL, unsigned char **hes, int n, int ssl)
STATIC mp_obj_t wifi_http_get(size_t n_args, const mp_obj_t *args) {
    char **header = (char **)NULL;
    mp_obj_t *items;
    int len = 0;
    int i;
    int ret;
    if (n_args == 2) {
        mp_obj_get_array(args[1], (size_t *)&len, &items);
        if (len > 0) {
            header = (char **)malloc(len * sizeof(char *));
            if (header == 0) {
                return mp_const_none;
            }
            for (i = 0; i < len; i++) {
                header[i] = (char *)mp_obj_str_get_str(items[i]);
            }
        }
    }
    ret = esp8266_get(mp_obj_str_get_str(args[0]),  /* url */
        header,
        len,
        0);
    if (header != 0) {
        free(header);
    }
    return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(wifi_http_get_obj, 1, 2, wifi_http_get);

STATIC mp_obj_t wifi_https_get(size_t n_args, const mp_obj_t *args) {
    char **header = (char **)NULL;
    mp_obj_t *items;
    int len = 0;
    int i;
    int ret;
    if (n_args == 2) {
        mp_obj_get_array(args[1], (size_t *)&len, &items);
        if (len > 0) {
            header = (char **)malloc(len * sizeof(char *));
            if (header == 0) {
                return mp_const_none;
            }
            for (i = 0; i < len; i++) {
                header[i] = (char *)mp_obj_str_get_str(items[i]);
            }
        }
    }
    ret = esp8266_get(mp_obj_str_get_str(args[0]),  /* url */
        header,
        len,
        1);
    if (header != 0) {
        free(header);
    }
    return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(wifi_https_get_obj, 1, 2, wifi_https_get);

#if MICROPY_HW_HAS_SDCARD
STATIC mp_obj_t wifi_http_get_sd(size_t n_args, const mp_obj_t *args) {
    char **header = (char **)NULL;
    mp_obj_t *items;
    int len = 0;
    int i;
    int ret;
    if (n_args == 3) {
        mp_obj_get_array(args[2], (size_t *)&len, &items);
        if (len > 0) {
            header = (char **)malloc(len * sizeof(char *));
            if (header == 0) {
                return mp_const_none;
            }
            for (i = 0; i < len; i++) {
                header[i] = (char *)mp_obj_str_get_str(items[i]);
            }
        }
    }
    ret = esp8266_get_sd(mp_obj_str_get_str(args[0]),   /* url */
        mp_obj_str_get_str(args[1]),
        len,
        header,
        0);
    if (header != 0) {
        free(header);
    }
    return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(wifi_http_get_sd_obj, 2, 3, wifi_http_get_sd);

STATIC mp_obj_t wifi_https_get_sd(size_t n_args, const mp_obj_t *args) {
    char **header = (char **)NULL;
    mp_obj_t *items;
    int len = 0;
    int i;
    int ret;
    if (n_args == 3) {
        mp_obj_get_array(args[2], (size_t *)&len, &items);
        if (len > 0) {
            header = (char **)malloc(len * sizeof(char *));
            if (header == 0) {
                return mp_const_none;
            }
            for (i = 0; i < len; i++) {
                header[i] = (char *)mp_obj_str_get_str(items[i]);
            }
        }
    }
    ret = esp8266_get_sd(mp_obj_str_get_str(args[0]),   /* url */
        mp_obj_str_get_str(args[1]),
        len,
        header,
        1);
    if (header != 0) {
        free(header);
    }
    return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(wifi_https_get_sd_obj, 2, 3, wifi_https_get_sd);
#endif

STATIC mp_obj_t wifi_udpopen(size_t n_args, const mp_obj_t *args) {
    esp8266_udpopen(mp_obj_get_int(args[0]),
        mp_obj_str_get_str(args[1]),
        mp_obj_get_int(args[2]),
        mp_obj_get_int(args[3]));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(wifi_udpopen_obj, 4, 4, wifi_udpopen);

#if MICROPY_HW_HAS_SDCARD
/* http_post_sd */
/* url */
/* header */
/* src file */
/* dst file */
STATIC mp_obj_t wifi_http_post_sd(size_t n_args, const mp_obj_t *args) {
    char **header = (char **)NULL;
    mp_obj_t *items;
    int len = 0;
    int i;
    int ret;
    mp_obj_get_array(args[1], (size_t *)&len, &items);
    if (len > 0) {
        header = (char **)malloc(len * sizeof(char *));
        if (header == 0) {
            return mp_const_none;
        }
        for (i = 0; i < len; i++) {
            header[i] = (char *)mp_obj_str_get_str(items[i]);
        }
    }
    ret = esp8266_post_sd(mp_obj_str_get_str(args[0]),  /* url */
        mp_obj_str_get_str(args[2]),                    /* src file */
        mp_obj_str_get_str(args[3]),                    /* dst file */
        len, header, 0);
    if (header != 0) {
        free(header);
    }
    return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(wifi_http_post_sd_obj, 4, 4, wifi_http_post_sd);

/* https_post_sd */
/* url */
/* header */
/* src file */
/* dst file */
STATIC mp_obj_t wifi_https_post_sd(size_t n_args, const mp_obj_t *args) {
    char **header = (char **)NULL;
    mp_obj_t *items;
    int len = 0;
    int i;
    int ret;
    mp_obj_get_array(args[1], (size_t *)&len, &items);
    if (len > 0) {
        header = (char **)malloc(len * sizeof(char *));
        if (header == 0) {
            return mp_const_none;
        }
        for (i = 0; i < len; i++) {
            header[i] = (char *)mp_obj_str_get_str(items[i]);
        }
    }
    ret = esp8266_post_sd(mp_obj_str_get_str(args[0]),  /* url */
        mp_obj_str_get_str(args[2]),                    /* src file */
        mp_obj_str_get_str(args[3]),                    /* dst file */
        len, header, 1);
    if (header != 0) {
        free(header);
    }
    return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(wifi_https_post_sd_obj, 4, 4, wifi_https_post_sd);
#endif

/* http_post */
/* url */
/* header */
/* data */
/* dst file */
STATIC mp_obj_t wifi_http_post(size_t n_args, const mp_obj_t *args) {
    char **header = (char **)NULL;
    mp_obj_t *items;
    int len = 0;
    int i;
    int ret;
    mp_obj_get_array(args[1], (size_t *)&len, &items);
    if (len > 0) {
        header = (char **)malloc(len * sizeof(char *));
        if (header == 0) {
            return mp_const_none;
        }
        for (i = 0; i < len; i++) {
            header[i] = (char *)mp_obj_str_get_str(items[i]);
        }
    }
    ret = esp8266_post(mp_obj_str_get_str(args[0]),
        (char *)mp_obj_str_get_str(args[2]),
        mp_obj_str_get_str(args[3]),
        len, header, 0);
    if (header != 0) {
        free(header);
    }
    return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(wifi_http_post_obj, 4, 4, wifi_http_post);

/* https_post */
/* url */
/* header */
/* data */
/* dst file */
STATIC mp_obj_t wifi_https_post(size_t n_args, const mp_obj_t *args) {
    char **header = (char **)NULL;
    mp_obj_t *items;
    int len = 0;
    int i;
    int ret;
    mp_obj_get_array(args[1], (size_t *)&len, &items);
    if (len > 0) {
        header = (char **)malloc(len * sizeof(char *));
        if (header == 0) {
            return mp_const_none;
        }
        for (i = 0; i < len; i++) {
            header[i] = (char *)mp_obj_str_get_str(items[i]);
        }
    }
    ret = esp8266_post(mp_obj_str_get_str(args[0]),
        (char *)mp_obj_str_get_str(args[2]),
        mp_obj_str_get_str(args[3]),
        len, header, 1);
    if (header != 0) {
        free(header);
    }
    return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(wifi_https_post_obj, 4, 4, wifi_https_post);

STATIC mp_obj_t wifi_cclose(mp_obj_t mode) {
    esp8266_cclose(mp_obj_get_int(mode));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(wifi_cclose_obj, wifi_cclose);

STATIC mp_obj_t wifi_send(mp_obj_t num, mp_obj_t data, mp_obj_t size) {
    esp8266_send(mp_obj_get_int(num), (char *)mp_obj_str_get_str(data), mp_obj_get_int(size));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(wifi_send_obj, wifi_send);

STATIC mp_obj_t wifi_recv(mp_obj_t num, mp_obj_t data, mp_obj_t size) {
    int len = mp_obj_get_int(size);
    esp8266_recv(mp_obj_get_int(num), (char *)mp_obj_str_get_str(data), &len);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(wifi_recv_obj, wifi_recv);

STATIC mp_obj_t wifi_ntp(size_t n_args, const mp_obj_t *args) {
    uint32_t val = 0;
    if (n_args == 1) {
        val = ntp(mp_obj_str_get_str(args[0]), 0);
    } else {
        val = ntp(NTP_DEFAULT_SERVER, 0);
    }
    return mp_obj_new_int_from_ull((unsigned long long)val);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(wifi_ntp_obj, 0, 1, wifi_ntp);

STATIC mp_obj_t wifi_ntp_unix(size_t n_args, const mp_obj_t *args) {
    uint32_t val = 0;
    if (n_args == 1) {
        val = ntp(mp_obj_str_get_str(args[0]), 1);
    } else {
        val = ntp(NTP_DEFAULT_SERVER, 1);
    }
    return mp_obj_new_int_from_ull((unsigned long long)val);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(wifi_ntp_unix_obj, 0, 1, wifi_ntp_unix);


// ===== micropyhton module definition

STATIC const mp_map_elem_t wifi_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_wifi) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_init), (mp_obj_t)&wifi_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_version), (mp_obj_t)&wifi_version_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_mode), (mp_obj_t)&wifi_set_mode_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_serialout), (mp_obj_t)&wifi_serialout_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_at), (mp_obj_t)&wifi_at_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_connect), (mp_obj_t)&wifi_connect_obj },
//    { MP_OBJ_NEW_QSTR(MP_QSTR_softap), (mp_obj_t)&wifi_softap_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_connectedip), (mp_obj_t)&wifi_connectedip_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_dhcp), (mp_obj_t)&wifi_dhcp_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ipconfig), (mp_obj_t)&wifi_ipconfig_obj },
//    { MP_OBJ_NEW_QSTR(MP_QSTR_bypass), (mp_obj_t)&wifi_bypass_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_disconnect), (mp_obj_t)&wifi_disconnect_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_multiconnect), (mp_obj_t)&wifi_multiconnect_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_http_get), (mp_obj_t)&wifi_http_get_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_https_get), (mp_obj_t)&wifi_https_get_obj },
#if MICROPY_HW_HAS_SDCARD
    { MP_OBJ_NEW_QSTR(MP_QSTR_http_get_sd), (mp_obj_t)&wifi_http_get_sd_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_https_get_sd), (mp_obj_t)&wifi_https_get_sd_obj },
#endif
    { MP_OBJ_NEW_QSTR(MP_QSTR_udpopen), (mp_obj_t)&wifi_udpopen_obj },
#if MICROPY_HW_HAS_SDCARD
    { MP_OBJ_NEW_QSTR(MP_QSTR_http_post_sd), (mp_obj_t)&wifi_http_post_sd_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_https_post_sd), (mp_obj_t)&wifi_https_post_sd_obj },
#endif
    { MP_OBJ_NEW_QSTR(MP_QSTR_http_post), (mp_obj_t)&wifi_http_post_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_https_post), (mp_obj_t)&wifi_https_post_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_cclose), (mp_obj_t)&wifi_cclose_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_send), (mp_obj_t)&wifi_send_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_recv), (mp_obj_t)&wifi_recv_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ntp), (mp_obj_t)&wifi_ntp_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ntp_unix), (mp_obj_t)&wifi_ntp_unix_obj },
    //{ MP_OBJ_NEW_QSTR(MP_QSTR_helloObj), (mp_obj_t)&wifi_helloObj_type },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_wifi_globals, wifi_globals_table);

const mp_obj_module_t mp_module_wifi = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_wifi_globals,
};

#endif
