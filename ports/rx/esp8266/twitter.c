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

#include "base64encode.h"
#include "esp8266.h"
#include "urlencode.h"
#include "hmac.h"
#include "wifi.h"
#include "twitter.h"

#if MICROPY_PY_PYB_TWITTER

//#define DEBUG_TWITTER_NO_WIFI
//#define DEBUG_TWITTER_AUTH_STR
//#define DEBUG_TWITTER_STATUSES_UPDATE
//#define DEBUG_UPLOAD
//#define DEBUG_HTTP_POST
//#define DEBUG_TWITTER_PARAMS
//#define DEBUG_TWITTER_PARAMS_SORT
//#define DEBUG_PARAMS
//#define CHECK_AUTH_STRING
//#define DUMP_RESPONSE
//#define SKIP_STATUSES_UPLOAD

#ifndef NULL
#define NULL 0
#endif

#define NTP_URL	"ntp.nict.go.jp"
#define STR_AUTHORIZATION   "Authorization: "

#define MAX_MESSAGE_LENGTH 2048

#define ENCODE_BUF_MAX  (2048)
#define PARAM_BUF_MAX   512
#define SIG_KEY_MAX     256
#define SIG_DATA_MAX    512
#define SIG_STR_MAX     512
#define AUTH_STR_MAX    1024
#define BODY_MAX    2048

#define DIGEST_SIZE 20
static unsigned char digest[DIGEST_SIZE];

static char encode_buf[ENCODE_BUF_MAX];
static char param_buf[PARAM_BUF_MAX];
static char sig_key[SIG_KEY_MAX];
static char sig_data[SIG_DATA_MAX];
static char sig_str[SIG_STR_MAX];
static char auth_str[AUTH_STR_MAX];

static char body[BODY_MAX];

#if 0
#define BOUNDARY_MAX   20
#define UPLOAD_HEADER_MAX   128
#define MEDIA_BUF_MAX   100*1024

static char boundary_buf[BOUNDARY_MAX];
static char upload_header_buf[UPLOAD_HEADER_MAX];
static char upload_body_top[256];
static char upload_body_end[256];
static char media_buf[MEDIA_BUF_MAX];
#endif

twitter_t twitter_params;

typedef struct _PARAM {
    char *key;
    char *value;
} PARAM;

PARAM req_params[] = {
    { (char *)"oauth_consumer_key", (char *)"" },
    { (char *)"oauth_nonce", (char *)"" },
    { (char *)"oauth_signature_method", (char *)OAUTH_SIGNATURE_METHOD },
    { (char *)"oauth_timestamp", (char *)"" },
    { (char *)"oauth_token", (char *)"" },
    { (char *)"oauth_version", (char *)OAUTH_VERSION },
    { (char *)NULL, (char *)NULL }, };
#define REQ_PARAMS_SIZE (sizeof(req_params)/sizeof(PARAM))

PARAM statuses_oauth_params1[] = {
    { (char *)"oauth_consumer_key", (char *)"" },
    { (char *)"oauth_nonce", (char *)"" },
    { (char *)"oauth_signature_method", (char *)OAUTH_SIGNATURE_METHOD },
    { (char *)"oauth_timestamp", (char *)"" },
    { (char *)"oauth_token", (char *)"" },
    { (char *)"oauth_version", (char *)OAUTH_VERSION },
    { (char *)"status", (char *)"" },
    { (char *)"oauth_signature", (char *)"" },
    { (char *)NULL, (char *)NULL }, };
#define STATUSES_OAUTH_PARAMS1_SIZE (sizeof(statuses_oauth_params1)/sizeof(PARAM))

PARAM statuses_oauth_params2[] = {
    { (char *)"media_ids", (char *)"" },
    { (char *)"oauth_consumer_key", (char *)"" },
    { (char *)"oauth_nonce", (char *)"" },
    { (char *)"oauth_signature_method", (char *)OAUTH_SIGNATURE_METHOD },
    { (char *)"oauth_timestamp", (char *)"" },
    { (char *)"oauth_token", (char *)"" },
    { (char *)"oauth_version", (char *)OAUTH_VERSION },
    { (char *)"status", (char *)"" },
    { (char *)"oauth_signature", (char *)"" },
    { (char *)NULL, (char *)NULL }, };
#define STATUSES_OAUTH_PARAMS2_SIZE (sizeof(statuses_oauth_params2)/sizeof(PARAM))

PARAM upload_oauth_params[] = {
    { (char *)"oauth_consumer_key", (char *)"" },
    { (char *)"oauth_nonce", (char *)"" },
    { (char *)"oauth_signature_method", (char *)OAUTH_SIGNATURE_METHOD },
    { (char *)"oauth_timestamp", (char *)"" },
    { (char *)"oauth_token", (char *)"" },
    { (char *)"oauth_version", (char *)OAUTH_VERSION },
    { (char *)"oauth_signature", (char *)"" },
    { (char *)NULL, (char *)NULL }, };
#define UPLOAD_OAUTH_PARAMS_SIZE (sizeof(upload_oauth_params)/sizeof(PARAM))

static PARAM *param_get_by_key(PARAM *p, char *k) {
    while (p->key != NULL) {
        if (strcmp((const char *)(p->key), (const char *)k) == 0) {
            return p;
        }
        p++;
    }
    return NULL;
}

static void param_set_value(PARAM *p, char *k, char *v) {
    PARAM *t;
    t = param_get_by_key(p, k);
    if (t != NULL) {
        t->value = v;
    }
}

#if defined(DEBUG_TWITTER_PARAMS_SORT)
static void param_swap(PARAM *p, int a, int b) {
    PARAM holder;

    holder = p[a];
    p[a] = p[b];
    p[b] = holder;
}

void param_sort(PARAM *p, int left, int right) {
    int pivot;
    int i;
    int j;
    PARAM item;

    if (right - left == 1) {
        if (strcmp(p[left].key, p[right].key) > 0) {
            param_swap(p, left, right);
        }
        return;
    }
    pivot = (left + right) / 2;
    item = p[pivot];
    //printf ("Sorting %d:%d: midpoint %d, '%s'\n", left, right, pivot, key);
    param_swap(p, left, pivot);
    i = left + 1;
    j = right;
    while (i < j) {
        while (i <= right && strcmp(p[i].key, item.key) < 0) {
            i++;
        }
        while (j >= left && strcmp(p[j].key, item.key) > 0) {
            j--;
        }
        if (i < j) {
            param_swap(p, i, j);
        }
    }
    param_swap(p, left, j);
    if (left < j - 1) {
        param_sort(p, left, j - 1);
    }
    if (j + 1 < right) {
        param_sort(p, j + 1, right);
    }
}
#endif

#if defined(DEBUG_TWITTER_PARAMS)
static void print_param_value(PARAM *p) {
    while (p->key != NULL) {
        debug_printf("%s: %s\r\n", p->key, p->value);
        p++;
    }
}
#endif

static int get_param_url_encode_size(PARAM *p) {
    int size = 0;
    while (p->key != NULL) {
        size += get_url_encode_size(p->key);
        size += get_url_encode_size("=");       /* = */
        size += get_url_encode_size(p->value);
        p++;
        if (p->key != NULL) {
            size += get_url_encode_size(",");   /* , */
        }
    }
    return size;
}

static char *url_encode_static_buf(char *str) {
    if (get_url_encode_size(str) < ENCODE_BUF_MAX) {
        url_encode(str, encode_buf, ENCODE_BUF_MAX);
    } else {
#ifdef DEBUG_TWITTER
        debug_printf("ERR: encode buf short!\r\n");
#endif
    }
    return encode_buf;
}

static char *urlenccpy(char *dst, char *src) {
    int enc_size = get_url_encode_size(src) + 1;
    char *enc = (char *)malloc(enc_size);
    if (enc) {
        url_encode(src, enc, enc_size);
        strcpy(dst, enc);
        free(enc);
    } else {
#ifdef DEBUG_TWITTER
        debug_printf("ERR: malloc failed!\r\n");
#endif
    }
    return dst;
}

static char *urlenccat(char *dst, char *src) {
    int enc_size = get_url_encode_size(src) + 1;
    char *enc = (char *)malloc(enc_size);
    if (enc) {
        url_encode(src, enc, enc_size);
        strcat(dst, enc);
        free(enc);
    } else {
#ifdef DEBUG_TWITTER
        debug_printf("ERR: malloc failed!\r\n");
#endif
    }
    return dst;
}

static void create_signature(char *sig_str, int sig_size, char *sec1, char *sec2,
    char *req_method, char *req_url, PARAM *params) {
    int sig_key_len;
    int sig_data_len;
    int sig_str_len;
    int size = 0;
    int calc_sig_key_len =
        get_url_encode_size(sec1) + get_url_encode_size(sec2) + 1;
    int calc_param_len = get_param_url_encode_size(params);
    int calc_sig_data_len =
        get_url_encode_size(req_method) + get_url_encode_size(req_url) + 1 +
        calc_param_len + 1;
    /* sig key */
    url_encode(sec1, encode_buf, ENCODE_BUF_MAX);
    strcpy(sig_key, encode_buf);
    strcat(sig_key, "&");
    url_encode(sec2, encode_buf, ENCODE_BUF_MAX);
    strcat(sig_key, encode_buf);
    /* params */
    strcpy(param_buf, "");
    while (params->key != NULL) {
        strcat(param_buf, params->key);
        strcat(param_buf, "=");
        strcat(param_buf, params->value);
        params++;
        if (params->key != NULL) {
            strcat(param_buf, "&");
        }
    }
    /* sig data */
    strcpy(sig_data, req_method);
    strcat(sig_data, "&");
    url_encode(req_url, encode_buf, ENCODE_BUF_MAX);
    strcat(sig_data, encode_buf);
    strcat(sig_data, "&");
    url_encode(param_buf, encode_buf, ENCODE_BUF_MAX);
#ifdef DEBUG_TWITTER
    debug_printf("param_len act:%d calc:%d\r\n", strlen(encode_buf), calc_param_len);
#endif
    strcat(sig_data, encode_buf);
    sig_key_len = strlen(sig_key);
    sig_data_len = strlen(sig_data);
#ifdef DEBUG_TWITTER
    debug_printf("sig_key_len act:%d calc:%d\r\n", sig_key_len, calc_sig_key_len);
    debug_printf("sig_data_len act:%d calc:%d\r\n", sig_data_len, calc_sig_data_len);
#endif
    hmac_sha1((unsigned char *)sig_key,
        sig_key_len,
        (unsigned char *)sig_data,
        sig_data_len,
        (unsigned char *)digest);
    base64_encode(digest, DIGEST_SIZE, sig_str, &sig_str_len);
}

static void create_oauth_params(char *oauth_str, PARAM *params) {
    strcpy(oauth_str, "OAuth ");
    strcpy(param_buf, "");
    while (params->key != NULL) {
        url_encode(params->key, encode_buf, ENCODE_BUF_MAX);
        strcat(param_buf, encode_buf);
        strcat(param_buf, "=");
        url_encode(params->value, encode_buf, ENCODE_BUF_MAX);
        strcat(param_buf, encode_buf);
        params++;
        if (params->key != NULL) {
            strcat(param_buf, ",");
        }
    }
    strcat(oauth_str, param_buf);
}

static void _statuses_update(char *ckey, char *csec, char *akey, char *asec, char *str, char *media_id_string) {
    char timestamp_str[11];
#if defined (DEBUG_TWITTER_NO_WIFI)
    unsigned int timestamp = 1542801184;
#else
    unsigned int timestamp = (unsigned int)ntp(NTP_URL, 1);
#endif
    //timestamp -= 2208988800;
    sprintf(timestamp_str, "%u", (unsigned int)timestamp);
    param_set_value((PARAM *)req_params, (char *)"oauth_consumer_key", (char *)ckey);
    param_set_value((PARAM *)req_params, (char *)"oauth_nonce", (char *)timestamp_str);
    param_set_value((PARAM *)req_params, (char *)"oauth_timestamp", (char *)timestamp_str);
    param_set_value((PARAM *)req_params, (char *)"oauth_token", (char *)akey);
#if defined(DEBUG_TWITTER_PARAMS)
    print_param_value((PARAM *)req_params);
#endif
    create_signature((char *)sig_str,
        SIG_STR_MAX,
        (char *)csec,
        (char *)asec,
        (char *)"POST",
        (char *)TWITTER_API_UPDATE,
        req_params);
#ifdef DEBUG_TWITTER
    debug_printf("sig_str: %s\r\n", sig_str);
#endif
    if ((media_id_string != NULL) && (strlen(media_id_string) > 0)) {
        param_set_value((PARAM *)statuses_oauth_params2, (char *)"oauth_consumer_key", (char *)ckey);
        param_set_value((PARAM *)statuses_oauth_params2, (char *)"oauth_nonce", (char *)timestamp_str);
        param_set_value((PARAM *)statuses_oauth_params2, (char *)"oauth_timestamp", (char *)timestamp_str);
        param_set_value((PARAM *)statuses_oauth_params2, (char *)"oauth_token", (char *)akey);
        param_set_value((PARAM *)statuses_oauth_params2, (char *)"oauth_signature", (char *)sig_str);
        param_set_value((PARAM *)statuses_oauth_params2, (char *)"media_ids", (char *)media_id_string);
        param_set_value((PARAM *)statuses_oauth_params2, (char *)"status", (char *)str);
#if defined(DEBUG_TWITTER_PARAMS_SORT)
        #if defined(DEBUG_TWITTER_PARAMS)
        print_param_value((PARAM *)statuses_oauth_params2);
#endif
        param_sort(statuses_oauth_params2, 0, STATUSES_OAUTH_PARAMS2_SIZE-1 );
#if defined(DEBUG_TWITTER_PARAMS)
        print_param_value((PARAM *)statuses_oauth_params2);
#endif
#endif
        create_oauth_params((char *)auth_str, (PARAM *)statuses_oauth_params2);
    } else {
        param_set_value((PARAM *)statuses_oauth_params1, (char *)"oauth_consumer_key", (char *)ckey);
        param_set_value((PARAM *)statuses_oauth_params1, (char *)"oauth_nonce", (char *)timestamp_str);
        param_set_value((PARAM *)statuses_oauth_params1, (char *)"oauth_timestamp", (char *)timestamp_str);
        param_set_value((PARAM *)statuses_oauth_params1, (char *)"oauth_token", (char *)akey);
        param_set_value((PARAM *)statuses_oauth_params1, (char *)"oauth_signature", (char *)sig_str);
        param_set_value((PARAM *)statuses_oauth_params1, (char *)"status", (char *)str);
#if defined(DEBUG_TWITTER_PARAMS_SORT)
#if defined(DEBUG_TWITTER_PARAMS)
        debug_printf("Before sort\r\n");
        print_param_value((PARAM *)statuses_oauth_params1);
#endif
        param_sort(statuses_oauth_params1, 0, STATUSES_OAUTH_PARAMS1_SIZE-1 );
#if defined(DEBUG_TWITTER_PARAMS)
        debug_printf("After sort\r\n");
        print_param_value((PARAM *)statuses_oauth_params1);
#endif
#endif
        create_oauth_params((char *)auth_str, (PARAM *)statuses_oauth_params1);
    }
#if defined(DEBUG_TWITTER_AUTH_STR)
    debug_printf("auth_str:%s\r\n", auth_str);
#endif
}

#ifdef DEBUG_PARAMS
// expected result
// signature = mu4s4b2t4T0HsjD0z0J749fMGPA=
static void test_create_signature(void) {
    PARAM param[] = {{(char *)"name", (char *)"BBB"},
        {(char *)"text", (char *)"CCC"},
        {(char *)"title", (char *)"AAA"},
        {(char *)NULL, (char *)NULL}};
    create_signature((char *)sig_str,
        SIG_STR_MAX,
        (char *)"bbbbbb",
        (char *)"dddddd",
        (char *)"POST",
        (char *)"http://example.com/sample.php",
        (PARAM *)param);
    debug_printf("sig_str:%s\r\n", sig_str);
}

static void test_create_oauth_params(void) {
    PARAM param[] = {{(char *)"name", (char *)"BBB"},
        {(char *)"text", (char *)"CCC"},
        {(char *)"title", (char *)"AAA"},
        {(char *)NULL, (char *)NULL}};
    create_oauth_params((char *)auth_str, (PARAM *)param);
    debug_printf("auth_str:%s\r\n", auth_str);
}
#endif

#ifdef DUMP_RESPONSE
static void dump_response(HttpResponse* res) {
    mbedtls_printf("Status: %d - %s\n", res->get_status_code(), res->get_status_message().c_str());

    mbedtls_printf("Headers:\n");
    for (size_t ix = 0; ix < res->get_headers_length(); ix++) {
        mbedtls_printf("\t%s: %s\n", res->get_headers_fields()[ix]->c_str(), res->get_headers_values()[ix]->c_str());
    }
    mbedtls_printf("\nBody (%d bytes):\n\n%s\n", res->get_body_length(), res->get_body_as_string().c_str());
}
#endif

void twitter_api_init() {
#ifdef DEBUG_PARAMS
    test_create_signature();
    test_create_oauth_params();
#endif
}

void twitter_api_deinit() {
}

void twitter_api_set_keys(char *cons_key, char *cons_sec, char *accs_key, char *accs_sec) {
    twitter_params._cons_key = cons_key;
    twitter_params._cons_sec = cons_sec;
    twitter_params._accs_key = accs_key;
    twitter_params._accs_sec = accs_sec;
}

void twitter_api_statuses_update(char *str, char *media_id_string) {
    char *strDFname = (char *)NULL;
    char *head[3];
    int size;
    twitter_t *t = &twitter_params;
    _statuses_update(t->_cons_key, t->_cons_sec, t->_accs_key, t->_accs_sec, str, media_id_string);
    head[0] = (char *)"User-Agent: gr-citurs";
    head[1] = (char *)"Content-Type: application/x-www-form-urlencoded";
    size = strlen((char *)STR_AUTHORIZATION) + strlen(auth_str) + 1;
    head[2] = (char *)malloc(size);
    if (!(head[2])) {
        return;
    }
    strcpy(head[2], (char *)STR_AUTHORIZATION);
    strcat(head[2], auth_str);
#ifdef DEBUG_TWITTER
    debug_printf("Header[2]: %s\r\n", head[2]);
#endif
    strcpy(body, "status=");
    url_encode(str, encode_buf, ENCODE_BUF_MAX);
    strcat(body, encode_buf);
    strcat(body, "\r\n");
#if defined (DEBUG_TWITTER_NO_WIFI)
#else
    esp8266_post(TWITTER_API_UPDATE_STR, body, "RESPONSE.TXT", 3, head, 1);
#endif
#ifdef DEBUG_TWITTER_STATUSES_UPDATE
    debug_printf("statuses_update:ret=%d\r\n", ret);
#endif
    if (head[2]) {
        free((void * )head[2]);
    }
}

#if 0
static void create_boundary(void) {
    unsigned int irandom;
    irandom = (unsigned int)rand();
    itoa(irandom, (char *)boundary_buf, 16);
}

static void _upload(NetworkInterface *iface, char *ckey, char *csec, char *akey, char *asec, char *buf, int size) {
    char timestamp_str[11];
    NTPClient ntp( iface);
    unsigned int timestamp = (unsigned int)ntp.get_timestamp();
    //timestamp -= 2208988800;
    sprintf(timestamp_str, "%u", (unsigned int)timestamp);
    param_set_value((PARAM *)req_params, (char *)"oauth_consumer_key", (char *)ckey);
    param_set_value((PARAM *)req_params, (char *)"oauth_nonce", (char *)timestamp_str);
    param_set_value((PARAM *)req_params, (char *)"oauth_timestamp", (char *)timestamp_str);
    param_set_value((PARAM *)req_params, (char *)"oauth_token", (char *)akey);
    //print_param_value((PARAM *)req_params);
    create_signature((char *)sig_str, SIG_STR_MAX,
        (char *)csec, (char *)asec, (char *)"POST", (char *)TWITTER_API_UPLOAD, req_params);
    //printf("sig_str: %s\n", sig_str);
    param_set_value((PARAM *)upload_oauth_params, (char *)"oauth_consumer_key", (char *)ckey);
    param_set_value((PARAM *)upload_oauth_params, (char *)"oauth_nonce", (char *)timestamp_str);
    param_set_value((PARAM *)upload_oauth_params, (char *)"oauth_timestamp", (char *)timestamp_str);
    param_set_value((PARAM *)upload_oauth_params, (char *)"oauth_token", (char *)akey);
    param_set_value((PARAM *)upload_oauth_params, (char *)"oauth_signature", (char *)sig_str);
    //print_param_value((PARAM *)oauth_params);
    create_oauth_params((char *)auth_str, (PARAM *)upload_oauth_params);
#ifdef CHECK_AUTH_STRING
    printf("auth_str: %s\r\n", auth_str);
#endif
    create_boundary();
    sprintf(upload_body_top, "--%s\r\n", boundary_buf);
    strcat(upload_body_top, "Content-Disposition: form-data; name=\"media_data\"; \r\n\r\n");
    sprintf(upload_body_end, "\r\n--%s--\r\n\r\n", boundary_buf);
    sprintf(upload_header_buf, "multipart/form-data; boundary=%s", boundary_buf);
}

static void get_media_id_string(char *body, char *media_id_string) {
    static char key[] = "media_id_string";
    char *start = body;
    char *end;
    int len;

    media_id_string[0] = 0;
    if ((start = strstr(body, (char *)key)) != NULL) {
        start += strlen(key) + 3;
        if ((end = strchr(start, '"')) != NULL) {
            len = end - start;
            strncpy(media_id_string, start, len);
            media_id_string[len] = 0;
        }
    }
}

void Twitter::upload(char *media_id_string, char *buf, int size)
{
    int idx;
    int encode_len;

#ifdef DEBUG_UPLOAD
    printf("\n----- Twitter image upload start -----\n");
#endif
    _upload(_iface, _cons_key, _cons_sec, _accs_key, _accs_sec, buf, size);

    idx = strlen(upload_body_top);
    strcpy(media_buf, upload_body_top);
    base64_encode((const unsigned char *)buf, size, (char *)&media_buf[idx], &encode_len);
    idx += encode_len;
    strcpy((char *)&media_buf[idx], upload_body_end);
    idx += strlen(upload_body_end);
#ifdef DEBUG_UPLOAD
    printf("body size: %d\r\n", idx);
    //printf("body: %s\r\n", media_buf);
#endif
    HttpsRequest* post_req = new HttpsRequest(_iface, SSL_CA_PEM, HTTP_POST, TWITTER_API_UPLOAD);
#ifdef DEBUG_HTTP_POST
    post_req->set_debug(true);
#endif
    post_req->set_header("User-Agent", "gr-peach");
    post_req->set_header("Content-Type", upload_header_buf);
    post_req->set_header("Authorization", auth_str);
    HttpResponse* post_res = post_req->send((const void *)media_buf, idx);
    if (!post_res) {
        printf("HttpRequest failed (error code %d)\n", post_req->get_error());
        return;
    }
#ifdef DUMP_RESPONSE
    dump_response(post_res);
#endif
    get_media_id_string((char *)post_res->get_body_as_string().c_str(), (char *)media_id_string);
#ifdef DEBUG_UPLOAD
    printf("media_id_string: %s\r\n", media_id_string);
    printf("\n----- Twitter image upload end -----\n");
#endif
    delete post_req;
}

void Twitter::upload_and_statuses_update(char *str, char *media_id_string, char *buf, int size)
{
    upload(media_id_string, buf, size);
#ifndef SKIP_STATUSES_UPLOAD
    statuses_update(str, media_id_string);
#endif
}
#endif

STATIC mp_obj_t twitter_init(size_t n_args, const mp_obj_t *args) {
    twitter_api_init();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(twitter_init_obj, 0, 0, twitter_init);

STATIC mp_obj_t twitter_set_keys(size_t n_args, const mp_obj_t *args) {
    twitter_api_set_keys(mp_obj_str_get_str(args[0]),
        mp_obj_str_get_str(args[1]),
        mp_obj_str_get_str(args[2]),
        mp_obj_str_get_str(args[3]));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(twitter_set_keys_obj, 4, 4, twitter_set_keys);

STATIC mp_obj_t twitter_statuses_update(size_t n_args, const mp_obj_t *args) {
    if (n_args == 1) {
        twitter_api_statuses_update(mp_obj_str_get_str(args[0]), "");
    } else {
        twitter_api_statuses_update(mp_obj_str_get_str(args[0]), args[1]);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(twitter_statuses_update_obj, 1, 2, twitter_statuses_update);

STATIC const mp_map_elem_t twitter_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_twitter) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_init), (mp_obj_t)&twitter_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_keys), (mp_obj_t)&twitter_set_keys_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_statuses_update), (mp_obj_t)&twitter_statuses_update_obj },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_twitter_globals, twitter_globals_table);

const mp_obj_module_t mp_module_twitter = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_twitter_globals,
};
#endif
