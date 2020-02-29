#include "mbed.h"
#include "Serial.h"

Serial pc(UART_TXD, UART_RXD);

// Implement the micropython HAL I/O functions
extern "C" void mp_hal_stdout_tx_chr(int c);
void mp_hal_stdout_tx_chr(int c) {
    pc.putc(c);
}
extern "C" int mp_hal_stdin_rx_chr(void);
int mp_hal_stdin_rx_chr(void) {
    int c = pc.getc();
    return c;
}

extern "C" void mp_hal_stdout_tx_strn(const char *str, int len);
void mp_hal_stdout_tx_strn(const char *str, int len) {
    while (len--) {
        pc.putc(((int)*str++) & 0xff);
    }
}
