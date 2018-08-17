/*
  util.c - 
  Copyright (c) 2014 Nozomu Fujita.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "rx63n/util.h"
#include "rx63n/iodefine.h"
#include "rx63n/interrupt_handlers.h"

static void _startOrStopModule(MstpId m, bool start);

#define sei() \
do { \
  __asm __volatile("setpsw i\n"); \
} while (0)

#define cli() \
do { \
  __asm __volatile("clrpsw i\n"); \
} while (0)

#define isNoInterrupts() \
({ \
  bool ret; \
  __asm __volatile( \
    "mvfc psw, %0\n" \
    "btst #16, %0\n" \
    "sceq.l %0\n" \
    : "=r" (ret) \
    : \
    : \
  ); \
  ret; \
})

#define pushi() \
{ \
  bool _di = isNoInterrupts();

#define popi() \
  if (_di) { \
    cli(); \
  } else { \
    sei(); \
  } \
}

#define BSET(port, bit) \
do { \
  volatile byte* _port = (port); \
  int _bit = (bit); \
  __asm __volatile( \
    "bset %1, [%0].b\n" \
    : \
    : "r" (_port), "r" (_bit) \
    : \
  ); \
} while (0)

#define BCLR(port, bit) \
do { \
  volatile byte* _port = (port); \
  int _bit = (bit); \
  __asm __volatile( \
    "bclr %1, [%0].b\n" \
    : \
    : "r" (_port), "r" (_bit) \
    : \
  ); \
} while (0)

#define BTST(port, bit) \
({ \
  volatile byte* _port = (port); \
  int _bit = (bit); \
  int ret; \
  __asm __volatile( \
    "btst %2, [%1].b\n" \
    "scnz.l %0\n" \
    : "=r" (ret) \
    : "r" (_port), "r" (_bit) \
    : \
  ); \
  ret; \
})

#define sbi(port, bit) BSET((port), (bit))
#define cbi(port, bit) BCLR((port), (bit))

void startModule(MstpId module)
{
    _startOrStopModule(module, true);
}

void stopModule(MstpId module)
{
    _startOrStopModule(module, false);
}

static void _startOrStopModule(MstpId id, bool start)
{
    static const struct {
        int id:9;
        uint8_t reg:2;
        uint8_t bit:5;
    } t[] = {
        {MstpIdEXDMAC,  0, 29},
        {MstpIdEXDMAC0, 0, 29},
        {MstpIdEXDMAC1, 0, 29},
        {MstpIdDMAC,    0, 28},
        {MstpIdDMAC0,   0, 28},
        {MstpIdDMAC1,   0, 28},
        {MstpIdDMAC2,   0, 28},
        {MstpIdDMAC3,   0, 28},
        {MstpIdDTC,     0, 28},
        {MstpIdA27,     0, 27},
        {MstpIdA24,     0, 24},
        {MstpIdAD,      0, 23},
        {MstpIdDA,      0, 19},
        {MstpIdS12AD,   0, 17},
        {MstpIdCMT0,    0, 15},
        {MstpIdCMT1,    0, 15},
        {MstpIdCMT2,    0, 14},
        {MstpIdCMT3,    0, 14},
        {MstpIdTPU0,    0, 13},
        {MstpIdTPU1,    0, 13},
        {MstpIdTPU2,    0, 13},
        {MstpIdTPU3,    0, 13},
        {MstpIdTPU4,    0, 13},
        {MstpIdTPU5,    0, 13},
        {MstpIdTPU6,    0, 12},
        {MstpIdTPU7,    0, 12},
        {MstpIdTPU8,    0, 12},
        {MstpIdTPU9,    0, 12},
        {MstpIdTPU10,   0, 12},
        {MstpIdTPU11,   0, 12},
        {MstpIdPPG0,    0, 11},
        {MstpIdPPG1,    0, 10},
        {MstpIdMTU,     0,  9},
        {MstpIdMTU0,    0,  9},
        {MstpIdMTU1,    0,  9},
        {MstpIdMTU2,    0,  9},
        {MstpIdMTU3,    0,  9},
        {MstpIdMTU4,    0,  9},
        {MstpIdMTU5,    0,  9},
        {MstpIdTMR0,    0,  5},
        {MstpIdTMR1,    0,  5},
        {MstpIdTMR01,   0,  5},
        {MstpIdTMR2,    0,  4},
        {MstpIdTMR3,    0,  4},
        {MstpIdTMR23,   0,  4},
        {MstpIdSCI0,    1, 31},
        {MstpIdSMCI0,   1, 31},
        {MstpIdSCI1,    1, 30},
        {MstpIdSMCI1,   1, 30},
        {MstpIdSCI2,    1, 29},
        {MstpIdSMCI2,   1, 29},
        {MstpIdSCI3,    1, 28},
        {MstpIdSMCI3,   1, 28},
        {MstpIdSCI4,    1, 27},
        {MstpIdSMCI4,   1, 27},
        {MstpIdSCI5,    1, 26},
        {MstpIdSMCI5,   1, 26},
        {MstpIdSCI6,    1, 25},
        {MstpIdSMCI6,   1, 25},
        {MstpIdSCI7,    1, 24},
        {MstpIdSMCI7,   1, 24},
        {MstpIdCRC,     1, 23},
        {MstpIdPDC,     1, 22},
        {MstpIdRIIC0,   1, 21},
        {MstpIdRIIC1,   1, 20},
        {MstpIdUSB0,    1, 19},
        {MstpIdUSB1,    1, 18},
        {MstpIdRSPI0,   1, 17},
        {MstpIdRSPI1,   1, 16},
        {MstpIdEDMAC,   1, 15},
        {MstpIdTEMPS,   1,  8},
        {MstpIdSCI12,   1,  4},
        {MstpIdSMCI12,  1,  4},
        {MstpIdCAN2,    1,  2},
        {MstpIdCAN1,    1,  1},
        {MstpIdCAN0,    1,  0},
        {MstpIdSCI8,    2, 27},
        {MstpIdSMCI8,   2, 27},
        {MstpIdSCI9,    2, 26},
        {MstpIdSMCI9,   2, 26},
        {MstpIdSCI10,   2, 25},
        {MstpIdSMCI10,  2, 25},
        {MstpIdSCI11,   2, 24},
        {MstpIdSMCI11,  2, 24},
        {MstpIdRSPI2,   2, 22},
        {MstpIdMCK,     2, 19},
        {MstpIdIEB,     2, 18},
        {MstpIdRIIC2,   2, 17},
        {MstpIdRIIC3,   2, 16},
        {MstpIdRAM1,    2,  1},
        {MstpIdRAM0,    2,  0},
        {MstpIdDEU,     3, 31},
    };
    static uint8_t f[(NumOfMstpId + __CHAR_BIT__ - 1) / __CHAR_BIT__] = {0};

    if (id >= 0 && id < NumOfMstpId) {
        int reg;
        volatile uint32_t* mstpcr = NULL;
        int bit;
        int i;
        for (i = 0; i < (int)(sizeof(t) / sizeof(*t)); i++) {
            if (t[i].id == id) {
                reg = t[i].reg;
                mstpcr = (volatile uint32_t*)((uint32_t*)&SYSTEM.MSTPCRA.LONG + reg);
                bit = t[i].bit;
                break;
            }
        }
        if (mstpcr != NULL) {
            int c = 0;
            int j;
            for (j = 0; j < (int)(sizeof(t) / sizeof(*t)); j++) {
                if (t[j].id != id && (t[j].reg == reg && t[j].bit == bit)) {
                    if ((f[t[j].id / __CHAR_BIT__] & (1 << (t[j].id % __CHAR_BIT__))) != 0) {
                        c++;
                    }
                }
            }
            if (start) {
                if ((f[id / __CHAR_BIT__] & (1 << (id % __CHAR_BIT__))) == 0) {
                    f[id / __CHAR_BIT__] |= (1 << (id % __CHAR_BIT__));
                    if (c == 0) {
                        pushi();
                        cli();
                        *mstpcr &= ~(1U << bit);
                        popi();
                    }
                }
            } else {
                if ((f[id / __CHAR_BIT__] & (1 << (id % __CHAR_BIT__))) != 0) {
                    f[id / __CHAR_BIT__] &= ~(1 << (id % __CHAR_BIT__));
                    if (c == 0) {
                        pushi();
                        cli();
                        *mstpcr |= (1U << bit);
                        popi();
                    }
                }
            }
        }
    }
}

