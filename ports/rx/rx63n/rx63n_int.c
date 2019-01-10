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

#include <stdint.h>
#include <stdbool.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "common.h"
#include "iodefine.h"

#if 0
void USBHALInterruptHandler(void);

void __attribute__ ((interrupt)) INT_Excep_USB0_USBI0(void) {
    if (USB0.SYSCFG.BIT.DCFM == 0) {/* Function controller is selected */
        USBHALInterruptHandler();
    } else if (USB0.SYSCFG.BIT.DCFM == 1) {/* Host controller is selected */
        //InterruptHandler_USBHost();
    }
}
#endif

#if defined(GRCITRUS)

#else

#if MICROPY_HW_HAS_ETHERNET && MICROPY_PY_LWIP

void __attribute__ ((interrupt)) INT_Excep_ETHER_EINT(void) {
    rx_ether_input_callback();
}

#endif // MICROPY_HW_HAS_ETHERNET && MICROPY_PY_LWIP

#endif // GRCITRUS
