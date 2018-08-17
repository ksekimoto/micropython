#include "py/obj.h"
#include "pin.h"

// Returns the pin mode. This value returned by this macro should be one of:
// GPIO_MODE_INPUT, GPIO_MODE_OUTPUT_PP, GPIO_MODE_OUTPUT_OD,
// GPIO_MODE_AF_PP, GPIO_MODE_AF_OD, or GPIO_MODE_ANALOG.

uint32_t pin_get_mode(const pin_obj_t *pin) {
    uint32_t mode = 0;
    return mode;
}

// Returns the pin pullup/pulldown. The value returned by this macro should
// be one of GPIO_NOPULL, GPIO_PULLUP, or GPIO_PULLDOWN.

uint32_t pin_get_pull(const pin_obj_t *pin) {
    return 1;
}

// Returns the af (alternate function) index currently set for a pin.

uint32_t pin_get_af(const pin_obj_t *pin) {
    return 1;
}

