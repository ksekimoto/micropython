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

#include "common.h"
#include "iodefine.h"
#include "rza2m_config.h"
#include "rza2m_rtc.h"
#include "rza2m_init.h"

void internal_flash_init(void);

void rza2m_software_reset(void) {
    volatile uint16_t data;
    WDT.WTCNT.WORD = 0x5A00;
    data = WDT.WRCSR.WORD;
    WDT.WTCNT.WORD = 0x5A00;
    WDT.WRCSR.WORD = 0xA500;
    WDT.WTCSR.WORD = 0xA578;
    WDT.WRCSR.WORD = 0x5A40;
    while(1){}
}

void rza2m_init(void) {
    bootstrap();
    //exti_init();
    //exti_deinit();
    //udelay_init();
    rz_rtc_init();
#ifdef USE_DBG_PRINT
    sci_init_default(DEBUG_CH, SCI_BAUD);
    sci_tx_str(DEBUG_CH, (uint8_t *)"\r\n*** USE_DBG_PRINT ***\r\n");
    sci_tx_str(DEBUG_CH, (uint8_t *)"rza2m_init\r\n");
#endif
    //usb_init();
    //internal_flash_init();
}

