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

#include <stdio.h>
#include <stdint.h>
#include "common.h"
#include "iodefine.h"
#include "rz_gpio.h"
#include "rz_mtu.h"

static bool rz_pwm_channel_init_flag[PWM_CHANNEL_SIZE] = {
    false, false, false, false, false
};
static float rz_pwm_channel_freq[PWM_CHANNEL_SIZE] = {
    PWM_DEFAULT_FREQ,
    PWM_DEFAULT_FREQ,
    PWM_DEFAULT_FREQ,
    PWM_DEFAULT_FREQ,
    PWM_DEFAULT_FREQ
};
static float rz_pwm_channel_duty[PWM_CHANNEL_SIZE] = {
    PWM_DEFAULT_DUTY,
    PWM_DEFAULT_DUTY,
    PWM_DEFAULT_DUTY,
    PWM_DEFAULT_DUTY,
    PWM_DEFAULT_DUTY
};

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
    MTIOC5U,
    MTIOC5V,
    MTIOC5W,
    MTIOC6A,    /* channel 6-0: output */
    MTIOC6B,
    MTIOC6C,    /* channel 6-1: output */
    MTIOC6D,
    MTIOC7A,    /* channel 7-0: output */
    MTIOC7B,
    MTIOC7C,    /* channel 7-1: output */
    MTIOC7D,
    MTIOC8A,    /* channel 7-0: output */
    MTIOC8B,
    MTIOC8C,    /* channel 7-1: output */
    MTIOC8D,
    MTU_END,
};

static const uint32_t mtu_pin_channel[] = {
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
    MTIOC5U, 5,
    MTIOC5V, 5,
    MTIOC5W, 5,
    MTIOC6A, 6,
    MTIOC6B, 6,
    MTIOC6C, 6,
    MTIOC6D, 6,
    MTIOC7A, 7,
    MTIOC7B, 7,
    MTIOC7C, 7,
    MTIOC7D, 7,
    MTIOC8A, 8,
    MTIOC8B, 8,
    MTIOC8C, 8,
    MTIOC8D, 8,
    MTU_END, 0xff,
};

static const mtu_map_t pin_mtu_af1[] = {
    {P00, MTIOC6B, 4},
    {P01, MTIOC6C, 4},
    {P02, MTIOC6D, 4},
    {P03, MTIOC7A, 4},
    {P04, MTIOC7B, 4},
    {P05, MTIOC7C, 4},
    {P06, MTIOC7D, 4},
    {P07, MTIOC7A, 4},
    {P11, MTIOC8A, 2},
    {P12, MTIOC8B, 2},
    {P13, MTIOC8C, 2},
    {P14, MTIOC8D, 2},
    {P40, MTIOC8A, 5},
    {P41, MTIOC8B, 5},
    {P42, MTIOC8C, 5},
    {P43, MTIOC8D, 5},
    {P61, MTIOC2A, 4},
    {P62, MTIOC2B, 4},
    {P80, MTIOC8D, 5},
    {P83, MTIOC6A, 4},
    {PA0, MTIOC8C, 5},
    {PA1, MTIOC8B, 5},
    {PA2, MTIOC8A, 5},
    {PA3, MTIOC0D, 5},
    {PA4, MTIOC0C, 5},
    {PA5, MTIOC0B, 5},
    {PA6, MTIOC0A, 5},
    {PE3, MTIOC0A, 4},
    {PE4, MTIOC0B, 4},
    {PE5, MTIOC0C, 4},
    {PE6, MTIOC0D, 4},
    {PF0, MTIOC7A, 4},
    {PF1, MTIOC7B, 4},
    {PF2, MTIOC7C, 4},
    {PF3, MTIOC7D, 4},
    {PF4, MTIOC6A, 4},
    {PF5, MTIOC6B, 4},
    {PF6, MTIOC6C, 4},
    {PG0, MTIOC3A, 4},
    {PG1, MTIOC3C, 4},
    {PG2, MTIOC3B, 4},
    {PG3, MTIOC3D, 4},
    {PG4, MTIOC4A, 4},
    {PG5, MTIOC4B, 4},
    {PG6, MTIOC4C, 4},
    {PG7, MTIOC4D, 4},
    {PH0, MTIOC1A, 4},
    {PH1, MTIOC1B, 4},
    {PH2, MTIOC6D, 4},
    {PH3, MTIOC2A, 4},
    {PH4, MTIOC2B, 4},
    {PH5, MTIOC5U, 4},
    {PH6, MTIOC5V, 4},
    {PJ5, MTIOC1A, 4},
    {PK0, MTIOC1B, 4},
    {PIN_END, MTIOC1B, 4},
};
#define MTU_MAP_SIZE   (sizeof(pin_mtu_af1) / sizeof(mtu_map_t))

uint8_t rz_pwm_get_mtu_pin(uint32_t pin) {
    int i = 0;
    uint8_t mtu_pin = MTU_END;
    while (true) {
        if (pin_mtu_af1[i].pinw == (uint16_t)PIN_END) {
            break;
        }
        if (pin_mtu_af1[i].pinw == (uint16_t)pin) {
            mtu_pin = pin_mtu_af1[i].mtu_id;
            break;
        }
        i++;
    }
    return mtu_pin;
}

uint8_t rz_pwm_get_af(uint32_t pin) {
    int i = 0;
    uint8_t af_no = 0;
    while (true) {
        if (pin_mtu_af1[i].pinw == (uint16_t)PIN_END) {
            break;
        }
        if (pin_mtu_af1[i].pinw == (uint16_t)pin) {
            af_no = pin_mtu_af1[i].af_no;
            break;
        }
        i++;
    }
    return af_no;
}

uint8_t rz_pwm_get_mtu_channel(uint32_t pin) {
    int i = 0;
    uint8_t mtu_channel = 0xff;
    uint8_t mtu_pin = rz_pwm_get_mtu_pin(pin);
    if (mtu_pin == MTU_END) {
        return mtu_channel;
    }
    while (true) {
        if (mtu_pin_channel[i] == MTU_END) {
            break;
        }
        if (mtu_pin_channel[i] == mtu_pin) {
            mtu_channel = mtu_pin_channel[i + 1];
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
uint32_t rz_pwm_get_clock_dev(int channel, float freq) {
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

void rz_pwm_set_clock(int channel, uint32_t clkdev) {
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


void rz_pwm_channel_enable(int channel) {
    switch (channel) {
        case 0:
            MTU.TSTRA.BYTE |= 0x01; // start counter
            break;
        case 1:
            MTU.TSTRA.BYTE |= 0x02; // start counter
            break;
        case 2:
            MTU.TSTRA.BYTE |= 0x04; // start counter
            break;
        case 3:
            MTU.TSTRA.BYTE |= 0x40; // start counter
            break;
        case 4:
            MTU.TSTRA.BYTE |= 0x80; // start counter
            break;
        default:
            break;
    }
}

void rz_pwm_channel_disable(int channel) {
    switch (channel) {
        case 0:
            MTU.TSTRA.BYTE &= 0xFE; // stop counter
            break;
        case 1:
            MTU.TSTRA.BYTE &= 0xFD; // stop counter
            break;
        case 2:
            MTU.TSTRA.BYTE &= 0xFB; // stop counter
            break;
        case 3:
            MTU.TSTRA.BYTE &= 0xBF; // stop counter
            break;
        case 4:
            MTU.TSTRA.BYTE &= 0x7F; // stop counter
            break;
        default:
            break;
    }
}

void rz_pwm_channel_deinit(int channel) {
    rz_pwm_channel_disable(channel);
}

void rz_pwm_channel_init(int channel) {
    rz_pwm_channel_disable(channel);
    switch (channel) {
        case 0:
            MTU0.TCR.BIT.CCLR = 0x6; // compare match clear by TGRD
            MTU0.TMDR1.BIT.MD = 0x2; // PWM mode 1
            MTU0.TIORH.BIT.IOA = 0x5; // compare match high - low
            MTU0.TIORH.BIT.IOB = 0x6; // compare match high - high
            MTU0.TIORL.BIT.IOC = 0x5; // compare match high - low
            MTU0.TIORL.BIT.IOD = 0x6; // compare match high - high
            break;
        case 1:
            MTU1.TCR.BIT.CCLR = 0x6; // compare match clear by TGRD
            MTU1.TMDR1.BIT.MD = 0x2; // PWM mode 1
            MTU1.TIOR.BIT.IOA = 0x5; // compare match high - low
            MTU1.TIOR.BIT.IOB = 0x6; // compare match high - high
            break;
        case 2:
            MTU2.TCR.BIT.CCLR = 0x6; // compare match clear by TGRD
//        MTU2.TMDR1.BIT.MD = 0x2;     // PWM mode 1
            MTU2.TIOR.BIT.IOA = 0x5; // compare match high - low
            MTU2.TIOR.BIT.IOB = 0x6; // compare match high - high
            break;
        case 3:
            MTU3.TCR.BIT.CCLR = 0x6; // compare match clear by TGRD
            MTU3.TMDR1.BIT.MD = 0x2; // PWM mode 1
            MTU3.TIORH.BIT.IOA = 0x5; // compare match high - low
            MTU3.TIORH.BIT.IOB = 0x6; // compare match high - high
            MTU3.TIORL.BIT.IOC = 0x5; // compare match high - low
            MTU3.TIORL.BIT.IOD = 0x6; // compare match high - high
            break;
        case 4:
            MTU4.TCR.BIT.CCLR = 0x6; // compare match clear by TGRD
            MTU4.TMDR1.BIT.MD = 0x2; // PWM mode 1
            MTU4.TIORH.BIT.IOA = 0x5; // compare match high - low
            MTU4.TIORH.BIT.IOB = 0x6; // compare match high - high
            MTU4.TIORL.BIT.IOC = 0x5; // compare match high - low
            MTU4.TIORL.BIT.IOD = 0x6; // compare match high - high
            break;
        default:
            break;
    }
}

void rz_pwm_set_channel_params(uint32_t pin, int channel, float freq, float duty) {
    uint32_t period_ticks;
    uint32_t duration_ticks;
    uint8_t mtu_pin = rz_pwm_get_mtu_pin(pin);
    uint32_t clkdev = rz_pwm_get_clock_dev(channel, freq);
    rz_pwm_set_clock(channel, clkdev);
    float ticks = ((float)PCLK) / (freq * ((float)clkdev));
    period_ticks = (uint32_t)ticks;
    duration_ticks = (uint32_t)(ticks * duty);
    switch (channel)
    {
        case 0:
            if (mtu_pin == MTIOC0A) {
                MTU0.TGRA.WORD = duration_ticks;
                MTU0.TGRB.WORD = period_ticks;
                MTU0.TGRD.WORD = period_ticks;
                MTU0.TCNT.WORD = 0;
            } else if (mtu_pin == MTIOC0C) {
                MTU0.TGRC.WORD = duration_ticks;
                MTU0.TGRB.WORD = period_ticks;
                MTU0.TGRD.WORD = period_ticks;
                MTU0.TCNT.WORD = 0;
                MTU0.TCNT.WORD = 0;
            }
            break;
        case 1:
            if (mtu_pin == MTIOC1A) {
                MTU1.TGRA.WORD = duration_ticks;
                MTU1.TGRB.WORD = period_ticks;
                MTU1.TCNT.WORD = 0;
            }
            break;
        case 2:
            if (mtu_pin == MTIOC2A) {
                MTU2.TGRA.WORD = duration_ticks;
                MTU2.TGRB.WORD = period_ticks;
                MTU2.TCNT.WORD = 0;
            }
            break;
        case 3:
            if (mtu_pin == MTIOC3A) {
                MTU3.TGRA.WORD = duration_ticks;
                MTU3.TGRB.WORD = period_ticks;
                MTU3.TGRD.WORD = period_ticks;
                MTU3.TCNT.WORD = 0;
            } else if (mtu_pin == MTIOC3C) {
                MTU3.TGRC.WORD = duration_ticks;
                MTU3.TGRB.WORD = period_ticks;
                MTU3.TGRD.WORD = period_ticks;
                MTU3.TCNT.WORD = 0;
            }
            break;
        case 4:
            if (mtu_pin == MTIOC4A) {
                MTU4.TGRA.WORD = duration_ticks;
                MTU4.TGRB.WORD = period_ticks;
                MTU4.TGRD.WORD = period_ticks;
                MTU4.TCNT.WORD = 0;
                MTU.TOERA.BIT.OE4A = 1;
            } else if (mtu_pin == MTIOC4C) {
                MTU4.TGRC.WORD = duration_ticks;
                MTU4.TGRB.WORD = period_ticks;
                MTU4.TGRD.WORD = period_ticks;
                MTU4.TCNT.WORD = 0;
                MTU.TOERA.BIT.OE4C = 1;
            }
            break;
        default:
            return;
    }
    #ifdef DEBUG_PWM
    debug_printf("Dt/D/P %04x/%06x/%06x\r\n", (UINT16)duration_ticks, duration, period);
    #endif
    return;
}

void rz_pwm_set_freq(uint32_t pin, float freq) {
    uint8_t channel = rz_pwm_get_mtu_channel(pin);
    if (channel == MTU_END) {
        return;
    }
    rz_pwm_channel_freq[channel] = freq;
    rz_pwm_set_channel_params(pin, channel, freq, rz_pwm_channel_duty[channel]);
}

float rz_pwm_get_freq(uint32_t pin) {
    uint8_t channel = rz_pwm_get_mtu_channel(pin);
    if (channel == MTU_END) {
        return PWM_DEFAULT_FREQ;
    }
    return rz_pwm_channel_freq[channel];
}

void rz_pwm_set_duty(uint32_t pin, float duty) {
    uint8_t channel = rz_pwm_get_mtu_channel(pin);
    if (channel == MTU_END) {
        return;
    }
    rz_pwm_channel_duty[channel] = duty;
    rz_pwm_set_channel_params(pin, channel, rz_pwm_channel_freq[channel], duty);
}

float rz_pwm_get_duty(uint32_t pin) {
    uint8_t channel = rz_pwm_get_mtu_channel(pin);
    if (channel == MTU_END) {
        return PWM_DEFAULT_DUTY;
    }
    return rz_pwm_channel_duty[channel];
}

void rz_pwm_start(uint32_t pin) {
    uint8_t channel = rz_pwm_get_mtu_channel(pin);
    if (channel == MTU_END) {
        return;
    }
    rz_pwm_channel_enable(channel);
}

void rz_pwm_stop(uint32_t pin) {
    uint8_t channel = rz_pwm_get_mtu_channel(pin);
    if (channel == MTU_END) {
        return;
    }
    rz_pwm_channel_disable(channel);
}

static void rz_pwm_set_pin(uint32_t pin) {
    uint8_t af_no = rz_pwm_get_af(pin);
    rz_gpio_mode_output(pin);
    rz_gpio_mode_af(pin, af_no);
}

static void rz_pwm_reset_pin(uint32_t pin) {
    rz_gpio_mode_output(pin);
    rz_gpio_mode_af(pin, 0);
}

void rz_pwm_pin_init(uint32_t pin) {
    uint8_t channel = rz_pwm_get_mtu_channel(pin);
    if (channel == MTU_END) {
        return;
    }
    rz_pwm_set_pin(pin);
    rz_pwm_channel_init(channel);
    rz_pwm_set_channel_params(pin, channel,
        rz_pwm_channel_freq[channel],
        rz_pwm_channel_duty[channel]);
    rz_pwm_channel_enable(channel);
}

void rz_pwm_pin_deinit(uint32_t pin) {
    uint8_t channel = rz_pwm_get_mtu_channel(pin);
    if (channel == MTU_END) {
        return;
    }
    rz_pwm_reset_pin(pin);
    rz_pwm_channel_disable(channel);
}

static void rz_pwm_channel_init_clear_flag(void) {
    for (int i = 0; i < PWM_CHANNEL_SIZE; i++) {
        rz_pwm_channel_init_flag[i] = false;
    }
}

void rz_pwm_init(void) {
    CPG.STBCR3.BIT.MSTP33 = 0;
    rz_pwm_channel_init_clear_flag();
}

void rz_pwm_deinit(void) {
    CPG.STBCR3.BIT.MSTP33 = 1;
    rz_pwm_channel_init_clear_flag();
}
