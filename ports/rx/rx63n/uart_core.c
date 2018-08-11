#include <unistd.h>
#include "py/mpconfig.h"
#include "common.h"
#include "rx63n_sci.h"

/*
 * Core UART functions to implement for a port
 */

// Receive single character
int mp_hal_stdin_rx_chr(void) {
    unsigned char c = 0;
    c = SCI_Rx(SCI_CH);
    return (int)c;
}

// Send string of given length
void mp_hal_stdout_tx_strn(const char *str, mp_uint_t len) {
    while (len--) {
        SCI_Tx(SCI_CH, *str++);
    }
}
