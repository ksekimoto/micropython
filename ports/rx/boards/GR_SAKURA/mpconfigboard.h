#define MICROPY_HW_BOARD_NAME       "GR-SAKURA"
#define MICROPY_HW_MCU_NAME         "RX63N" /* R5F563NBDDF */

#define MICROPY_HW_HAS_SWITCH       (1)
#define MICROPY_HW_HAS_FLASH        (1)
#define MICROPY_HW_ENABLE_RTC       (0)

// UART config
#define MICROPY_HW_UART0_TX         (pin_P20)
#define MICROPY_HW_UART0_RX         (pin_P21)
#define MICROPY_HW_UART1_TX         (pin_P16)
#define MICROPY_HW_UART1_RX         (pin_P15)
#define MICROPY_HW_UART2_TX         (pin_P50)
#define MICROPY_HW_UART2_RX         (pin_P52)
#define MICROPY_HW_UART3_TX         (pin_P23)
#define MICROPY_HW_UART3_RX         (pin_P25)
#define MICROPY_HW_UART5_TX         (pin_PC3)
#define MICROPY_HW_UART5_RX         (pin_PC2)
#define MICROPY_HW_UART6_TX         (pin_P33)
#define MICROPY_HW_UART6_RX         (pin_P32)
#define MICROPY_HW_UART_REPL        PYB_UART_0
#define MICROPY_HW_UART_REPL_BAUD   115200

// USRSW is pulled low. Pressing the button makes the input go high.
#define MICROPY_HW_USRSW_PIN        (pin_PA7)
#define MICROPY_HW_USRSW_PULL       (GPIO_NOPULL)
#define MICROPY_HW_USRSW_EXTI_MODE  (GPIO_MODE_IT_FALLING)
#define MICROPY_HW_USRSW_PRESSED    (0)

// LEDs
#define MICROPY_HW_LED1             (pin_PA0)
#define MICROPY_HW_LED2             (pin_PA1)
#define MICROPY_HW_LED3             (pin_PA2)
#define MICROPY_HW_LED4             (pin_PA6)
#define MICROPY_HW_LED_ON(pin)      mp_hal_pin_high(pin)
#define MICROPY_HW_LED_OFF(pin)     mp_hal_pin_low(pin)
#define MICROPY_HW_LED_TOGGLE(pin)  mp_hal_pin_toggle(pin)
