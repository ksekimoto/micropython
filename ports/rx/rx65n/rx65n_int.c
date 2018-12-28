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
#include "common.h"
#include "iodefine.h"

#if (defined(RX64M) || defined(RX65N))
extern void sci_isr_te(int ch);
extern void sci_isr_er(int ch);

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
        sci_isr_te(0);
    }
    if (1 == ICU.GRPBL0.BIT.IS2) {
        sci_isr_te(1);
    }
    if (1 == ICU.GRPBL0.BIT.IS4) {
        sci_isr_te(2);
    }
    if (1 == ICU.GRPBL0.BIT.IS6) {
        sci_isr_te(3);
    }
    if (1 == ICU.GRPBL0.BIT.IS8) {
        sci_isr_te(4);
    }
    if (1 == ICU.GRPBL0.BIT.IS10) {
        sci_isr_te(5);
    }
    if (1 == ICU.GRPBL0.BIT.IS12) {
        sci_isr_te(6);
    }
    if (1 == ICU.GRPBL0.BIT.IS14) {
        sci_isr_te(7);
    }
    if (1 == ICU.GRPBL0.BIT.IS16) {
        sci_isr_te(12);
    }
    if (1 == ICU.GRPBL0.BIT.IS1) {
        sci_isr_er(0);
    }
    if (1 == ICU.GRPBL0.BIT.IS3) {
        sci_isr_er(1);
    }
    if (1 == ICU.GRPBL0.BIT.IS5) {
        sci_isr_er(2);
    }
    if (1 == ICU.GRPBL0.BIT.IS7) {
        sci_isr_er(3);
    }
    if (1 == ICU.GRPBL0.BIT.IS9) {
        sci_isr_er(4);
    }
    if (1 == ICU.GRPBL0.BIT.IS11) {
        sci_isr_er(5);
    }
    if (1 == ICU.GRPBL0.BIT.IS13) {
        sci_isr_er(6);
    }
    if (1 == ICU.GRPBL0.BIT.IS15) {
        sci_isr_er(7);
    }
    if (1 == ICU.GRPBL0.BIT.IS17) {
        sci_isr_er(12);
    }
}

// ICU GROUPBL1
// vec: 111
void __attribute__ ((interrupt)) INT_Excep_ICU_GROUPBL1(void) {
    if (1 == ICU.GRPBL1.BIT.IS24) {
        sci_isr_te(8);
    }
    if (1 == ICU.GRPBL1.BIT.IS26) {
        sci_isr_te(9);
    }
    if (1 == ICU.GRPBL1.BIT.IS25) {
        sci_isr_er(8);
    }
    if (1 == ICU.GRPBL1.BIT.IS27) {
        sci_isr_er(9);
    }
}

// ICU GROUPAL0
// vec: 112
void __attribute__ ((interrupt)) INT_Excep_ICU_GROUPAL0(void) {
    if (1 == ICU.GRPAL0.BIT.IS8) {
        sci_isr_te(10);
    }
    if (1 == ICU.GRPAL0.BIT.IS12) {
        sci_isr_te(11);
    }
    if (1 == ICU.GRPAL0.BIT.IS9) {
        sci_isr_er(10);
    }
    if (1 == ICU.GRPAL0.BIT.IS13) {
        sci_isr_er(11);
    }
}

// ICU GROUPAL1
// vec: 113
void __attribute__ ((interrupt)) INT_Excep_ICU_GROUPAL1(void) {

}

void __attribute__ ((interrupt)) INT_Excep_PERIB_INTB110(void)
{
    USBHALInterruptHandler();
    ICU.PIBR7.BYTE |= 0x40;
}
#endif


