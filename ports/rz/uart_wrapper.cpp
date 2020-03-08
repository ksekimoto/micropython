#include "mbed.h"

extern "C" void mp_hal_stdout_tx_chr(int c);
extern "C" int mp_hal_stdin_rx_chr(void);
extern "C" void mp_hal_stdout_tx_strn(const char *str, int len);

#define MBED_CONSOLE

#if !defined(MBED_CONSOLE)
#include "USBSerial.h"
USBSerial pc;

void mp_hal_stdout_tx_chr(int c) {
    pc._putc(c);
}

int mp_hal_stdin_rx_chr(void) {
    int c = -1;
    //if (pc.readable()) {
        c = pc._getc();
    //}
    return c;
}

void mp_hal_stdout_tx_strn(const char *str, int len) {
    while (len--) {
        pc._putc(((int)*str++) & 0xff);
    }
}

#else
#include "Serial.h"
Serial pc(USBTX, USBRX);

void mp_hal_stdout_tx_chr(int c) {
    pc.putc(c);
}

int mp_hal_stdin_rx_chr(void) {
    return pc.getc();
}

void mp_hal_stdout_tx_strn(const char *str, int len) {
    while (len--) {
        pc.putc(((int)*str++) & 0xff);
    }
}

#endif
