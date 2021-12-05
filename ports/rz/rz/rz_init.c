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

#include "common.h"
#include "iodefine.h"
#include "rza2m_config.h"
#include "rz_rtc.h"
#include "rz_init.h"

void internal_flash_init(void);

void rz_software_reset(void) {
    volatile uint16_t data;
    WDT.WTCNT.WORD = 0x5A00;
    data = WDT.WRCSR.WORD;  // dummy read
    (void)data;             // to suppress gcc warning
    WDT.WTCNT.WORD = 0x5A00;
    WDT.WRCSR.WORD = 0xA500;
    WDT.WTCSR.WORD = 0xA578;
    WDT.WRCSR.WORD = 0x5A40;
    while (1) {
    }
}

void rz_init(void) {
    bootstrap();
    // exti_init();
    // exti_deinit();
    // udelay_init();
    rz_rtc_init();
    #ifdef USE_DBG_PRINT
    sci_init_default(DEBUG_CH, SCI_BAUD);
    sci_tx_str(DEBUG_CH, (uint8_t *)"\r\n*** USE_DBG_PRINT ***\r\n");
    sci_tx_str(DEBUG_CH, (uint8_t *)"rza2m_init\r\n");
    #endif
    // usb_init();
    // internal_flash_init();
}
