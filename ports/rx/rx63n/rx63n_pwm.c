/*
 * Copyright (c) 2018, Kentaro Sekimoto
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  -Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *  -Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdint.h>
#include "common.h"
#include "iodefine.h"
#include "rx63n_gpio.h"
#include "rx63n_pwm.h"

static bool rx_pwm_channel_init_flag[PWM_CHANNEL_SIZE] = {
    false, false, false, false, false };
static float rx_pwm_channel_freq[PWM_CHANNEL_SIZE] = {
    PWM_DEFAULT_FREQ,
    PWM_DEFAULT_FREQ,
    PWM_DEFAULT_FREQ,
    PWM_DEFAULT_FREQ,
    PWM_DEFAULT_FREQ };
static float rx_pwm_channel_duty[PWM_CHANNEL_SIZE] = {
    PWM_DEFAULT_DUTY,
    PWM_DEFAULT_DUTY,
    PWM_DEFAULT_DUTY,
    PWM_DEFAULT_DUTY,
    PWM_DEFAULT_DUTY};

enum PWM_MTU_CHANNEL_PIN {
    MTIOC0A,    /* channel 0-0: output */
    MTIOC0B,
    MTIOC0C,    /* channel 0-1: output */
    MTIOC0D,
    MTIOC1A,    /* channel 1-0: output */
    MTIOC1B,
    MTIOC2A,    /* channel 2-0: output */
    MTIOC2B,
    MTIOC3A,    /* channel 3-0: output */
    MTIOC3B,
    MTIOC3C,    /* channel 3-1: output */
    MTIOC3D,
    MTIOC4A,    /* channel 4-0: output */
    MTIOC4B,
    MTIOC4C,    /* channel 4-1: output */
    MTIOC4D,
    MTU_END,
};

static uint8_t mtu_pin_channel[] = {
    MTIOC0A, 0,
    MTIOC0B, 0,
    MTIOC0C, 0,
    MTIOC0D, 0,
    MTIOC1A, 1,
    MTIOC1B, 1,
    MTIOC2A, 2,
    MTIOC2B, 2,
    MTIOC3A, 3,
    MTIOC3B, 3,
    MTIOC3C, 3,
    MTIOC3D, 3,
    MTIOC4A, 4,
    MTIOC4B, 4,
    MTIOC4C, 4,
    MTIOC4D, 4,
    MTU_END, 0xff,
};

static uint8_t pin_mtu_af1[] = {
    P13, MTIOC0B,
    P14, MTIOC3A,
    P15, MTIOC0B,
    P16, MTIOC3C,
    P17, MTIOC3A,
    P20, MTIOC1A,
    P21, MTIOC1B,
    P22, MTIOC3B,
    P23, MTIOC3D,
    P24, MTIOC4A,
    P25, MTIOC4C,
    P26, MTIOC2A,
    P27, MTIOC2B,
    P30, MTIOC4B,
    P31, MTIOC4D,
    P32, MTIOC0C,
    P33, MTIOC0D,
    P34, MTIOC0A,
    P54, MTIOC4B,
    P55, MTIOC4D,
    P56, MTIOC3C,
    P80, MTIOC3B,
    P81, MTIOC3D,
    P82, MTIOC4A,
    P83, MTIOC4C,
    PA0, MTIOC4A,
    PA1, MTIOC0B,
    PA3, MTIOC0D,
    PB1, MTIOC0C,
    PB3, MTIOC0A,
    PB5, MTIOC2A,
    PB6, MTIOC3D,
    PB7, MTIOC3B,
    PC0, MTIOC3C,
    PC1, MTIOC3C,
    PC2, MTIOC4B,
    PC3, MTIOC4D,
    PC4, MTIOC3D,
    PC5, MTIOC3B,
    PC6, MTIOC3C,
    PC7, MTIOC3A,
    PD1, MTIOC4B,
    PD2, MTIOC4D,
    PE1, MTIOC4C,
    PE2, MTIOC4A,
    PE3, MTIOC4B,
    PE4, MTIOC4D,
    PE5, MTIOC4C,
    PJ3, MTIOC3C,
    PIN_END, MTU_END,
};

uint8_t rx_pwm_get_mtu_pin(uint8_t pin_idx) {
    int i = 0;
    uint8_t mtu_pin = MTU_END;
    while (true) {
        if (pin_mtu_af1[i] == PIN_END) {
            break;
        }
        if (pin_mtu_af1[i] == pin_idx) {
            mtu_pin = pin_mtu_af1[i+1];
            break;
        }
        i += 2;
    }
    return mtu_pin;
}

uint8_t rx_pwm_get_mtu_channel(uint8_t pin_idx) {
    int i = 0;
    uint8_t mtu_channel = 0xff;
    uint8_t mtu_pin = rx_pwm_get_mtu_pin(pin_idx);
    if (mtu_pin == MTU_END) {
        return mtu_channel;
    }
    while (true) {
        if (mtu_pin_channel[i] == MTU_END) {
            break;
        }
        if (mtu_pin_channel[i] == mtu_pin) {
            mtu_channel = mtu_pin_channel[i+1];
            break;
        }
        i += 2;
    }
    return mtu_channel;
}

/*
 * Prescale: 1                    ticks
 * freq = 1000(1/s) = 1000(us) -> 48000
 * freq = 10000(1/s) = 100(us) -> 4800 <--
 * freq = 100000(1/s) = 10(us) -> 480
 * freq = 1000000(1/s) = 1(us) -> 48
 * Prescale: 4
 * freq = 1000(1/s) = 1000(us) -> 12000 <--
 * freq = 10000(1/s) = 100(us) -> 1200
 * freq = 100000(1/s) = 10(us) -> 120
 * freq = 1000000(1/s) = 1(us) -> 12
 * Prescale: 16
 * freq = 100(1/s) = 10000(us) -> 30000
 * freq = 1000(1/s) = 1000(us) -> 3000 <--
 * freq = 10000(1/s) = 100(us) -> 300
 * freq = 100000(1/s) = 10(us) -> 30
 * freq = 1000000(1/s) = 1(us) -> 3
 * Prescale: 64
 * freq = 1(1/s) = 1000000(us) -> 750000
 * freq = 10(1/s) = 100000(us) -> 75000
 * freq = 100(1/s) = 10000(us) -> 7500 <--
 * freq = 1000(1/s) = 1000(us) -> 750
 * freq = 10000(1/s) = 100(us) -> 75
 * freq = 100000(1/s) = 10(us) -> 7.5
 * freq = 1000000(1/s) = 1(us) -> 0.75
 * Prescale: 256
 * freq = 1(1/s) = 1000000(us) -> 175000
 * freq = 10(1/s) = 100000(us) -> 17500 <--
 * freq = 100(1/s) = 10000(us) -> 1750
 * freq = 1000(1/s) = 1000(us) -> 175
 * freq = 10000(1/s) = 100(us) -> 17.5
 * freq = 100000(1/s) = 10(us) -> 1.75
 * freq = 1000000(1/s) = 1(us) -> 0.175
 */
uint32_t rx_pwm_get_clock_dev(int channel, float freq) {
    uint32_t clkdev;
    if (freq > 10000.0f) {
        clkdev = 1;
    } else if (freq > 1000.0f) {
        clkdev = 4;
    } else if (freq > 100.0f) {
        clkdev = 16;
    } else {
        if ((channel >= 0) && (channel <= 2)) {
            clkdev = 64;
        } else if (freq < 10.0f) {
            clkdev = 1024;
        } else {
            clkdev = 256;
        }
    }
    return clkdev;
}

void rx_pwm_set_clock(int channel, uint32_t clkdev) {
    switch (channel) {
    case 0:
        switch (clkdev) {
        case 1:
            MTU0.TCR.BIT.TPSC = 0x0;
            break;
        case 4:
            MTU0.TCR.BIT.TPSC = 0x1;
            break;
        case 16:
            MTU0.TCR.BIT.TPSC = 0x2;
            break;
        case 64:
            MTU0.TCR.BIT.TPSC = 0x3;
            break;
        default:
            break;
        }
        break;
    case 1:
        switch (clkdev) {
        case 1:
            MTU1.TCR.BIT.TPSC = 0x0;
            break;
        case 4:
            MTU1.TCR.BIT.TPSC = 0x1;
            break;
        case 16:
            MTU1.TCR.BIT.TPSC = 0x2;
            break;
        case 64:
            MTU1.TCR.BIT.TPSC = 0x3;
            break;
        default:
            break;
        }
        break;
    case 2:
        switch (clkdev) {
        case 1:
            MTU2.TCR.BIT.TPSC = 0x0;
            break;
        case 4:
            MTU2.TCR.BIT.TPSC = 0x1;
            break;
        case 16:
            MTU2.TCR.BIT.TPSC = 0x2;
            break;
        case 64:
            MTU2.TCR.BIT.TPSC = 0x3;
            break;
        default:
            break;
        }
        break;
    case 3:
        switch (clkdev) {
        case 1:
            MTU3.TCR.BIT.TPSC = 0x0;
            break;
        case 4:
            MTU3.TCR.BIT.TPSC = 0x1;
            break;
        case 16:
            MTU3.TCR.BIT.TPSC = 0x2;
            break;
        case 64:
            MTU3.TCR.BIT.TPSC = 0x3;
            break;
        case 256:
            MTU3.TCR.BIT.TPSC = 0x4;
            break;
        case 1024:
            MTU3.TCR.BIT.TPSC = 0x5;
            break;
        default:
            break;
        }
        break;
    case 4:
    default:
        switch (clkdev) {
        case 1:
            MTU4.TCR.BIT.TPSC = 0x0;
            break;
        case 4:
            MTU4.TCR.BIT.TPSC = 0x1;
            break;
        case 16:
            MTU4.TCR.BIT.TPSC = 0x2;
            break;
        case 64:
            MTU4.TCR.BIT.TPSC = 0x3;
            break;
        case 256:
            MTU4.TCR.BIT.TPSC = 0x4;
            break;
        case 1024:
            MTU4.TCR.BIT.TPSC = 0x5;
            break;
        }
        break;
    }
}


void rx_pwm_channel_enable(int channel) {
    switch (channel) {
    case 0:
        MTU.TSTR.BYTE |= 0x01;     // start counter
        break;
    case 1:
        MTU.TSTR.BYTE |= 0x02;     // start counter
        break;
    case 2:
        MTU.TSTR.BYTE |= 0x04;     // start counter
        break;
    case 3:
        MTU.TSTR.BYTE |= 0x40;     // start counter
        break;
    case 4:
        MTU.TSTR.BYTE |= 0x80;     // start counter
        break;
    default:
        break;
    }
}

void rx_pwm_channel_disable(int channel) {
    switch (channel) {
    case 0:
        MTU.TSTR.BYTE &= 0xFE;    // stop counter
        break;
    case 1:
        MTU.TSTR.BYTE &= 0xFD;    // stop counter
        break;
    case 2:
        MTU.TSTR.BYTE &= 0xFB;    // stop counter
        break;
    case 3:
        MTU.TSTR.BYTE &= 0xBF;    // stop counter
        break;
    case 4:
        MTU.TSTR.BYTE &= 0x7F;    // stop counter
        break;
    default:
        break;
    }
}

void rx_pwm_channel_deinit(int channel) {
    rx_pwm_channel_disable(channel);
}

void rx_pwm_channel_init(int channel) {
    rx_pwm_channel_disable(channel);
    switch (channel) {
    case 0:
        MTU0.TCR.BIT.CCLR = 0x6;    // compare match clear by TGRD
        MTU0.TMDR.BIT.MD = 0x2;     // PWM mode 1
        MTU0.TIORH.BIT.IOA = 0x5;   // compare match high - low
        MTU0.TIORH.BIT.IOB = 0x6;   // compare match high - high
        MTU0.TIORL.BIT.IOC = 0x5;   // compare match high - low
        MTU0.TIORL.BIT.IOD = 0x6;   // compare match high - high
        break;
    case 1:
        MTU1.TCR.BIT.CCLR = 0x6;    // compare match clear by TGRD
        MTU1.TMDR.BIT.MD = 0x2;     // PWM mode 1
        MTU1.TIOR.BIT.IOA = 0x5;    // compare match high - low
        MTU1.TIOR.BIT.IOB = 0x6;    // compare match high - high
        break;
    case 2:
        MTU2.TCR.BIT.CCLR = 0x6;    // compare match clear by TGRD
        MTU2.TMDR.BIT.MD = 0x2;     // PWM mode 1
        MTU2.TIOR.BIT.IOA = 0x5;    // compare match high - low
        MTU2.TIOR.BIT.IOB = 0x6;    // compare match high - high
        break;
    case 3:
        MTU3.TCR.BIT.CCLR = 0x6;    // compare match clear by TGRD
        MTU3.TMDR.BIT.MD = 0x2;     // PWM mode 1
        MTU3.TIORH.BIT.IOA = 0x5;   // compare match high - low
        MTU3.TIORH.BIT.IOB = 0x6;   // compare match high - high
        MTU3.TIORL.BIT.IOC = 0x5;   // compare match high - low
        MTU3.TIORL.BIT.IOD = 0x6;   // compare match high - high
        break;
    case 4:
        MTU4.TCR.BIT.CCLR = 0x6;    // compare match clear by TGRD
        MTU4.TMDR.BIT.MD = 0x2;     // PWM mode 1
        MTU4.TIORH.BIT.IOA = 0x5;   // compare match high - low
        MTU4.TIORH.BIT.IOB = 0x6;   // compare match high - high
        MTU4.TIORL.BIT.IOC = 0x5;   // compare match high - low
        MTU4.TIORL.BIT.IOD = 0x6;   // compare match high - high
        break;
    default:
        break;
    }
}

void rx_pwm_set_channel_params(uint8_t pin_idx, int channel, float freq, float duty) {
    uint32_t period_ticks;
    uint32_t duration_ticks;
    uint8_t mtu_pin = rx_pwm_get_mtu_pin(pin_idx);
    uint32_t clkdev = rx_pwm_get_clock_dev(channel, freq);
    rx_pwm_set_clock(channel, clkdev);
    float ticks = ((float)PCLK)/(freq * ((float)clkdev));
    period_ticks = (uint32_t)ticks;
    duration_ticks = (uint32_t)(ticks * duty);
    switch (channel)
    {
    case 0:
        if (mtu_pin == MTIOC0A) {
            MTU0.TGRA = duration_ticks;
            MTU0.TGRB = period_ticks;
            MTU0.TGRD = period_ticks;
            MTU0.TCNT = 0;
        } else if (mtu_pin == MTIOC0C) {
            MTU0.TGRC = duration_ticks;
            MTU0.TGRB = period_ticks;
            MTU0.TGRD = period_ticks;
            MTU0.TCNT = 0;
            MTU0.TCNT = 0;
        }
        break;
    case 1:
        if (mtu_pin == MTIOC1A) {
            MTU1.TGRA = duration_ticks;
            MTU1.TGRB = period_ticks;
            MTU1.TCNT = 0;
        }
        break;
    case 2:
        if (mtu_pin == MTIOC2A) {
            MTU2.TGRA = duration_ticks;
            MTU2.TGRB = period_ticks;
            MTU2.TCNT = 0;
        }
        break;
    case 3:
        if (mtu_pin == MTIOC3A) {
            MTU3.TGRA = duration_ticks;
            MTU3.TGRB = period_ticks;
            MTU3.TGRD = period_ticks;
            MTU3.TCNT = 0;
        } else if (mtu_pin == MTIOC3C) {
            MTU3.TGRC = duration_ticks;
            MTU3.TGRB = period_ticks;
            MTU3.TGRD = period_ticks;
            MTU3.TCNT = 0;
        }
        break;
    case 4:
        if (mtu_pin == MTIOC4A) {
            MTU4.TGRA = duration_ticks;
            MTU4.TGRB = period_ticks;
            MTU4.TGRD = period_ticks;
            MTU4.TCNT = 0;
            MTU.TOER.BIT.OE4A = 1;
        } else if (mtu_pin == MTIOC4C) {
            MTU4.TGRC = duration_ticks;
            MTU4.TGRB = period_ticks;
            MTU4.TGRD = period_ticks;
            MTU4.TCNT = 0;
            MTU.TOER.BIT.OE4C = 1;
        }
        break;
    default:
        return false;
    }
#ifdef DEBUG_PWM
    debug_printf("Dt/D/P %04x/%06x/%06x\r\n", (UINT16)duration_ticks, duration, period);
#endif
    return true;
}

void rx_pwm_set_freq(uint8_t pin_idx, float freq) {
    uint8_t channel = rx_pwm_get_mtu_channel(pin_idx);
    if (channel == MTU_END) {
        return;
    }
    rx_pwm_channel_freq[channel] = freq;
    rx_pwm_set_channel_params(pin_idx, channel, freq, rx_pwm_channel_duty[channel]);
}

float rx_pwm_get_freq(uint8_t pin_idx) {
    uint8_t channel = rx_pwm_get_mtu_channel(pin_idx);
    if (channel == MTU_END) {
        return PWM_DEFAULT_FREQ;
    }
    return rx_pwm_channel_freq[channel];
}

void rx_pwm_set_duty(uint8_t pin_idx, float duty) {
    uint8_t channel = rx_pwm_get_mtu_channel(pin_idx);
    if (channel == MTU_END) {
        return;
    }
    rx_pwm_channel_duty[channel] = duty;
    rx_pwm_set_channel_params(pin_idx, channel, rx_pwm_channel_freq[channel], duty);
}

float rx_pwm_get_duty(uint8_t pin_idx) {
    uint8_t channel = rx_pwm_get_mtu_channel(pin_idx);
    if (channel == MTU_END) {
        return PWM_DEFAULT_DUTY;
    }
    return rx_pwm_channel_duty[channel];
}

void rx_pwm_start(uint8_t pin_idx) {
    uint8_t channel = rx_pwm_get_mtu_channel(pin_idx);
    if (channel == MTU_END) {
        return;
    }
    rx_pwm_channel_enable(channel);
}

void rx_pwm_stop(uint8_t pin_idx) {
    uint8_t channel = rx_pwm_get_mtu_channel(pin_idx);
    if (channel == MTU_END) {
        return;
    }
    rx_pwm_channel_disable(channel);
}

static void rx_pwm_set_pin(uint8_t pin_idx)
{
    uint8_t port = GPIO_PORT(pin_idx);
    uint8_t mask = GPIO_MASK(pin_idx);
    MPC.PWPR.BIT.B0WI = 0;  /* Enable write to PFSWE */
    MPC.PWPR.BIT.PFSWE = 1; /* Enable write to PFS */
    _PMR(port) &= ~mask;
    _PDR(port) |= mask;
    _PODR(port) |= mask;
    _PXXPFS(port, pin_idx & 7) = 0x01;
    _PMR(port) |= mask;
    MPC.PWPR.BYTE = 0x80;   /* Disable write to PFSWE and PFS*/
}

static void rx_pwm_reset_pin(uint8_t pin_idx)
{
    uint8_t port = GPIO_PORT(pin_idx);
    uint8_t mask = GPIO_MASK(pin_idx);
    MPC.PWPR.BIT.B0WI = 0;  /* Enable write to PFSWE */
    MPC.PWPR.BIT.PFSWE = 1; /* Enable write to PFS */
    _PXXPFS(port, pin_idx & 7) = 0x00;
    _PMR(port) &= ~mask;
    MPC.PWPR.BYTE = 0x80;   /* Disable write to PFSWE and PFS*/
}

void rx_pwm_pin_init(uint8_t pin_idx) {
    uint8_t channel = rx_pwm_get_mtu_channel(pin_idx);
    if (channel == MTU_END) {
        return;
    }
    rx_pwm_set_pin(pin_idx);
    rx_pwm_channel_init(channel);
    rx_pwm_set_channel_params(pin_idx, channel,
        rx_pwm_channel_freq[channel],
        rx_pwm_channel_duty[channel]);
    rx_pwm_channel_enable(channel);
}

void rx_pwm_pin_deinit(uint8_t pin_idx) {
    uint8_t channel = rx_pwm_get_mtu_channel(pin_idx);
    if (channel == MTU_END) {
        return;
    }
    rx_pwm_reset_pin(pin_idx);
    rx_pwm_channel_disable(channel);
}

static void rx_pwm_channel_init_clear_flag(void) {
    for (int i = 0; i < PWM_CHANNEL_SIZE; i++) {
        rx_pwm_channel_init_flag[i] = false;
    }
}

void rx_pwm_init(void) {
    SYSTEM.PRCR.WORD = 0xA502;
    SYSTEM.MSTPCRA.BIT.MSTPA9 = 0;  // SYSTEM.MSTPCRA.BIT.MSTPA9
    SYSTEM.PRCR.WORD = 0xA500;
    rx_pwm_channel_init_clear_flag();
}

void rx_pwm_deinit(void) {
    SYSTEM.PRCR.WORD = 0xA502;
    SYSTEM.MSTPCRA.BIT.MSTPA9 = 1;  // SYSTEM.MSTPCRA.BIT.MSTPA9
    SYSTEM.PRCR.WORD = 0xA500;
    rx_pwm_channel_init_clear_flag();
}
