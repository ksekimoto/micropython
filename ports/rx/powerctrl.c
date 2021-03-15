/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2018 Damien P. George
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

#include "py/runtime.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "rtc.h"
#include "powerctrl.h"

void powerctrl_mcu_reset(void) {
    // ToDo: implementation
}

// NORETURN void powerctrl_enter_bootloader(uint32_t r0, uint32_t bl_addr) {
// }

// static __attribute__((naked)) void branch_to_bootloader(uint32_t r0, uint32_t
// bl_addr) {
// }

// void powerctrl_check_enter_bootloader(void) {
// }

void powerctrl_enter_stop_mode(void) {
    // ToDo: implementation
    // Disable IRQs so that the IRQ that wakes the device from stop mode is not
    // executed until after the clocks are reconfigured
    uint32_t irq_state = disable_irq();

    #if defined(MICROPY_BOARD_ENTER_STOP)
    MICROPY_BOARD_ENTER_STOP
    #endif

    // reconfigure the system clock after waking up

    #if defined(MICROPY_BOARD_LEAVE_STOP)
    MICROPY_BOARD_LEAVE_STOP
    #endif

    // Enable IRQs now that all clocks are reconfigured
    enable_irq(irq_state);
}

void powerctrl_enter_standby_mode(void) {
    // ToDo: implementation
    rtc_init_finalise();

    #if defined(MICROPY_BOARD_ENTER_STANDBY)
    MICROPY_BOARD_ENTER_STANDBY
    #endif

    // We need to clear the PWR wake-up-flag before entering standby, since
    // the flag may have been set by a previous wake-up event.  Furthermore,
    // we need to disable the wake-up sources while clearing this flag, so
    // that if a source is active it does actually wake the device.
    // See section 5.3.7 of RM0090.

    // save RTC interrupts
    // disable register write protection
    // disable RTC interrupts
    // clear RTC wake-up flags

    // enable previously-enabled RTC interrupts
    // enable register write protection

    // enter standby mode

    // we never return; MCU is reset on exit from standby
}
