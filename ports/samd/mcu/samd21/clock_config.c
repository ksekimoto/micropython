/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * This file provides functions for configuring the clocks.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 Robert Hammelrath
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

#include <stdint.h>

#include "py/runtime.h"
#include "py/mphal.h"
#include "samd_soc.h"

static uint32_t cpu_freq = CPU_FREQ;
static uint32_t peripheral_freq = DFLL48M_FREQ;
static uint32_t dfll48m_calibration;

int sercom_gclk_id[] = {
    GCLK_CLKCTRL_ID_SERCOM0_CORE, GCLK_CLKCTRL_ID_SERCOM1_CORE,
    GCLK_CLKCTRL_ID_SERCOM2_CORE, GCLK_CLKCTRL_ID_SERCOM3_CORE,
    GCLK_CLKCTRL_ID_SERCOM4_CORE, GCLK_CLKCTRL_ID_SERCOM5_CORE
};

uint32_t get_cpu_freq(void) {
    return cpu_freq;
}

uint32_t get_peripheral_freq(void) {
    return peripheral_freq;
}

void set_cpu_freq(uint32_t cpu_freq_arg) {
    cpu_freq = cpu_freq_arg;
}

void check_usb_recovery_mode(void) {
    #if !MICROPY_HW_XOSC32K
    mp_hal_delay_ms(500);
    // Check USB status. If not connected, switch DFLL48M back to open loop
    if (USB->DEVICE.DeviceEndpoint[0].EPCFG.reg == 0) {
        // Set/keep the open loop mode of the device.
        SYSCTRL->DFLLVAL.reg = dfll48m_calibration;
        SYSCTRL->DFLLCTRL.reg = SYSCTRL_DFLLCTRL_CCDIS | SYSCTRL_DFLLCTRL_ENABLE;
    }
    #endif // MICROPY_HW_XOSC32K
}

void init_clocks(uint32_t cpu_freq) {

    dfll48m_calibration = 0; // please the compiler

    // SAMD21 Clock settings
    // GCLK0: 48MHz from DFLL open loop mode or closed loop mode from 32k Crystal
    // GCLK1: 32768 Hz from 32K ULP or DFLL48M
    // GCLK2: 48MHz from DFLL for Peripherals
    // GCLK3: 1Mhz for the us-counter (TC4/TC5)
    // GCLK4: 32kHz from crystal, if present
    // GCLK8: 1kHz clock for WDT

    NVMCTRL->CTRLB.bit.MANW = 1; // errata "Spurious Writes"
    NVMCTRL->CTRLB.bit.RWS = 1; // 1 read wait state for 48MHz

    #if MICROPY_HW_XOSC32K
    // Set up OSC32K according datasheet 17.6.3
    SYSCTRL->XOSC32K.reg = SYSCTRL_XOSC32K_STARTUP(0x3) | SYSCTRL_XOSC32K_EN32K |
        SYSCTRL_XOSC32K_XTALEN;
    SYSCTRL->XOSC32K.bit.ENABLE = 1;
    while (SYSCTRL->PCLKSR.bit.XOSC32KRDY == 0) {
    }
    // Set up the DFLL48 according to the data sheet 17.6.7.1.2
    // Step 1: Set up the reference clock

    #if MICROPY_HW_MCU_OSC32KULP
    // Connect the GCLK1 to the XOSC32KULP
    GCLK->GENDIV.reg = GCLK_GENDIV_ID(1) | GCLK_GENDIV_DIV(1);
    GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_OSCULP32K | GCLK_GENCTRL_ID(1);
    #else
    // Connect the GCLK1 to OSC32K via GCLK1 to the DFLL input and for further use.
    GCLK->GENDIV.reg = GCLK_GENDIV_ID(1) | GCLK_GENDIV_DIV(1);
    GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_XOSC32K | GCLK_GENCTRL_ID(1);
    #endif

    while (GCLK->STATUS.bit.SYNCBUSY) {
    }

    // Connect the GCLK4 to OSC32K via GCLK1 to the DFLL input and for further use.
    GCLK->GENDIV.reg = GCLK_GENDIV_ID(4) | GCLK_GENDIV_DIV(1);
    GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_XOSC32K | GCLK_GENCTRL_ID(4);
    while (GCLK->STATUS.bit.SYNCBUSY) {
    }

    // Connect GCLK4 to the DFLL input and for further use.
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_DFLL48 | GCLK_CLKCTRL_GEN_GCLK4 | GCLK_CLKCTRL_CLKEN;
    // Enable access to the DFLLCTRL reg acc. to Errata 1.2.1
    SYSCTRL->DFLLCTRL.reg = SYSCTRL_DFLLCTRL_ENABLE;
    while (SYSCTRL->PCLKSR.bit.DFLLRDY == 0) {
    }
    // Step 2: Set the coarse and fine values.
    // Get the coarse value from the calib data. In case it is not set,
    // set a midrange value.
    uint32_t coarse = (*((uint32_t *)FUSES_DFLL48M_COARSE_CAL_ADDR) & FUSES_DFLL48M_COARSE_CAL_Msk)
        >> FUSES_DFLL48M_COARSE_CAL_Pos;
    if (coarse == 0x3f) {
        coarse = 0x1f;
    }
    SYSCTRL->DFLLVAL.reg = SYSCTRL_DFLLVAL_COARSE(coarse) | SYSCTRL_DFLLVAL_FINE(512);
    while (SYSCTRL->PCLKSR.bit.DFLLRDY == 0) {
    }
    // Step 3: Set the multiplication values. The offset of 16384 to the freq is for rounding.
    SYSCTRL->DFLLMUL.reg = SYSCTRL_DFLLMUL_MUL((CPU_FREQ + 16384) / 32768) |
        SYSCTRL_DFLLMUL_FSTEP(1) | SYSCTRL_DFLLMUL_CSTEP(1);
    while (SYSCTRL->PCLKSR.bit.DFLLRDY == 0) {
    }
    // Step 4: Start the DFLL and wait for the PLL lock. We just wait for the fine lock, since
    // coarse adjusting is bypassed.
    SYSCTRL->DFLLCTRL.reg |= SYSCTRL_DFLLCTRL_MODE | SYSCTRL_DFLLCTRL_WAITLOCK | SYSCTRL_DFLLCTRL_STABLE |
        SYSCTRL_DFLLCTRL_BPLCKC | SYSCTRL_DFLLCTRL_ENABLE;
    while (SYSCTRL->PCLKSR.bit.DFLLLCKF == 0) {
    }

    #else // MICROPY_HW_XOSC32K

    // Enable DFLL48M
    SYSCTRL->DFLLCTRL.reg = SYSCTRL_DFLLCTRL_ENABLE;
    while (!SYSCTRL->PCLKSR.bit.DFLLRDY) {
    }

    uint32_t coarse = (*((uint32_t *)FUSES_DFLL48M_COARSE_CAL_ADDR) & FUSES_DFLL48M_COARSE_CAL_Msk)
        >> FUSES_DFLL48M_COARSE_CAL_Pos;
    if (coarse == 0x3f) {
        coarse = 0x1f;
    }
    SYSCTRL->DFLLVAL.reg = SYSCTRL_DFLLVAL_COARSE(coarse) | SYSCTRL_DFLLVAL_FINE(511);

    #if MICROPY_HW_DFLL_USB_SYNC
    // Configure the DFLL48M for USB clock recovery.
    // Will have to switch back if no USB
    SYSCTRL->DFLLSYNC.bit.READREQ = 1;
    dfll48m_calibration = SYSCTRL->DFLLVAL.reg;
    // Set the Multiplication factor.
    SYSCTRL->DFLLMUL.reg = SYSCTRL_DFLLMUL_CSTEP(1) | SYSCTRL_DFLLMUL_FSTEP(1)
        | SYSCTRL_DFLLMUL_MUL(48000);
    // Set the mode to closed loop USB Recovery mode
    SYSCTRL->DFLLCTRL.reg = SYSCTRL_DFLLCTRL_USBCRM | SYSCTRL_DFLLCTRL_CCDIS
        | SYSCTRL_DFLLCTRL_MODE | SYSCTRL_DFLLCTRL_ENABLE;
    #else
    // Set/keep the open loop mode of the device.
    SYSCTRL->DFLLCTRL.reg = SYSCTRL_DFLLCTRL_CCDIS | SYSCTRL_DFLLCTRL_ENABLE;
    #endif

    while (!SYSCTRL->PCLKSR.bit.DFLLRDY) {
    }

    // Enable 32768 Hz on GCLK1 for consistency
    GCLK->GENDIV.reg = GCLK_GENDIV_ID(1) | GCLK_GENDIV_DIV(48016384 / 32768);
    GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_DFLL48M | GCLK_GENCTRL_ID(1);
    while (GCLK->STATUS.bit.SYNCBUSY) {
    }

    #endif // MICROPY_HW_XOSC32K

    // Enable GCLK output: 48M on both CCLK0 and GCLK2
    GCLK->GENDIV.reg = GCLK_GENDIV_ID(0) | GCLK_GENDIV_DIV(1);
    GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_DFLL48M | GCLK_GENCTRL_ID(0);
    while (GCLK->STATUS.bit.SYNCBUSY) {
    }
    GCLK->GENDIV.reg = GCLK_GENDIV_ID(2) | GCLK_GENDIV_DIV(1);
    GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_DFLL48M | GCLK_GENCTRL_ID(2);
    while (GCLK->STATUS.bit.SYNCBUSY) {
    }

    // Enable GCLK output: 1MHz on GCLK3 for TC4
    GCLK->GENDIV.reg = GCLK_GENDIV_ID(3) | GCLK_GENDIV_DIV(48);
    GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_DFLL48M | GCLK_GENCTRL_ID(3);
    while (GCLK->STATUS.bit.SYNCBUSY) {
    }
    // Set GCLK8 to 1 kHz.
    GCLK->GENDIV.reg = GCLK_GENDIV_ID(8) | GCLK_GENDIV_DIV(32);
    GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_OSCULP32K | GCLK_GENCTRL_ID(8);
    while (GCLK->STATUS.bit.SYNCBUSY) {
    }

}

void enable_sercom_clock(int id) {
    // Enable synchronous clock. The bits are nicely arranged
    PM->APBCMASK.reg |= 0x04 << id;
    // Select multiplexer generic clock source and enable.
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK2 | sercom_gclk_id[id];
    // Wait while it updates synchronously.
    while (GCLK->STATUS.bit.SYNCBUSY) {
    }
}
