#include "pin.h"

#define MP_HAL_PIN_FMT                  "%q"
#define MP_HAL_PIN_MODE_INPUT           (0)
#define MP_HAL_PIN_MODE_OUTPUT          (1)
#define MP_HAL_PIN_MODE_ALT             (2)
#define MP_HAL_PIN_MODE_ANALOG          (3)
#define MP_HAL_PIN_MODE_ADC             (3)
#define MP_HAL_PIN_MODE_OPEN_DRAIN      (5)
#define MP_HAL_PIN_MODE_ALT_OPEN_DRAIN  (6)
#define MP_HAL_PIN_PULL_NONE            (GPIO_NOPULL)
#define MP_HAL_PIN_PULL_UP              (GPIO_PULLUP)
#define MP_HAL_PIN_PULL_DOWN            (GPIO_PULLDOWN)

#define mp_hal_pin_obj_t const pin_obj_t*
#define mp_hal_get_pin_obj(o)
#define mp_hal_pin_name(p)      ((p)->name)
#define mp_hal_pin_input(p)     gpio_mode_input((p)->pin)
#define mp_hal_pin_output(p)    gpio_mode_output((p)->pin)
#define mp_hal_pin_open_drain(p)
#define mp_hal_pin_high(p)      gpio_write((p)->pin, 1)
#define mp_hal_pin_low(p)       gpio_write((p)->pin, 0)
#define mp_hal_pin_od_low(p)    mp_hal_pin_low(p)
#define mp_hal_pin_od_high(p)   mp_hal_pin_high(p)
#define mp_hal_pin_read(p)      gpio_read((p)->pin)
#define mp_hal_pin_write(p, v)  do { if (v) { mp_hal_pin_high(p); } else { mp_hal_pin_low(p); } } while (0)

void mp_hal_gpio_clock_enable();
void mp_hal_pin_config(mp_hal_pin_obj_t pin, uint32_t mode, uint32_t pull, uint32_t alt);
bool mp_hal_pin_config_alt(mp_hal_pin_obj_t pin, uint32_t mode, uint32_t pull, uint8_t fn, uint8_t unit);
void mp_hal_pin_config_speed(mp_hal_pin_obj_t pin_obj, uint32_t speed);

