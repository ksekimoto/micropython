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
#include "common.h"

#define PID_NAK     0
#define PID_BUF     1
#define PID_STALL_1 2
#define PID_STALL_2 3
#define SERIAL_BUFFER_SIZE 1024

unsigned char _rx_buffer[SERIAL_BUFFER_SIZE];
unsigned char _tx_buffer[SERIAL_BUFFER_SIZE];

volatile bool _begin = false;
volatile uint32_t _rx_buffer_head;
volatile uint32_t _rx_buffer_tail;
volatile uint32_t _tx_buffer_head;
volatile uint32_t _tx_buffer_tail;

bool _buffer_available()
{
    return (_tx_buffer_head != _tx_buffer_tail);
}

int _store_char(unsigned char c)
{
  uint32_t i = (uint32_t)(_rx_buffer_head + 1) % SERIAL_BUFFER_SIZE;
  if (i != _rx_buffer_tail) {
    _rx_buffer[_rx_buffer_head] = c;
    _rx_buffer_head = i;
    return 1;
  } else {
    return 0;
  }
}

unsigned char _extract_char()
{
    unsigned char c = _tx_buffer[_tx_buffer_tail];
    if (_tx_buffer_head != _tx_buffer_tail) {
      _tx_buffer_tail = (_tx_buffer_tail + 1) % SERIAL_BUFFER_SIZE;
    }
    return c;
}

void ReadBulkOUTPacket(void)
{
    uint16_t DataLength = 0;

    /*Read data using D1FIFO*/
    /*NOTE: This probably will have already been selected if using BRDY interrupt.*/
    do{
        USB0.D1FIFOSEL.BIT.CURPIPE = PIPE_BULK_OUT;
    }while(USB0.D1FIFOSEL.BIT.CURPIPE != PIPE_BULK_OUT);

    /*Set PID to BUF*/
    USB0.PIPE1CTR.BIT.PID = PID_BUF;

    /*Wait for buffer to be ready*/
    while(USB0.D1FIFOCTR.BIT.FRDY == 0){;}

    /*Set Read Count Mode - so DTLN count will decrement as data read from buffer*/
    USB0.D1FIFOSEL.BIT.RCNT = 1;

    /*Read length of data */
    DataLength = USB0.D1FIFOCTR.BIT.DTLN;

    if( DataLength == 0 ) {
        USB0.D1FIFOCTR.BIT.BCLR = 1;
        return;
    }

    while(DataLength != 0){
        /*Read from the FIFO*/
        uint16_t Data = USB0.D1FIFO.WORD;
        if(DataLength >= 2){
            /*Save first byte*/
            _store_char((uint8_t)Data);
            /*Save second byte*/
            _store_char((uint8_t)(Data>>8));
            DataLength-=2;
        } else {
            _store_char((uint8_t)Data);
            DataLength--;
        }
    }

}
void WriteBulkINPacket(void)
{
    uint32_t Count = 0;

    /*Write data to Bulk IN pipe using D0FIFO*/
    /*Select pipe (Check this happens before continuing)*/
    /*Set 8 bit access*/
    USB0.D0FIFOSEL.BIT.MBW = 0;
    do{
        USB0.D0FIFOSEL.BIT.CURPIPE = PIPE_BULK_IN;
    }while(USB0.D0FIFOSEL.BIT.CURPIPE != PIPE_BULK_IN);


    /*Wait for buffer to be ready*/
    while(USB0.D0FIFOCTR.BIT.FRDY == 0){;}

    /* Write data to the IN Fifo until have written a full packet
     or we have no more data to write */
    while((Count < BULK_IN_PACKET_SIZE) && _buffer_available())
    {
        USB0.D0FIFO.WORD = (unsigned short)_extract_char();
        Count++;
    }

    /*Send the packet */
    /*Set PID to BUF*/
    USB0.PIPE2CTR.BIT.PID = PID_BUF;

    /*If we have not written a full packets worth to the buffer then need to
    signal that the buffer is now ready to be sent, set the buffer valid flag (BVAL).*/
    if(Count != BULK_IN_PACKET_SIZE)
    {
        USB0.D0FIFOCTR.BIT.BVAL = 1;
    }

    if(!_buffer_available())
    {
        USB0.BRDYENB.BIT.PIPE2BRDYE = 0;
    }
}

void usbcdc_write(unsigned char c)
{
    unsigned int i = (_tx_buffer_head + 1) % SERIAL_BUFFER_SIZE;

    if (_begin) {
        if (i != _tx_buffer_tail) {
          USB0.INTENB0.BIT.BRDYE = 0;
          _tx_buffer[_tx_buffer_head] = c;
          _tx_buffer_head = i;
          USB0.INTENB0.BIT.BRDYE = 1;
          USB0.BRDYENB.BIT.PIPE2BRDYE = 1;
        }
    } else {
        if (USBCDC_IsConnected()) {
            _begin = true;
        }
        _tx_buffer[_tx_buffer_head] = c;
        _tx_buffer_head = i;
    }
}

int usbcdc_read(void)
{
  // if the head isn't ahead of the tail, we don't have any characters
  if (_rx_buffer_head == _rx_buffer_tail) {
    return -1;
  } else {
    unsigned char c = _rx_buffer[_rx_buffer_tail];
    _rx_buffer_tail = (uint32_t)(_rx_buffer_tail + 1) % SERIAL_BUFFER_SIZE;
    return c;
  }
}

void usb_init(void)
{
    USB_ERR err = USBCDC_Init();
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
}

void INT_Excep_USB0_USBI0(void)
{
    if(USB0.SYSCFG.BIT.DCFM==0)
    {/* Function controller is selected */
        USBHALInterruptHandler();
    }
    else if(USB0.SYSCFG.BIT.DCFM==1)
    {/* Host controller is selected */
        //InterruptHandler_USBHost();
    }
}

