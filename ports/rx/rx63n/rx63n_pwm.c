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

enum PWM_MTU_CHANNEL_PIN {
    MTIOC0A,
    MTIOC0B,
    MTIOC0C,
    MTIOC0D,
    MTIOC1A,
    MTIOC1B,
    MTIOC2A,
    MTIOC2B,
    MTIOC3A,
    MTIOC3B,
    MTIOC3C,
    MTIOC3D,
    MTIOC4A,
    MTIOC4B,
    MTIOC4C,
    MTIOC4D,
    MTU_END,
};

uint8_t mtu_pin_channel[] = {
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

uint8_t pin_mtu_af1[] = {
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

uint32_t rx_pwm_get_clock_dev(int scale, uint32_t period) {
    uint32_t clkdev;
    if ((scale == PWM_NANOSECONDS) && (period < 1000000)) {
        clkdev = 1;
    } else if ((scale == PWM_NANOSECONDS) && (period >= 1000000)) {
        clkdev = 64;
    } else if ((scale == PWM_MICROSECONDS) && (period <= 10000)) {
        clkdev = 64;
    } else {
        clkdev = 1024;
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

void rx_pwm_channel_init(int channel) {
    SYSTEM.MSTPCRA.BIT.MSTPA9 = 0;  // SYSTEM.MSTPCRA.BIT.MSTPA9
    switch (channel) {
    case 0:
        MTU.TSTR.BYTE &= 0xFE;      // stop counter
        MTU0.TCR.BIT.CCLR = 0x6;    // compare match clear by TGRD
        //MTU0.TCR.BIT.TPSC = 0x3;    // compare match clear PCLK/64
        MTU0.TMDR.BIT.MD = 0x3;     // PWM mode 2
        MTU0.TIORH.BIT.IOB = 0x5;   // compare match high
        //MTU0.TIORL.BIT.IOC = 0x6; // compare match high
        MTU0.TIORL.BIT.IOD = 0x6;   // compare match low
        break;
    case 1:
        MTU.TSTR.BYTE &= 0xFD;      // stop counter
        MTU1.TCR.BIT.CCLR = 0x6;    // compare match clear by TGRD
        //MTU1.TCR.BIT.TPSC = 0x3;    // compare match clear PCLK/64
        MTU1.TMDR.BIT.MD = 0x3;     // PWM mode 2
        //MTU1.TIORH.BIT.IOB = 0x5;   // compare match high
        //MTU1.TIORL.BIT.IOC = 0x6; // compare match high
        //MTU1.TIORL.BIT.IOD = 0x6;   // compare match low
        break;
    case 2:
        MTU.TSTR.BYTE &= 0xFB;      // stop counter
        MTU2.TCR.BIT.CCLR = 0x6;    // compare match clear by TGRD
        //MTU2.TCR.BIT.TPSC = 0x3;    // compare match clear PCLK/64
        MTU2.TMDR.BIT.MD = 0x3;     // PWM mode 2
        //MTU2.TIORH.BIT.IOB = 0x5;   // compare match high
        //MTU0.TIORL.BIT.IOC = 0x6; // compare match high
        //MTU2.TIORL.BIT.IOD = 0x6;   // compare match low
        break;
    case 3:
        MTU.TSTR.BYTE &= 0xBF;      // stop counter
        MTU3.TCR.BIT.CCLR = 0x1;    // compare match clear by TGRA
        //MTU3.TCR.BIT.TPSC = 0x3;    // compare match clear PCLK/64
        MTU3.TMDR.BIT.MD = 0x2;     // PWM mode 1
        MTU3.TIORH.BIT.IOA = 0x6;   // compare match high
        MTU3.TIORH.BIT.IOB = 0x5;   // compare match low
        //MTU3.TIORL.BIT.IOC = 0x6; // compare match high
        //MTU3.TIORL.BIT.IOD = 0x5; // compare match low
        break;
    case 4:
        MTU.TSTR.BYTE &= 0x7F;      // stop counter
        MTU4.TCR.BIT.CCLR = 0x1;    // compare match clear by TGRA
        //MTU4.TCR.BIT.TPSC = 0x3;    // compare match clear PCLK/64
        MTU4.TMDR.BIT.MD = 0x2;     // PWM mode 1
        MTU4.TIORH.BIT.IOA = 0x6;   // compare match high
        MTU4.TIORH.BIT.IOB = 0x5;   // compare match low
        //MTU4.TIORL.BIT.IOC = 0x6; // compare match high
        //MTU4.TIORL.BIT.IOD = 0x5; // compare match low
        MTU.TOER.BIT.OE4A = 1;
        //MTUA.TOER.BIT.OE4C = 1;
        break;
    default:
        break;
    }
}

void rx_pwm_channel_deinit(int channel) {
    switch (channel) {
    case 0:
        MTU.TSTR.BYTE &= 0xFE;      // stop counter
        break;
    case 1:
        MTU.TSTR.BYTE &= 0xFD;      // stop counter
        break;
    case 2:
        MTU.TSTR.BYTE &= 0xFB;      // stop counter
        break;
    case 3:
        MTU.TSTR.BYTE &= 0xBF;      // stop counter
        break;
    case 4:
        MTU.TSTR.BYTE &= 0x7F;      // stop counter
        break;
    default:
        break;
    }
}

void rx_pwm_apply_config(int channel, uint8_t pin, uint32_t period, uint32_t duration, int scale, bool invert) {
    uint32_t period_ticks;
    uint32_t duration_ticks;
    uint32_t clkdev = rx_pwm_get_clock_dev(scale, period);
    rx_pwm_set_clock(channel, clkdev);
    float ratio = (float)(PCLK/clkdev) / (float)scale;
    period_ticks = (uint32_t)((float)period *  ratio);
    duration_ticks = (uint32_t)((float)duration *  ratio);
    switch (channel)
    {
    case 0:
        MTU0.TGRD = period_ticks;
        MTU0.TGRB = duration_ticks;
        MTU0.TCNT = 0;
        break;
    case 1:
        MTU1.TGRB = duration_ticks;
        //MTU0.TGRC = duration_ticks;
        MTU1.TCNT = 0;
        break;
    case 2:
        MTU2.TGRB = duration_ticks;
        MTU0.TCNT = 0;
        break;
    case 3:
        MTU3.TGRA = period_ticks;
        MTU3.TGRB = duration_ticks;
        //MTU3.TGRC = period_ticks;
        MTU3.TCNT = 0;
        break;
    case 4:
        MTU4.TGRA = period_ticks;
        MTU4.TGRB = duration_ticks;
        //MTU4.TGRC = period_ticks;
        //MTU4.TGRD = duration_ticks;
        MTU4.TCNT = 0;
        break;
    default:
        return false;
    }
#ifdef DEBUG_PWM
    debug_printf("Dt/D/P %04x/%06x/%06x\r\n", (UINT16)duration_ticks, duration, period);
#endif
    return true;
}

void rx_pwm_enable(int channel) {
    //CPU_GPIO_DisablePin(g_RX63N_PWM_Driver.c_Pins[channel], RESISTOR_DISABLED, 0, GPIO_ALT_MODE_2);
    switch (channel) {
    case 0:
        MTU.TSTR.BYTE |= 0x01;     // stop counter
        break;
    case 1:
        MTU.TSTR.BYTE |= 0x02;     // stop counter
        break;
    case 2:
        MTU.TSTR.BYTE |= 0x04;     // stop counter
        break;
    case 3:
        MTU.TSTR.BYTE |= 0x40;     // stop counter
        break;
    case 4:
        MTU.TSTR.BYTE |= 0x80;     // stop counter
        break;
    default:
        break;
    }
}

void rx_pwm_disable(int channel) {
    switch (channel) {
    case 0:
        MTU.TSTR.BYTE &= 0xFE;    // start counter
        break;
    case 1:
        MTU.TSTR.BYTE &= 0xFD;    // start counter
        break;
    case 2:
        MTU.TSTR.BYTE &= 0xFB;    // start counter
        break;
    case 3:
        MTU.TSTR.BYTE &= 0xBF;    // start counter
        break;
    case 4:
        MTU.TSTR.BYTE &= 0x7F;    // start counter
        break;
    default:
        break;
    }
    //CPU_GPIO_DisablePin(g_RX63N_PWM_Driver.c_Pins[channel], RESISTOR_PULLDOWN, 0, GPIO_ALT_PRIMARY);
}

void rx_pwm_start(int channel, uint8_t pin) {
    rx_pwm_enable(channel);
}

void rx_pwm_stop(int channel, uint8_t pin) {
    rx_pwm_disable(channel);
}
