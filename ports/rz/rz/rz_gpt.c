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

#include "iodefine.h"
#include "rza2m_config.h"

#define GPT_CH_NUM  8

typedef struct _gpt_map_t {
    uint16_t pinw;
    uint8_t gpt_id;
    uint8_t af_no;

} gpt_map_t;

enum gpt_pin {
    GTIOC0A,
    GTIOC0B,
    GTIOC1A,
    GTIOC1B,
    GTIOC2A,
    GTIOC2B,
    GTIOC3A,
    GTIOC3A,
    GTIOC3B,
    GTIOC3B,
    GTIOC4A,
    GTIOC4A,
    GTIOC4B,
    GTIOC4B,
    GTIOC5A,
    GTIOC5B,
    GTIOC6A,
    GTIOC6A,
    GTIOC6A,
    GTIOC6B,
    GTIOC6B,
    GTIOC6B,
    GTIOC7A,
    GTIOC7A,
    GTIOC7B,
    GTIOC7B,
    GPT_NC,
};

const gpt_map_t GPT_MAP[] = {
    {PG2,  GTIOC0A, 5},
    {PG3,  GTIOC0B, 5},
    {PG4,  GTIOC1A, 5},
    {PG5,  GTIOC1B, 5},
    {PG6,  GTIOC2A, 5},
    {PG7,  GTIOC2B, 5},
    {P83,  GTIOC3A, 5},
    {P76,  GTIOC3A, 4},
    {P77,  GTIOC3B, 4},
    {P00,  GTIOC3B, 5},
    {PH0,  GTIOC4A, 3},
    {P01,  GTIOC4A, 5},
    {PH1,  GTIOC4B, 3},
    {P02,  GTIOC4B, 5},
    {P82,  GTIOC5A, 4},
    {P81,  GTIOC5B, 4},
    {PH3,  GTIOC6A, 3},
    {P03,  GTIOC6A, 5},
    {P20,  GTIOC6A, 2},
    {PH4,  GTIOC6B, 3},
    {P04,  GTIOC6B, 5},
    {P21,  GTIOC6B, 2},
    {P05,  GTIOC7A, 5},
    {P22,  GTIOC7A, 2},
    {P06,  GTIOC7B, 5},
    {P23,  GTIOC7B, 2},
    {PIN_END, GPT_NC, 0}
};
#define GPT_MAP_SIZE    (sizeof(GPT_MAP) / sizeof(gpt_map_t))

static const volatile struct st_gpt32e *GPT32E[] = {
    &GPT32E0, &GPT32E1, &GPT32E2, &GPT32E3,
    &GPT32E4, &GPT32E5, &GPT32E6, &GPT32E7
};

static float gpt_duty[GPT_CH_NUM] = { 0.0f };

uint8_t find_gpt_ch(uint32_t pin) {
    return 0;
}

uint8_t find_gpt_af_no(uint32 pin) {
    return 0;
}

void rz_gpt_pin_init(uint32_t pin) {
    volatile struct st_gpt32e *gpt_reg;
    uint8_t ch2 = find_get_ch(pin);
    if (ch2 == GPT_NC) {
        return;
    }
    gpt_reg = GPT32E[ch2 >> 1];
    CPG.STBCR3.BIT.MSTP30 = 0;
    uint8_t af_no = find_gpt_af_no(pin);
    _gpio_mode_af(pin, af_no);

    gpt_duty[ch2 >> 1] = 0.0f;

    /* Set operating mode : saw-wave PWM mode */
    gpt_reg->GTCR.BIT.MD = 0x0;
    gpt_reg->GTUDDTYC.BIT.UDF = 0x1;
    gpt_reg->GTUDDTYC.BIT.UD = 0x1;
    gpt_reg->GTUDDTYC.BIT.UDF = 0x0;
    // pwmout_period_us(obj, 1000);
    if ((ch2 & 1) == 0) {
        gpt_reg->GTIOR.BIT.GTIOA = 0x19;  /* Set GTIOC pin function */
        gpt_reg->GTBER.BIT.CCRA = 0x01;   /* Set buffer operation : Single buffer operation */
        gpt_reg->GTCCRA.LONG = 0;         /* Set compare match value */
        gpt_reg->GTIOR.BIT.OAE = 0x1;     /* Enable GTIOC pin output */
    } else {
        gpt_reg->GTIOR.BIT.GTIOB = 0x19;  /* Set GTIOC pin function */
        gpt_reg->GTBER.BIT.CCRB = 0x01;   /* Set buffer operation : Single buffer operation */
        gpt_reg->GTCCRB.LONG = 0;         /* Set compare match value */
        gpt_reg->GTIOR.BIT.OBE = 0x1;     /* Enable GTIOC pin output */
    }
    /* Start count operation */
    gpt_reg->GTCR.BIT.CST = 0x01;
}

void rz_gpt_pin_deinit(uint32_t pin) {
    _gpio_mode_af(pin, 0);
}

void rz_gpt_pin_write_duty(uint32_t pin, float value) {
    volatile struct st_gpt32e *gpt_reg;
    uint8_t ch2 = find_get_ch(pin);
    if (ch2 == GPT_NC) {
        return;
    }
    gpt_reg = GPT32E[ch2 >> 1];
    if (value < 0.0f) {
        value = 0.0f;
    } else if (value > 1.0f) {
        value = 1.0f;
    } else {
        // Do Nothing
    }
    gpt_duty[ch2 >> 1] = value;

    if ((ch2 & 1) == 0) {
        gpt_reg->GTCCRC.LONG = gpt_reg->GTPR.LONG * value; /* Set buffer value */
        gpt_reg->GTUDDTYC.BIT.OADTYF = 0x1;
        if (value == 0.0f) {
            gpt_reg->GTUDDTYC.BIT.OADTY = 0x2;
        } else if (value == 1.0f) {
            gpt_reg->GTUDDTYC.BIT.OADTY = 0x3;
        } else {
            gpt_reg->GTUDDTYC.BIT.OADTY = 0x0;
        }
        gpt_reg->GTUDDTYC.BIT.OADTYF = 0x0;
    } else {
        gpt_reg->GTCCRE.LONG = gpt_reg->GTPR.LONG * value; /* Set buffer value */
        gpt_reg->GTUDDTYC.BIT.OBDTYF = 0x1;
        if (value == 0.0f) {
            gpt_reg->GTUDDTYC.BIT.OBDTY = 0x2;
        } else if (value == 1.0f) {
            gpt_reg->GTUDDTYC.BIT.OBDTY = 0x3;
        } else {
            gpt_reg->GTUDDTYC.BIT.OBDTY = 0x0;
        }
        gpt_reg->GTUDDTYC.BIT.OBDTYF = 0x0;
    }
}

float rz_gpt_pin_read_duty(uint32_t pin) {
    float value;
    volatile struct st_gpt32e *gpt_reg;
    uint8_t ch2 = find_get_ch(pin);
    if (ch2 == GPT_NC) {
        return 0.0f;
    }
    gpt_reg = GPT32E[ch2 >> 1];
    if ((ch2 & 1) == 0) {
        value = (float)gpt_reg->GTCCRC.LONG / (float)gpt_reg->GTPR.LONG;
    } else {
        value = (float)gpt_reg->GTCCRE.LONG / (float)gpt_reg->GTPR.LONG;
    }
    return (value > 1.0f) ? (1.0f) : (value);
}

// Set the PWM period, keeping the duty cycle the same.
void rz_gpt_pin_period_us(uint32_t pin, int us) {
    uint32_t pclk_base;
    uint32_t wk_cycle;
    volatile struct st_gpt32e *gpt_reg;
    uint8_t ch2 = find_get_ch(pin);
    if (ch2 == GPT_NC) {
        return 0.0f;
    }
    gpt_reg = GPT32E[ch2 >> 1];
    pclk_base = (uint32_t)PCLK / 1000000;
    uint32_t us_max = 0xFFFFFFFF / pclk_base;
    if ((uint32_t)us > us_max) {
        us = us_max;
    } else if (us < 1) {
        us = 1;
    } else {
        // Do Nothing
    }
    wk_cycle = pclk_base * us;
    gpt_reg->GTCR.BIT.TPCS = 0x0;        /* Select count clock */
    gpt_reg->GTPR.LONG = wk_cycle - 1;   /* Set cycle */
    gpt_reg->GTCNT.LONG = 0;             /* Set initial value for counter */
    /* set duty again */
    pwmout_write(pin, gpt_duty[ch2 >> 1]);
}


void rz_gpt_pin_period_sec(uint32_t pin, float seconds) {
    rz_gpt_pin_period_us(pin, seconds * 1000000.0f);
}

void rz_gpt_pin_period_ms(uint32_t pin, int ms) {
    rz_gpt_pin_period_us(pin, ms * 1000);
}

void rz_gpt_pin_pulsewidth_us(uint32_t pin, int us) {
    volatile struct st_gpt32e *gpt_reg;
    uint8_t ch2 = find_get_ch(pin);
    if (ch2 == GPT_NC) {
        return 0.0f;
    }
    gpt_reg = GPT32E[ch2 >> 1];
    float value = 0;
    value = (float)(us * ((uint32_t)PCLK / 1000000)) / (float)(gpt_reg->GTPR.LONG + 1);
    rz_gpt_pin_write_duty(pin, value);
}

void rz_gpt_pin_pulsewidth_sec(uint32_t pin, float seconds) {
    rz_gpt_pin_pulsewidth_us(pin, seconds * 1000000.0f);
}

void rz_gpt_pin_pulsewidth_ms(uint32_t pin, int ms) {
    rz_gpt_pin_pulsewidth_us(pin, ms * 1000);
}
