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

#include "interrupt_handlers.h"
#include "iodefine.h"
#include "usb_hal.h"
#include "usb_cdc.h"
#include "usb_msc.h"
#include "common.h"
#include "pendsv.h"
#include "usbdescriptors.h"

#define PID_NAK     0
#define PID_BUF     1
#define PID_STALL_1 2
#define PID_STALL_2 3
#define USBCDC_BUF_SIZE 512

static uint8_t rx_buf[USBCDC_BUF_SIZE];
static uint8_t tx_buf[USBCDC_BUF_SIZE];

volatile bool _begin = false;
volatile uint32_t rx_buf_head;
volatile uint32_t rx_buf_tail;
volatile uint32_t tx_buf_head;
volatile uint32_t tx_buf_tail;

static USB_CALLBACK usb_callback = 0;

void usb_rx_set_callback(USB_CALLBACK callback) {
    usb_callback = callback;
}

bool tx_buf_available() {
    return (tx_buf_head != tx_buf_tail);
}

int rx_write_buf(uint8_t c) {
    uint32_t i = (uint32_t)(rx_buf_head + 1) % USBCDC_BUF_SIZE;
    if (i != rx_buf_tail) {
        rx_buf[rx_buf_head] = c;
        rx_buf_head = i;
        return 1;
    } else {
        return 0;
    }
}

uint8_t tx_read_buf() {
    uint8_t c = tx_buf[tx_buf_tail];
    if (tx_buf_head != tx_buf_tail) {
        tx_buf_tail = (tx_buf_tail + 1) % USBCDC_BUF_SIZE;
    }
    return c;
}

void ReadBulkOUTPacketCDC(void) {
    uint16_t DataLength = 0;
    /*Read data using D1FIFO*/
    /*NOTE: This probably will have already been selected if using BRDY interrupt.*/
    do {
        USB0.D1FIFOSEL.BIT.CURPIPE = PIPE_BULK_OUT_CDC;
    } while (USB0.D1FIFOSEL.BIT.CURPIPE != PIPE_BULK_OUT_CDC);
    /*Set PID to BUF*/
    USB0.PIPE1CTR.BIT.PID = PID_BUF;
    /*Wait for buffer to be ready*/
    while (USB0.D1FIFOCTR.BIT.FRDY == 0) {
        ;
    }
    /*Set Read Count Mode - so DTLN count will decrement as data read from buffer*/
    USB0.D1FIFOSEL.BIT.RCNT = 1;
    /*Read length of data */
    DataLength = USB0.D1FIFOCTR.BIT.DTLN;
    if (DataLength == 0) {
        USB0.D1FIFOCTR.BIT.BCLR = 1;
        return;
    }
    while (DataLength != 0) {
        /*Read from the FIFO*/
        uint16_t c = USB0.D1FIFO.WORD;
        if (DataLength >= 2) {
            /*Save first byte*/
            if (usb_callback) {
                if ((*usb_callback)((int)(c & 0xff))) {
                    return;
                }
            }
            rx_write_buf((uint8_t)c);
            /*Save second byte*/
            if (usb_callback) {
                if ((*usb_callback)((int)(c >> 8))) {
                    return;
                }
            }
            rx_write_buf((uint8_t)(c >> 8));
            DataLength -= 2;
        } else {
            if (usb_callback) {
                if ((*usb_callback)((int)(c & 0xff))) {
                    return;
                }
            }
            rx_write_buf((uint8_t)c);
            DataLength--;
        }
    }
}

void WriteBulkINPacketCDC(void) {
    uint32_t Count = 0;
    /*Write data to Bulk IN pipe using D0FIFO*/
    /*Select pipe (Check this happens before continuing)*/
    /*Set 8 bit access*/
    USB0.D0FIFOSEL.BIT.MBW = 0;
    do {
        USB0.D0FIFOSEL.BIT.CURPIPE = PIPE_BULK_IN_CDC;
    } while (USB0.D0FIFOSEL.BIT.CURPIPE != PIPE_BULK_IN_CDC);
    /*Wait for buffer to be ready*/
    while (USB0.D0FIFOCTR.BIT.FRDY == 0) {
        ;
    }
    /* Write data to the IN Fifo until have written a full packet
     or we have no more data to write */
    while ((Count < BULK_IN_PACKET_SIZE) && tx_buf_available()) {
        USB0.D0FIFO.WORD = (unsigned short)tx_read_buf();
        Count++;
    }
    /*Send the packet */
    /*Set PID to BUF*/
    USB0.PIPE2CTR.BIT.PID = PID_BUF;
    /*If we have not written a full packets worth to the buffer then need to
     signal that the buffer is now ready to be sent, set the buffer valid flag (BVAL).*/
    if (Count != BULK_IN_PACKET_SIZE) {
        USB0.D0FIFOCTR.BIT.BVAL = 1;
    }
    if (!tx_buf_available()) {
        USB0.BRDYENB.BIT.PIPE2BRDYE = 0;
    }
}

void usbcdc_write(uint8_t c) {
    unsigned int i = (tx_buf_head + 1) % USBCDC_BUF_SIZE;

    if (_begin) {
        if (i != tx_buf_tail) {
            USB0.INTENB0.BIT.BRDYE = 0;
            tx_buf[tx_buf_head] = c;
            tx_buf_head = i;
            USB0.INTENB0.BIT.BRDYE = 1;
            USB0.BRDYENB.BIT.PIPE2BRDYE = 1;
        }
    } else {
        if (USBCDC_IsConnected()) {
            _begin = true;
        }
        tx_buf[tx_buf_head] = c;
        tx_buf_head = i;
    }
}

int usbcdc_read(void) {
    // if the head isn't ahead of the tail, we don't have any characters
    if (rx_buf_head == rx_buf_tail) {
        return -1;
    } else {
        uint8_t c = rx_buf[rx_buf_tail];
        rx_buf_tail = (uint32_t)(rx_buf_tail + 1) % USBCDC_BUF_SIZE;
        return c;
    }
}

void usb_init(void) {
#if defined(USB_COMB)
    USB_ERR err = USBCDCMSC_Init();
#elif defined (USB_MSC)
    USB_ERR err = USBMSC_Init();
#else
    USB_ERR err = USBCDC_Init();
#endif
    if (err == USB_ERR_OK) {
        const unsigned long TimeOut = 3000;
        unsigned long start = mtick();
        while ((mtick() - start) < TimeOut) {
            if (USBCDC_IsConnected()) {
                _begin = true;
                break;
            }
        }
    }
    //err = USBMSC_Init();
}

void INT_Excep_USB0_USBI0(void) {
    if (USB0.SYSCFG.BIT.DCFM == 0) {/* Function controller is selected */
        USBHALInterruptHandler();
    } else if (USB0.SYSCFG.BIT.DCFM == 1) {/* Host controller is selected */
        //InterruptHandler_USBHost();
    }
}

