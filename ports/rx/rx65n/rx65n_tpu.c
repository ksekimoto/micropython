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
#include "common.h"
#include "rx65n_tpu.h"

typedef struct st_tpu0* tpu_reg_t;

static tpu_reg_t TPU_REG[] = {
    (tpu_reg_t)0x88108,
    (tpu_reg_t)0x88108,
    (tpu_reg_t)0x8810A,
    (tpu_reg_t)0x8810A,
    (tpu_reg_t)0x8810C,
    (tpu_reg_t)0x8810C,
    (tpu_reg_t)0x88178,
    (tpu_reg_t)0x88178,
    (tpu_reg_t)0x8817A,
    (tpu_reg_t)0x8817A,
    (tpu_reg_t)0x8817C,
    (tpu_reg_t)0x8817C
};

#define TPU_IPR_VEC(tpu_pin)    (126 + (uint32_t)tpu_pin)
#define TPU_IER_VEC(tpu_pin)    ((126 + (uint32_t)tpu_pin) /8)

static bool rx_tpu_channel_init_flag[TPU_CHANNEL_SIZE] = {
    false, false, false, false, false, false,
    false, false, false, false, false, false
};

static float rx_tpu_channel_freq[TPU_CHANNEL_SIZE] = {
    TPU_DEFAULT_FREQ,
    TPU_DEFAULT_FREQ,
    TPU_DEFAULT_FREQ,
    TPU_DEFAULT_FREQ,
    TPU_DEFAULT_FREQ,
    TPU_DEFAULT_FREQ,
    TPU_DEFAULT_FREQ,
    TPU_DEFAULT_FREQ,
    TPU_DEFAULT_FREQ,
    TPU_DEFAULT_FREQ,
    TPU_DEFAULT_FREQ,
    TPU_DEFAULT_FREQ };

static float rx_tpu_channel_duty[TPU_CHANNEL_SIZE] = {
    TPU_DEFAULT_DUTY,
    TPU_DEFAULT_DUTY,
    TPU_DEFAULT_DUTY,
    TPU_DEFAULT_DUTY,
    TPU_DEFAULT_DUTY,
    TPU_DEFAULT_DUTY,
    TPU_DEFAULT_DUTY,
    TPU_DEFAULT_DUTY,
    TPU_DEFAULT_DUTY,
    TPU_DEFAULT_DUTY,
    TPU_DEFAULT_DUTY,
    TPU_DEFAULT_DUTY};

static uint8_t tpu_pin_channel[] = {
    TIOCA0, 0,
    TIOCB0, 0,
    TIOCC0, 0,
    TIOCD0, 0,
    TIOCA1, 1,
    TIOCB1, 1,
    TIOCA2, 2,
    TIOCB2, 2,
    TIOCA3, 3,
    TIOCB3, 3,
    TIOCC3, 3,
    TIOCD3, 3,
    TIOCA4, 4,
    TIOCB4, 4,
    TIOCA5, 5,
    TIOCB5, 5,
    TIOCA6, 6,
    TIOCB6, 6,
    TIOCC6, 6,
    TIOCD6, 6,
    TIOCA7, 7,
    TIOCB7, 7,
    TIOCA8, 8,
    TIOCB8, 8,
    TIOCA9, 9,
    TIOCB9, 9,
    TIOCC9, 9,
    TIOCD9, 9,
    TIOCA10, 10,
    TIOCB10, 10,
    TIOCA11, 11,
    TIOCB11, 11,
    TPU_END, 0xff
};

static uint8_t pin_tpu_af3[] = {
    P13, TIOCA5,
    P14, TIOCB5,
    P15, TIOCB2,
    P16, TIOCB1,
    P17, TIOCB0,
    P20, TIOCB3,
    P21, TIOCA3,
    P22, TIOCC3,
    P23, TIOCD3,
    P24, TIOCB4,
    P25, TIOCA4,
    P56, TIOCA1,
    P86, TIOCA0,
    P87, TIOCA2,
    PA0, TIOCA0,
    PA1, TIOCB0,
    PA3, TIOCD0,
    PA4, TIOCA1,
    PA5, TIOCB1,
    PA6, TIOCA2,
    PA7, TIOCB2,
    PB0, TIOCA3,
    PB1, TIOCB3,
    PB2, TIOCC3,
    PB3, TIOCD3,
    PB4, TIOCA4,
    PB5, TIOCB4,
    PB6, TIOCA5,
    PB7, TIOCB5,
    PC4, TIOCC6,
    PC5, TIOCD6,
    PC6, TIOCA6,
    PC7, TIOCB6,
    PD0, TIOCA7,
    PD1, TIOCB7,
    PD2, TIOCA8,
    PD3, TIOCB8,
    PE0, TIOCC9,
    PE1, TIOCD9,
    PE2, TIOCA9,
    PE3, TIOCB9,
    PE4, TIOCA10,
    PE5, TIOCB10,
    PE6, TIOCA11,
    PE7, TIOCB11,
    PIN_END, TPU_END
};

/*
 * tpu_pin: enum TPU_CHANNEL_PIN
 */
void rx_tpu_int_ipr(uint8_t tpu_pin, uint8_t priority) {
    int idx = (int)TPU_IPR_VEC(tpu_pin);
    //IPR(TPU5, TGRIA) = priority;
    ICU.IPR[idx].BYTE = priority;
}

/*
 * tpu_pin: enum TPU_CHANNEL_PIN
 */
void rx_tpu_int_ier(uint8_t tpu_pin, int flag) {
    int idx = (int)TPU_IER_VEC(tpu_pin);
    int bit = (6 + (int)tpu_pin) & 7;
    uint8_t mask = (1 << bit);
    ICU.IER[idx].BYTE = (ICU.IER[idx].BYTE & ~mask) | (flag << bit);
}

void rx_tpu_channel_int_enable(uint8_t channel, uint8_t bit) {
    //tpu_reg_t tpu_reg = TPU_REG[channel];
    //tpu_reg->TIER.BYTE |= (1 << bit);
    uint8_t mask = 1 << bit;
    switch (channel) {
    case 0:
        TPU0.TIER.BYTE |= mask;
        break;
    case 1:
        TPU1.TIER.BYTE |= mask;
        break;
    case 2:
        TPU2.TIER.BYTE |= mask;
        break;
    case 3:
        TPU3.TIER.BYTE |= mask;
        break;
    case 4:
        TPU4.TIER.BYTE |= mask;
        break;
    case 5:
        TPU5.TIER.BYTE |= mask;
        break;
#if defined(RX63N)
    case 6:
        TPU6.TIER.BYTE |= mask;
        break;
    case 7:
        TPU7.TIER.BYTE |= mask;
        break;
    case 8:
        TPU8.TIER.BYTE |= mask;
        break;
    case 9:
        TPU9.TIER.BYTE |= mask;
        break;
    case 10:
        TPU10.TIER.BYTE |= mask;
        break;
    case 11:
        TPU11.TIER.BYTE |= mask;
        break;
#endif
    }
}

void rx_tpu_channel_int_disable(uint8_t channel, uint8_t bit) {
    //tpu_reg_t tpu_reg = TPU_REG[channel];
    //tpu_reg->TIER.BYTE &= ~(1 << bit);
    uint8_t mask = 1 << bit;
    switch (channel) {
    case 0:
        TPU0.TIER.BYTE &= ~mask;
        break;
    case 1:
        TPU1.TIER.BYTE &= ~mask;
        break;
    case 2:
        TPU2.TIER.BYTE &= ~mask;
        break;
    case 3:
        TPU3.TIER.BYTE &= ~mask;
        break;
    case 4:
        TPU4.TIER.BYTE &= ~mask;
        break;
    case 5:
        TPU5.TIER.BYTE &= ~mask;
        break;
#if defined(RX63N)
    case 6:
        TPU6.TIER.BYTE &= ~mask;
        break;
    case 7:
        TPU7.TIER.BYTE &= ~mask;
        break;
    case 8:
        TPU8.TIER.BYTE &= ~mask;
        break;
    case 9:
        TPU9.TIER.BYTE &= ~mask;
        break;
    case 10:
        TPU10.TIER.BYTE &= ~mask;
        break;
    case 11:
        TPU11.TIER.BYTE &= ~mask;
        break;
#endif
    }
}

uint8_t rx_tpu_get_tpu_pin(uint8_t pin_idx) {
    int i = 0;
    uint8_t tpu_pin = TPU_END;
    while (true) {
        if (pin_tpu_af3[i] == PIN_END) {
            break;
        }
        if (pin_tpu_af3[i] == pin_idx) {
            tpu_pin = pin_tpu_af3[i+1];
            break;
        }
        i += 2;
    }
    return tpu_pin;
}

uint8_t rx_tpu_get_tpu_channel(uint8_t pin_idx) {
    int i = 0;
    uint8_t tpu_channel = 0xff;
    uint8_t tpu_pin = rx_tpu_get_tpu_pin(pin_idx);
    if (tpu_pin == TPU_END) {
        return tpu_channel;
    }
    while (true) {
        if (tpu_pin_channel[i] == TPU_END) {
            break;
        }
        if (tpu_pin_channel[i] == tpu_pin) {
            tpu_channel = tpu_pin_channel[i+1];
            break;
        }
        i += 2;
    }
    return tpu_channel;
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
 * freq = 1(1/s) = 1000000(us) -> 187500
 * freq = 10(1/s) = 100000(us) -> 18750 <--
 * freq = 100(1/s) = 10000(us) -> 1875
 * freq = 1000(1/s) = 1000(us) -> 187.5
 * freq = 10000(1/s) = 100(us) -> 18.75
 * freq = 100000(1/s) = 10(us) -> 1.875
 * freq = 1000000(1/s) = 1(us) -> 0.1875
 */
uint32_t rx_tpu_get_clock_dev(int channel, float freq) {
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

void rx_tpu_set_clock(int channel, uint32_t clkdev) {
    switch (channel) {
    case 0:
        switch (clkdev) {
        case 1:
            TPU0.TCR.BIT.TPSC = 0x0;
            break;
        case 4:
            TPU0.TCR.BIT.TPSC = 0x1;
            break;
        case 16:
            TPU0.TCR.BIT.TPSC = 0x2;
            break;
        case 64:
            TPU0.TCR.BIT.TPSC = 0x3;
            break;
        default:
            break;
        }
        break;
#if defined(RX63N)
    case 6:
        switch (clkdev) {
        case 1:
            TPU6.TCR.BIT.TPSC = 0x0;
            break;
        case 4:
            TPU6.TCR.BIT.TPSC = 0x1;
            break;
        case 16:
            TPU6.TCR.BIT.TPSC = 0x2;
            break;
        case 64:
            TPU6.TCR.BIT.TPSC = 0x3;
            break;
        default:
            break;
        }
        break;
#endif
    case 1:
        switch (clkdev) {
        case 1:
            TPU1.TCR.BIT.TPSC = 0x0;
            break;
        case 4:
            TPU1.TCR.BIT.TPSC = 0x1;
            break;
        case 16:
            TPU1.TCR.BIT.TPSC = 0x2;
            break;
        case 64:
            TPU1.TCR.BIT.TPSC = 0x3;
            break;
        default:
            break;
        }
        break;
#if defined(RX63N)
    case 7:
        switch (clkdev) {
        case 1:
            TPU7.TCR.BIT.TPSC = 0x0;
            break;
        case 4:
            TPU7.TCR.BIT.TPSC = 0x1;
            break;
        case 16:
            TPU7.TCR.BIT.TPSC = 0x2;
            break;
        case 64:
            TPU7.TCR.BIT.TPSC = 0x3;
            break;
        default:
            break;
        }
        break;
#endif
    case 2:
        switch (clkdev) {
        case 1:
            TPU2.TCR.BIT.TPSC = 0x0;
            break;
        case 4:
            TPU2.TCR.BIT.TPSC = 0x1;
            break;
        case 16:
            TPU2.TCR.BIT.TPSC = 0x2;
            break;
        case 64:
            TPU2.TCR.BIT.TPSC = 0x3;
            break;
        default:
            break;
        }
        break;
#if defined(RX63N)
    case 8:
        switch (clkdev) {
        case 1:
            TPU8.TCR.BIT.TPSC = 0x0;
            break;
        case 4:
            TPU8.TCR.BIT.TPSC = 0x1;
            break;
        case 16:
            TPU8.TCR.BIT.TPSC = 0x2;
            break;
        case 64:
            TPU8.TCR.BIT.TPSC = 0x3;
            break;
        default:
            break;
        }
        break;
#endif
    case 3:
        switch (clkdev) {
        case 1:
            TPU3.TCR.BIT.TPSC = 0x0;
            break;
        case 4:
            TPU3.TCR.BIT.TPSC = 0x1;
            break;
        case 16:
            TPU3.TCR.BIT.TPSC = 0x2;
            break;
        case 64:
            TPU3.TCR.BIT.TPSC = 0x3;
            break;
        case 256:
            TPU3.TCR.BIT.TPSC = 0x6;
            break;
        case 1024:
            TPU3.TCR.BIT.TPSC = 0x5;
            break;
        case 4096:
            TPU3.TCR.BIT.TPSC = 0x7;
            break;
        default:
            break;
        }
        break;
#if defined(RX63N)
    case 9:
        switch (clkdev) {
        case 1:
            TPU9.TCR.BIT.TPSC = 0x0;
            break;
        case 4:
            TPU9.TCR.BIT.TPSC = 0x1;
            break;
        case 16:
            TPU9.TCR.BIT.TPSC = 0x2;
            break;
        case 64:
            TPU9.TCR.BIT.TPSC = 0x3;
            break;
        case 256:
            TPU9.TCR.BIT.TPSC = 0x6;
            break;
        case 1024:
            TPU9.TCR.BIT.TPSC = 0x5;
            break;
        case 4096:
            TPU9.TCR.BIT.TPSC = 0x7;
            break;
        default:
            break;
        }
        break;
#endif
    case 4:
        switch (clkdev) {
        case 1:
            TPU4.TCR.BIT.TPSC = 0x0;
            break;
        case 4:
            TPU4.TCR.BIT.TPSC = 0x1;
            break;
        case 16:
            TPU4.TCR.BIT.TPSC = 0x2;
            break;
        case 64:
            TPU4.TCR.BIT.TPSC = 0x3;
            break;
        case 1024:
            TPU4.TCR.BIT.TPSC = 0x6;
            break;
        }
        break;
#if defined(RX63N)
    case 10:
        switch (clkdev) {
        case 1:
            TPU10.TCR.BIT.TPSC = 0x0;
            break;
        case 4:
            TPU10.TCR.BIT.TPSC = 0x1;
            break;
        case 16:
            TPU10.TCR.BIT.TPSC = 0x2;
            break;
        case 64:
            TPU10.TCR.BIT.TPSC = 0x3;
            break;
        case 1024:
            TPU10.TCR.BIT.TPSC = 0x6;
            break;
        }
        break;
#endif
    case 5:
        switch (clkdev) {
        case 1:
            TPU5.TCR.BIT.TPSC = 0x0;
            break;
        case 4:
            TPU5.TCR.BIT.TPSC = 0x1;
            break;
        case 16:
            TPU5.TCR.BIT.TPSC = 0x2;
            break;
        case 64:
            TPU5.TCR.BIT.TPSC = 0x3;
            break;
        case 256:
            TPU5.TCR.BIT.TPSC = 0x6;
            break;
        }
        break;
#if defined(RX63N)
    case 11:
        switch (clkdev) {
        case 1:
            TPU11.TCR.BIT.TPSC = 0x0;
            break;
        case 4:
            TPU11.TCR.BIT.TPSC = 0x1;
            break;
        case 16:
            TPU11.TCR.BIT.TPSC = 0x2;
            break;
        case 64:
            TPU11.TCR.BIT.TPSC = 0x3;
            break;
        case 256:
            TPU11.TCR.BIT.TPSC = 0x6;
            break;
        }
        break;
#endif
    }
}

void rx_tpu_channel_enable(int channel) {
    switch (channel) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        TPUA.TSTR.BYTE |= (1 << channel);       // start counter
        break;
#if defined(RX63N)
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
        TPUB.TSTR.BYTE |= (1 << (channel-6));   // start counter
        break;
#endif
    default:
        break;
    }
}

void rx_tpu_channel_disable(int channel) {
    switch (channel) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        TPUA.TSTR.BYTE &= ~(1 << channel);      // stop counter
        break;
#if defined(RX63N)
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
        TPUB.TSTR.BYTE &= ~(1 << (channel-6));  // stop counter
        break;
#endif
    default:
        break;
    }
}

void rx_tpu_channel_deinit(int channel) {
    rx_tpu_channel_disable(channel);
}

/*
 * Use PWM mode2
 *  one of pin as master of clearing counter
 *                           PINz = TIOCB5 and TIOCB11
 *                           v
 *      +--------+           +--------+
 *      |        |           |        |
 *   ---+        +-----------+        +---
 *      0        TGRx        TGRz
 *               CHx         Chz
 *                           |
 *      +----------+         +----------+
 *      |          |         |          |
 *   ---+          +---------+          +---
 *      0          TGRy      TGRz
 *                 CHy       Chz
 */
void rx_tpu_channel_init(int channel) {
    rx_tpu_channel_disable(channel);
    switch (channel) {
    case 0:
        TPU0.TCR.BIT.CCLR = 0x7;    // compare match clear by other counter
        TPU0.TMDR.BIT.MD = 0x3;     // PWM mode 2
        TPU0.TIORH.BIT.IOA = 0x5;   // compare match high - low
        TPU0.TIORH.BIT.IOB = 0x5;   // compare match high - low
        TPU0.TIORL.BIT.IOC = 0x5;   // compare match high - low
        TPU0.TIORL.BIT.IOD = 0x5;   // compare match high - low
        break;
    case 1:
        TPU1.TCR.BIT.CCLR = 0x7;    // compare match clear by other counter
        TPU1.TMDR.BIT.MD = 0x3;     // PWM mode 2
        TPU1.TIOR.BIT.IOA = 0x5;    // compare match high - low
        TPU1.TIOR.BIT.IOB = 0x5;    // compare match high - low
        break;
    case 2:
        TPU2.TCR.BIT.CCLR = 0x7;    // compare match clear by other counter
        TPU2.TMDR.BIT.MD = 0x3;     // PWM mode 2
        TPU2.TIOR.BIT.IOA = 0x5;    // compare match high - low
        TPU2.TIOR.BIT.IOB = 0x5;    // compare match high - low
        break;
    case 3:
        TPU3.TCR.BIT.CCLR = 0x7;    // compare match clear by other counter
        TPU3.TMDR.BIT.MD = 0x3;     // PWM mode 2
        TPU3.TIORH.BIT.IOA = 0x5;   // compare match high - low
        TPU3.TIORH.BIT.IOB = 0x5;   // compare match high - low
        TPU3.TIORL.BIT.IOC = 0x5;   // compare match high - low
        TPU3.TIORL.BIT.IOD = 0x5;   // compare match high - low
        break;
    case 4:
        TPU4.TCR.BIT.CCLR = 0x7;    // compare match clear by other counter
        TPU4.TMDR.BIT.MD = 0x3;     // PWM mode 2
        TPU4.TIOR.BIT.IOA = 0x5 ;   // compare match high - low
        TPU4.TIOR.BIT.IOB = 0x5;    // compare match high - low
        break;
    case 5:
        TPU5.TCR.BIT.CCLR = 0x2;    // compare match clear by TGRB
        TPU5.TMDR.BIT.MD = 0x3;     // PWM mode 2
        TPU5.TIOR.BIT.IOA = 0x5 ;   // compare match high - low
        //TPU5.TIOR.BIT.IOB = 0x6;    // compare match high - high
        break;
    default:
        break;
    }
}

void rx_tpu_set_channel_params(uint8_t pin_idx, int channel, float freq, float duty) {
    uint32_t period_ticks;
    uint32_t duration_ticks;
    uint8_t tpu_pin = rx_tpu_get_tpu_pin(pin_idx);
    uint32_t clkdev = rx_tpu_get_clock_dev(channel, freq);
    rx_tpu_set_clock(channel, clkdev);
    float ticks = ((float)PCLK)/(freq * ((float)clkdev));
    period_ticks = (uint32_t)ticks;
    duration_ticks = (uint32_t)(ticks * duty);
    if ((channel >= 0) && (channel <= 5)) {
        TPU5.TGRB = period_ticks;
        TPU5.TCR.BIT.CCLR = 0x2;    // compare match clear by TGRB
        TPU5.TMDR.BIT.MD = 0x3;     // PWM mode 2
        TPU5.TIOR.BIT.IOB = 0x6;    // compare match high - high
        TPU0.TCNT = 0;
        TPU1.TCNT = 0;
        TPU2.TCNT = 0;
        TPU3.TCNT = 0;
        TPU4.TCNT = 0;
        TPU5.TCNT = 0;
    } else {
#if defined(RX63N)
        TPU11.TGRB = period_ticks;
        TPU11.TCR.BIT.CCLR = 0x2;   // compare match clear by TGRB
        TPU11.TMDR.BIT.MD = 0x3;    // PWM mode 2
        TPU11.TIOR.BIT.IOB = 0x6;   // compare match high - high
        TPU6.TCNT = 0;
        TPU7.TCNT = 0;
        TPU8.TCNT = 0;
        TPU9.TCNT = 0;
        TPU10.TCNT = 0;
        TPU11.TCNT = 0;
#endif
    }
    switch (channel)
    {
    case 0:
        if (tpu_pin == TIOCA0) {
            TPU0.TGRA = duration_ticks;
        } else if (tpu_pin == TIOCB0) {
            TPU0.TGRB = duration_ticks;
        } else if (tpu_pin == TIOCC0) {
            TPU0.TGRC = duration_ticks;
        } else if (tpu_pin == TIOCD0) {
            TPU0.TGRD = duration_ticks;
        }
        break;
    case 1:
        if (tpu_pin == TIOCA1) {
            TPU1.TGRA = duration_ticks;
        } else if (tpu_pin == TIOCB1) {
            TPU1.TGRB = duration_ticks;
        }
        break;
    case 2:
        if (tpu_pin == TIOCA2) {
            TPU2.TGRA = duration_ticks;
        } else if (tpu_pin == TIOCB2) {
            TPU2.TGRB = duration_ticks;
        }
        break;
    case 3:
        if (tpu_pin == TIOCA3) {
            TPU3.TGRA = duration_ticks;
        } else if (tpu_pin == TIOCB3) {
            TPU3.TGRB = duration_ticks;
        } else if (tpu_pin == TIOCC3) {
            TPU3.TGRC = duration_ticks;
        } else if (tpu_pin == TIOCD3) {
            TPU3.TGRD = duration_ticks;
        }
        break;
    case 4:
        if (tpu_pin == TIOCA4) {
            TPU4.TGRA = duration_ticks;
        } else if (tpu_pin == TIOCB4) {
            TPU4.TGRB = duration_ticks;
        }
        break;
    case 5:
        if (tpu_pin == TIOCA5) {
            TPU5.TGRA = duration_ticks;
        }
        break;
#if defined(RX63N)
    case 6:
        if (tpu_pin == TIOCA6) {
            TPU6.TGRA = duration_ticks;
        } else if (tpu_pin == TIOCB6) {
            TPU6.TGRB = duration_ticks;
        } else if (tpu_pin == TIOCC6) {
            TPU6.TGRC = duration_ticks;
        } else if (tpu_pin == TIOCD6) {
            TPU6.TGRD = duration_ticks;
        }
        break;
    case 7:
        if (tpu_pin == TIOCA7) {
            TPU7.TGRA = duration_ticks;
        } else if (tpu_pin == TIOCB7) {
            TPU7.TGRB = duration_ticks;
        }
        break;
    case 8:
        if (tpu_pin == TIOCA8) {
            TPU8.TGRA = duration_ticks;
        } else if (tpu_pin == TIOCB8) {
            TPU8.TGRB = duration_ticks;
        }
        break;
    case 9:
        if (tpu_pin == TIOCA9) {
            TPU9.TGRA = duration_ticks;
        } else if (tpu_pin == TIOCB9) {
            TPU9.TGRB = duration_ticks;
        } else if (tpu_pin == TIOCC9) {
            TPU9.TGRC = duration_ticks;
        } else if (tpu_pin == TIOCD9) {
            TPU9.TGRD = duration_ticks;
        }
        break;
    case 10:
        if (tpu_pin == TIOCA10) {
            TPU10.TGRA = duration_ticks;
        } else if (tpu_pin == TIOCB10) {
            TPU10.TGRB = duration_ticks;
        }
        break;
    case 11:
        if (tpu_pin == TIOCA11) {
            TPU11.TGRA = duration_ticks;
        }
        break;
#endif
    default:
        return false;
    }
#ifdef DEBUG_PWM
    debug_printf("Dt/D/P %04x/%06x/%06x\r\n", (UINT16)duration_ticks, duration, period);
#endif
    return true;
}

void rx_tpu_set_default_freq(float freq) {
    for (int i = 0; i < TPU_CHANNEL_SIZE; i++) {
        rx_tpu_channel_freq[i] = freq;
    }
}

void rx_tpu_set_freq(uint8_t pin_idx, float freq) {
    uint8_t channel = rx_tpu_get_tpu_channel(pin_idx);
    if (channel == TPU_END) {
        return;
    }
    rx_tpu_channel_freq[channel] = freq;
    rx_tpu_set_channel_params(pin_idx, channel, freq, rx_tpu_channel_duty[channel]);
}

float rx_tpu_get_freq(uint8_t pin_idx) {
    uint8_t channel = rx_tpu_get_tpu_channel(pin_idx);
    if (channel == TPU_END) {
        return TPU_DEFAULT_FREQ;
    }
    return rx_tpu_channel_freq[channel];
}

void rx_tpu_set_duty(uint8_t pin_idx, float duty) {
    uint8_t channel = rx_tpu_get_tpu_channel(pin_idx);
    if (channel == TPU_END) {
        return;
    }
    rx_tpu_channel_duty[channel] = duty;
    rx_tpu_set_channel_params(pin_idx, channel, rx_tpu_channel_freq[channel], duty);
}

float rx_tpu_get_duty(uint8_t pin_idx) {
    uint8_t channel = rx_tpu_get_tpu_channel(pin_idx);
    if (channel == TPU_END) {
        return TPU_DEFAULT_DUTY;
    }
    return rx_tpu_channel_duty[channel];
}

void rx_tpu_start(uint8_t pin_idx) {
    uint8_t channel = rx_tpu_get_tpu_channel(pin_idx);
    if (channel == TPU_END) {
        return;
    }
    rx_tpu_channel_enable(channel);
}

void rx_tpu_stop(uint8_t pin_idx) {
    uint8_t channel = rx_tpu_get_tpu_channel(pin_idx);
    if (channel == TPU_END) {
        return;
    }
    rx_tpu_channel_disable(channel);
}

static void rx_tpu_set_pin(uint8_t pin_idx)
{
    uint8_t port = GPIO_PORT(pin_idx);
    uint8_t mask = GPIO_MASK(pin_idx);
    MPC.PWPR.BIT.B0WI = 0;  /* Enable write to PFSWE */
    MPC.PWPR.BIT.PFSWE = 1; /* Enable write to PFS */
    _PMR(port) &= ~mask;
    _PDR(port) |= mask;
    _PODR(port) |= mask;
    _PXXPFS(port, pin_idx & 7) = 0x03;
    _PMR(port) |= mask;
    MPC.PWPR.BYTE = 0x80;   /* Disable write to PFSWE and PFS*/
}

static void rx_tpu_reset_pin(uint8_t pin_idx)
{
    uint8_t port = GPIO_PORT(pin_idx);
    uint8_t mask = GPIO_MASK(pin_idx);
    MPC.PWPR.BIT.B0WI = 0;  /* Enable write to PFSWE */
    MPC.PWPR.BIT.PFSWE = 1; /* Enable write to PFS */
    _PXXPFS(port, pin_idx & 7) = 0x00;
    _PMR(port) &= ~mask;
    MPC.PWPR.BYTE = 0x80;   /* Disable write to PFSWE and PFS*/
}

void rx_tpu_pin_init(uint8_t pin_idx) {
    uint8_t channel = rx_tpu_get_tpu_channel(pin_idx);
    if (channel == TPU_END) {
        return;
    }
    rx_tpu_set_pin(pin_idx);
    rx_tpu_channel_init(channel);
    rx_tpu_set_channel_params(pin_idx, channel,
        rx_tpu_channel_freq[channel],
        rx_tpu_channel_duty[channel]);
    //rx_tpu_channel_enable(channel);
}

void rx_tpu_pin_deinit(uint8_t pin_idx) {
    uint8_t channel = rx_tpu_get_tpu_channel(pin_idx);
    if (channel == TPU_END) {
        return;
    }
    rx_tpu_reset_pin(pin_idx);
    rx_tpu_channel_disable(channel);
}

static void rx_tpu_channel_init_clear_flag(void) {
    for (int i = 0; i < TPU_CHANNEL_SIZE; i++) {
        rx_tpu_channel_init_flag[i] = false;
    }
}

void rx_tpu_init() {
    SYSTEM.PRCR.WORD = 0xA502;
#if defined(RX63N)
    SYSTEM.MSTPCRA.BIT.MSTPA12 = 0; // TPU1
#endif
    SYSTEM.MSTPCRA.BIT.MSTPA13 = 0; // TPU0
    SYSTEM.PRCR.WORD = 0xA500;
    rx_tpu_channel_init_clear_flag();
}

void rx_tpu_deinit() {
    SYSTEM.PRCR.WORD = 0xA502;
#if defined(RX63N)
    SYSTEM.MSTPCRA.BIT.MSTPA12 = 1; // TPU1
#endif
    SYSTEM.MSTPCRA.BIT.MSTPA13 = 1; // TPU0
    SYSTEM.PRCR.WORD = 0xA500;
    rx_tpu_channel_init_clear_flag();
}

