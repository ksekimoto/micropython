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

#include <stdint.h>
#include <stdbool.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "common.h"
#include "iodefine.h"
#include "usb_hal.h"

#if (defined(RX64M) || defined(RX65N))
// extern void sci_isr_te(int ch);
// extern void sci_isr_er(int ch);

void set_int_state_groupbl0(int flag) {
    IEN(ICU, GROUPBL0) = 0;
    IR(ICU, GROUPBL0)  = 0;
    IEN(ICU, GROUPBL0) = flag;
}

void set_int_state_groupbl1(int flag) {
    IEN(ICU, GROUPBL1) = 0;
    IR(ICU, GROUPBL1)  = 0;
    IEN(ICU, GROUPBL1) = flag;
}

void set_int_state_groupal0(int flag) {
    IEN(ICU, GROUPAL0) = 0;
    IR(ICU, GROUPAL0)  = 0;
    IEN(ICU, GROUPAL0) = flag;
}

void set_int_priority_groupbl0(int ipl) {
    int flag = IEN(ICU, GROUPBL0);
    IEN(ICU, GROUPBL0) = 0;
    IR(ICU, GROUPBL0)  = 0;
    IPR(ICU, GROUPBL0) = (uint8_t)(ipl > IPR(ICU, GROUPBL0) ? ipl : IPR(ICU, GROUPBL0));
    IEN(ICU, GROUPBL0) = flag;
}

void set_int_priority_groupbl1(int ipl) {
    int flag = IEN(ICU, GROUPBL1);
    IEN(ICU, GROUPBL1) = 0;
    IR(ICU, GROUPBL1)  = 0;
    IPR(ICU, GROUPBL1) = (uint8_t)(ipl > IPR(ICU, GROUPBL1) ? ipl : IPR(ICU, GROUPBL1));
    IEN(ICU, GROUPBL1) = flag;
}

void set_int_priority_groupal0(int ipl) {
    int flag = IEN(ICU, GROUPAL0);
    IEN(ICU, GROUPAL0) = 0;
    IR(ICU, GROUPAL0)  = 0;
    IPR(ICU, GROUPAL0) = (uint8_t)(ipl > IPR(ICU, GROUPAL0) ? ipl : IPR(ICU, GROUPAL0));
    IEN(ICU, GROUPAL0) = flag;
}

// ICU GROUPBL0
// vec: 110
void __attribute__ ((interrupt)) INT_Excep_ICU_GROUPBL0(void) {
    if (1 == ICU.GRPBL0.BIT.IS0) {
        rx_sci_isr_te(0);
    }
    if (1 == ICU.GRPBL0.BIT.IS2) {
        rx_sci_isr_te(1);
    }
    if (1 == ICU.GRPBL0.BIT.IS4) {
        rx_sci_isr_te(2);
    }
    if (1 == ICU.GRPBL0.BIT.IS6) {
        rx_sci_isr_te(3);
    }
    if (1 == ICU.GRPBL0.BIT.IS8) {
        rx_sci_isr_te(4);
    }
    if (1 == ICU.GRPBL0.BIT.IS10) {
        rx_sci_isr_te(5);
    }
    if (1 == ICU.GRPBL0.BIT.IS12) {
        rx_sci_isr_te(6);
    }
    if (1 == ICU.GRPBL0.BIT.IS14) {
        rx_sci_isr_te(7);
    }
    if (1 == ICU.GRPBL0.BIT.IS16) {
        rx_sci_isr_te(12);
    }
    if (1 == ICU.GRPBL0.BIT.IS1) {
        rx_sci_isr_er(0);
    }
    if (1 == ICU.GRPBL0.BIT.IS3) {
        rx_sci_isr_er(1);
    }
    if (1 == ICU.GRPBL0.BIT.IS5) {
        rx_sci_isr_er(2);
    }
    if (1 == ICU.GRPBL0.BIT.IS7) {
        rx_sci_isr_er(3);
    }
    if (1 == ICU.GRPBL0.BIT.IS9) {
        rx_sci_isr_er(4);
    }
    if (1 == ICU.GRPBL0.BIT.IS11) {
        rx_sci_isr_er(5);
    }
    if (1 == ICU.GRPBL0.BIT.IS13) {
        rx_sci_isr_er(6);
    }
    if (1 == ICU.GRPBL0.BIT.IS15) {
        rx_sci_isr_er(7);
    }
    if (1 == ICU.GRPBL0.BIT.IS17) {
        rx_sci_isr_er(12);
    }
}

// ICU GROUPBL1
// vec: 111
void __attribute__ ((interrupt)) INT_Excep_ICU_GROUPBL1(void) {
    if (1 == ICU.GRPBL1.BIT.IS24) {
        rx_sci_isr_te(8);
    }
    if (1 == ICU.GRPBL1.BIT.IS26) {
        rx_sci_isr_te(9);
    }
    if (1 == ICU.GRPBL1.BIT.IS25) {
        rx_sci_isr_er(8);
    }
    if (1 == ICU.GRPBL1.BIT.IS27) {
        rx_sci_isr_er(9);
    }
}

// ICU GROUPAL0
// vec: 112
void __attribute__ ((interrupt)) INT_Excep_ICU_GROUPAL0(void) {
    if (1 == ICU.GRPAL0.BIT.IS8) {
        rx_sci_isr_te(10);
    }
    if (1 == ICU.GRPAL0.BIT.IS12) {
        rx_sci_isr_te(11);
    }
    if (1 == ICU.GRPAL0.BIT.IS9) {
        rx_sci_isr_er(10);
    }
    if (1 == ICU.GRPAL0.BIT.IS13) {
        rx_sci_isr_er(11);
    }
}

// ICU GROUPAL1
// vec: 113
void __attribute__ ((interrupt)) INT_Excep_ICU_GROUPAL1(void) {
    #if MICROPY_HW_ETH_MDC && MICROPY_PY_LWIP
    if (1 == ICU.GRPAL1.BIT.IS4) {
        rx_ether_input_callback();
    }
    #endif // MICROPY_HW_ETH_MDC && MICROPY_PY_LWIP
}

void __attribute__ ((interrupt)) INT_Excep_PERIB_INTB110(void) {
    USBHALInterruptHandler();
    ICU.PIBR7.BYTE |= 0x40;
}
#endif
