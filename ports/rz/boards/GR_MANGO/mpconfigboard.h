#define MICROPY_HW_BOARD_NAME       "GR-MANGO"
#define MICROPY_HW_MCU_NAME         "RZA2M"
#define MICROPY_HW_MCU_SYSCLK       528000000
#define MICROPY_HW_MCU_PCLK0        33000000
#define MICROPY_HW_MCU_PCLK1        66000000
#define MICROPY_HW_MCU_PCLK1C       66000000
#define MICROPY_HW_MCU_PCLK         (MICROPY_HW_MCU_PCLK0)

#define MICROPY_HW_HAS_SWITCH       (0)
#define MICROPY_HW_HAS_FLASH        (0)
#define MICROPY_HW_ENABLE_SDCARD    (0)
#define MICROPY_HW_SDCARD_MOUNT_AT_BOOT (0)
#define MICROPY_HW_ENABLE_RTC       (0)
#define MICROPY_HW_ENABLE_SERVO     (0)
#define MICROPY_HW_ENABLE_RZ_USB    (0)
#define MICROPY_HW_ETH_RZ           (0)

// UART config
#define MICROPY_HW_UART0_RX         (pin_P41)
#define MICROPY_HW_UART0_TX         (pin_P42)
#define MICROPY_HW_UART0_SCK        (pin_P40)
#define MICROPY_HW_UART0_RTS        (pin_P43)
#define MICROPY_HW_UART0_CTS        (pin_P44)
#define MICROPY_HW_UART4_RX         (pin_P45)
#define MICROPY_HW_UART4_TX         (pin_P46)
#define MICROPY_HW_UART4_SCK        (pin_P47)
//#define MICROPY_HW_UART_REPL        PYB_UART_1  // Serial5 (P26:TX1, P30:RX1)
#define MICROPY_HW_UART_REPL_BAUD   115200

// I2C busses
#define MICROPY_HW_I2C1_SCL (pin_PD2)
#define MICROPY_HW_I2C1_SDA (pin_PD3)
#define MICROPY_HW_I2C2_SCL (pin_PD4)
#define MICROPY_HW_I2C2_SDA (pin_PD5)

// MMA accelerometer config
//#define MICROPY_HW_MMA_AVDD_PIN     (pin_P43)

// SPI busses
#define MICROPY_HW_SPI0_SCK  (pin_P87)
#define MICROPY_HW_SPI0_MISO (pin_P85)
#define MICROPY_HW_SPI0_MOSI (pin_P86)

// USRSW is pulled low. Pressing the button makes the input go high.
#define MICROPY_HW_USRSW0_PIN        (pin_PD6)
#define MICROPY_HW_USRSW0_PULL       (GPIO_NOPULL)
#define MICROPY_HW_USRSW0_EXTI_MODE  (GPIO_MODE_IT_FALLING)
#define MICROPY_HW_USRSW0_PRESSED    (0)
#define MICROPY_HW_USRSW1_PIN        (pin_PD7)
#define MICROPY_HW_USRSW1_PULL       (GPIO_NOPULL)
#define MICROPY_HW_USRSW1_EXTI_MODE  (GPIO_MODE_IT_FALLING)
#define MICROPY_HW_USRSW1_PRESSED    (0)

// LEDs
#define MICROPY_HW_LED1             (pin_P01)
#define MICROPY_HW_LED2             (pin_P03)
#define MICROPY_HW_LED3             (pin_P05)
#define MICROPY_HW_LED4             (pin_P82)
#define MICROPY_HW_LED_ON(pin)      mp_hal_pin_high(pin)
#define MICROPY_HW_LED_OFF(pin)     mp_hal_pin_low(pin)
#define MICROPY_HW_LED_TOGGLE(pin)  mp_hal_pin_toggle(pin)

#define MICROPY_HW_ENABLE_DAC   (0)

