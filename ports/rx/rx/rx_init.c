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
#include "bootstrap.h"
#include "rx_exti.h"
#include "rx_flash.h"
#include "rx_rtc.h"
#include "rx_timer.h"
#ifdef USE_DBG_PRINT
#include "rx_sci.h"
#endif

void internal_flash_init(void);

void rx_software_reset(void) {
    SYSTEM.PRCR.WORD = 0xA502;  /* Enable writing to the Software Reset */
    SYSTEM.SWRR = 0xA501;       /* Software Reset */
    SYSTEM.PRCR.WORD = 0xA500;  /* Disable writing to the Software Reset */
}

void rx_init(void) {
    bootstrap();
    rx_exti_init();
    rx_exti_deinit();
    udelay_init();
    rx_rtc_init();
    #ifdef USE_DBG_PRINT
    rx_sci_init_default(DEBUG_CH, DEBUG_CH_TX, DEBUG_CH_RX, DEBUG_BAUD);
    rx_sci_tx_str(DEBUG_CH, (uint8_t *)"\r\n*** USE_DBG_PRINT ***\r\n");
    rx_sci_tx_str(DEBUG_CH, (uint8_t *)"rx_init\r\n");
    #endif
    // usb_init();
    internal_flash_init();
}
