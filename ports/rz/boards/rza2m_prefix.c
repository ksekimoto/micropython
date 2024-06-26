// rx63n_prefix.c becomes the initial portion of the generated pins file.

#include <stdio.h>

#include "py/obj.h"
#include "py/mphal.h"
#include "pin.h"

#if 0
#define AF(af_idx, af_fn, af_unit, af_type, af_ptr) \
{ \
    { &pin_af_type }, \
    .name = MP_QSTR_AF ## af_idx ## _ ## af_fn ## af_unit, \
    .idx = (af_idx), \
    .fn = AF_FN_ ## af_fn, \
    .unit = (af_unit), \
    .type = AF_PIN_TYPE_ ## af_fn ## _ ## af_type, \
    .af_fn = (af_ptr) \
}
#endif

#define PIN_AD(p_name, p_id, ad_bit, ad_channel) \
{ \
    { &pin_type }, \
    .name = MP_QSTR_ ## p_name, \
    .id = p_id, \
    .bit = ad_bit, \
    .channel = ad_channel \
}

#define PIN(p_name, p_id, p_ad) \
{ \
    { &pin_type }, \
    .name = MP_QSTR_ ## p_name, \
    .id = p_id, \
    .ad = p_ad, \
}
