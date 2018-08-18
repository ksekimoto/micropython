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

#ifndef RX63N_UTIL_H_
#define RX63N_UTIL_H_

typedef enum {
    MstpIdINVALID = -1,
    MstpIdEXDMAC,
    MstpIdEXDMAC0,
    MstpIdEXDMAC1,
    MstpIdDMAC,
    MstpIdDMAC0,
    MstpIdDMAC1,
    MstpIdDMAC2,
    MstpIdDMAC3,
    MstpIdDTC,
    MstpIdA27,
    MstpIdA24,
    MstpIdAD,
    MstpIdDA,
    MstpIdS12AD,
    MstpIdCMT0,
    MstpIdCMT1,
    MstpIdCMT2,
    MstpIdCMT3,
    MstpIdTPU0,
    MstpIdTPU1,
    MstpIdTPU2,
    MstpIdTPU3,
    MstpIdTPU4,
    MstpIdTPU5,
    MstpIdTPU6,
    MstpIdTPU7,
    MstpIdTPU8,
    MstpIdTPU9,
    MstpIdTPU10,
    MstpIdTPU11,
    MstpIdPPG0,
    MstpIdPPG1,
    MstpIdMTU,
    MstpIdMTU0,
    MstpIdMTU1,
    MstpIdMTU2,
    MstpIdMTU3,
    MstpIdMTU4,
    MstpIdMTU5,
    MstpIdTMR0,
    MstpIdTMR1,
    MstpIdTMR01,
    MstpIdTMR2,
    MstpIdTMR3,
    MstpIdTMR23,
    MstpIdSCI0,
    MstpIdSMCI0,
    MstpIdSCI1,
    MstpIdSMCI1,
    MstpIdSCI2,
    MstpIdSMCI2,
    MstpIdSCI3,
    MstpIdSMCI3,
    MstpIdSCI4,
    MstpIdSMCI4,
    MstpIdSCI5,
    MstpIdSMCI5,
    MstpIdSCI6,
    MstpIdSMCI6,
    MstpIdSCI7,
    MstpIdSMCI7,
    MstpIdCRC,
    MstpIdPDC,
    MstpIdRIIC0,
    MstpIdRIIC1,
    MstpIdUSB0,
    MstpIdUSB1,
    MstpIdRSPI0,
    MstpIdRSPI1,
    MstpIdEDMAC,
    MstpIdTEMPS,
    MstpIdSCI12,
    MstpIdSMCI12,
    MstpIdCAN2,
    MstpIdCAN1,
    MstpIdCAN0,
    MstpIdSCI8,
    MstpIdSMCI8,
    MstpIdSCI9,
    MstpIdSMCI9,
    MstpIdSCI10,
    MstpIdSMCI10,
    MstpIdSCI11,
    MstpIdSMCI11,
    MstpIdRSPI2,
    MstpIdMCK,
    MstpIdIEB,
    MstpIdRIIC2,
    MstpIdRIIC3,
    MstpIdRAM1,
    MstpIdRAM0,
    MstpIdDEU,
    NumOfMstpId,
} MstpId;

#endif /* RX63N_UTIL_H_ */
