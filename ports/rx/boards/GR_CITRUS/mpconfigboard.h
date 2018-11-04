#define MICROPY_HW_BOARD_NAME       "GR-CITRUS"
#define MICROPY_HW_MCU_NAME         "RX631" /* RR5F5631FDDFP */

#define MICROPY_HW_HAS_SWITCH       (0)
#define MICROPY_HW_HAS_FLASH        (1)
#define MICROPY_HW_HAS_SDCARD       (1)
#define MICROPY_HW_ENABLE_RTC       (0)
#define MICROPY_HW_ENABLE_RX_USB    (1)

// UART config
#define MICROPY_HW_UART0_TX         (pin_P20)
#define MICROPY_HW_UART0_RX         (pin_P21)
#define MICROPY_HW_UART2_TX         (pin_P50)
#define MICROPY_HW_UART2_RX         (pin_P52)
#define MICROPY_HW_UART_REPL        PYB_UART_0
#define MICROPY_HW_UART_REPL_BAUD   115200

// I2C busses
//#define MICROPY_HW_I2C0_SCL (pin_P13)
//#define MICROPY_HW_I2C0_SDA (pin_P12)
//#define MICROPY_HW_I2C1_SCL (pin_P20)
//#define MICROPY_HW_I2C1_SDA (pin_P21)
//#define MICROPY_HW_I2C2_SCL (pin_PC4)
//#define MICROPY_HW_I2C2_SDA (pin_PC6)
//#define MICROPY_HW_I2C3_SCL (pin_P50)
//#define MICROPY_HW_I2C3_SDA (pin_P52)
//#define MICROPY_HW_I2C4_SCL (pin_PC7)
//#define MICROPY_HW_I2C4_SDA (pin_PC6)

// MMA accelerometer config
#define MICROPY_HW_MMA_AVDD_PIN     (pin_P43)

// SPI busses
#define MICROPY_HW_SPI0_NAME "X"
#define MICROPY_HW_SPI0_NSS  (pin_PC4)
#define MICROPY_HW_SPI0_SCK  (pin_PC5)
#define MICROPY_HW_SPI0_MISO (pin_PC7)
#define MICROPY_HW_SPI0_MOSI (pin_PC6)
// USRSW is pulled low. Pressing the button makes the input go high.
//#define MICROPY_HW_USRSW_PIN        (pin_PA7)
//#define MICROPY_HW_USRSW_PULL       (GPIO_NOPULL)
//#define MICROPY_HW_USRSW_EXTI_MODE  (GPIO_MODE_IT_FALLING)
//#define MICROPY_HW_USRSW_PRESSED    (0)

// LEDs
#define MICROPY_HW_LED1             (pin_PA0)
#define MICROPY_HW_LED_ON(pin)      mp_hal_pin_high(pin)
#define MICROPY_HW_LED_OFF(pin)     mp_hal_pin_low(pin)
#define MICROPY_HW_LED_TOGGLE(pin)  mp_hal_pin_toggle(pin)
// SD card detect switch
#define MICROPY_HW_SDCARD_DETECT_PIN        (pin_P13)
#define MICROPY_HW_SDCARD_DETECT_PULL       (GPIO_PULLUP)
#define MICROPY_HW_SDCARD_DETECT_PRESENT    (0)
#define MICROPY_HW_SDCARD_SPI_CH    (0)
//#define MICROPY_HW_SDCARD_SPI_CS    (pin_PC0)   /* Default */
#define MICROPY_HW_SDCARD_SPI_CS    (pin_PC2)   /* Jtag configuration */
#define MICROPY_HW_SDCARD_SPI_CK    (pin_PC5)
#define MICROPY_HW_SDCARD_SPI_MOSI  (pin_PC6)
#define MICROPY_HW_SDCARD_SPI_MISO  (pin_PC7)
