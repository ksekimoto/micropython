#define MICROPY_HW_BOARD_NAME       "GR-CITRUS"
#define MICROPY_HW_MCU_NAME         "RX631" /* RR5F5631FDDFP */
#define MICROPY_HW_MCU_SYSCLK       96000000
#define MICROPY_HW_MCU_PCLK         48000000

#define MICROPY_HW_HAS_SWITCH           (0)
#define MICROPY_HW_HAS_FLASH            (1)
#define MICROPY_HW_ENABLE_SDCARD        (1)
#define MICROPY_HW_SDCARD_MOUNT_AT_BOOT (0)
#define MICROPY_HW_ENABLE_RTC           (1)
#define MICROPY_HW_ENABLE_SERVO         (1)
#define MICROPY_HW_ENABLE_RX_USB        (1)
#define MICROPY_HW_ETH_MDC              (0)
#define MICROPY_HW_ESP8266              (1)
#define MICROPY_HW_ENABLE_RNG           (1)
#define MICROPY_HW_ENABLE_LCDSPI        (1)
#if MICROPY_HW_ENABLE_LCDSPI
#define MICROPY_HW_ENABLE_LCD_CONSOLE   (0)
#endif

// UART config
#define MICROPY_HW_UART1_TX         (pin_P20)
#define MICROPY_HW_UART1_RX         (pin_P21)
#define MICROPY_HW_UART3_TX         (pin_P50)
#define MICROPY_HW_UART3_RX         (pin_P52)
//#define MICROPY_HW_UART_REPL        PYB_UART_1
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
//#define MICROPY_HW_MMA_AVDD_PIN     (pin_P43)

// SPI busses
#define MICROPY_HW_SPI1_NAME "X"
#define MICROPY_HW_SPI1_NSS  (pin_PC4)
#define MICROPY_HW_SPI1_SCK  (pin_PC5)
#define MICROPY_HW_SPI1_MISO (pin_PC7)
#define MICROPY_HW_SPI1_MOSI (pin_PC6)
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

//#define MICROPY_PY_LWIP (1)
#define MICROPY_HW_ETH_MAC_ADDRESS_0    0x00
#define MICROPY_HW_ETH_MAC_ADDRESS_1    0x0E
#define MICROPY_HW_ETH_MAC_ADDRESS_2    0x2A
#define MICROPY_HW_ETH_MAC_ADDRESS_3    0x04
#define MICROPY_HW_ETH_MAC_ADDRESS_4    0x03
#define MICROPY_HW_ETH_MAC_ADDRESS_5    0x02

#define MICROPY_HW_ENABLE_DAC   (1)

#if MICROPY_HW_ESP8266
#define MICROPY_HW_HAS_ESP8266      (0)
#define MICROPY_HW_ESP8266_UART_CH      7   // CH=6
#define MICROPY_HW_ESP8266_UART_BAUD    115200
#define MICROPY_HW_ESP8266_EN           (pin_P50)
#endif

#define MICROPY_PY_PYB_TWITTER      (0)
#if MICROPY_HW_ENABLE_LCDSPI
#define MICROPY_PY_PYB_LCDSPI       (1)
#endif
#define MICROPY_PY_PYB_FONT         (1)
#define MICROPY_PY_PYB_UNICODE_FONT (1)

#if MICROPY_HW_ENABLE_LCDSPI
#include "lcdspi.h"
#define MICROPY_HW_LCDSPI_ID    (AIDEEPEN22SPI)
#define MICROPY_HW_LCDSPI_DIR   (LCDSPI_ROTATE_0)
#define MICROPY_HW_LCDSPI_CH    (1)
#define MICROPY_HW_LCDSPI_CLK   (pin_PC5)
#define MICROPY_HW_LCDSPI_MOSI  (pin_PC6)
#define MICROPY_HW_LCDSPI_MISO  (pin_PC7)
#define MICROPY_HW_LCDSPI_CS    (pin_PC4)
#define MICROPY_HW_LCDSPI_RESET (pin_P52)
#define MICROPY_HW_LCDSPI_RS    (pin_P50)
#endif

#if MICROPY_HW_ENABLE_LCD_CONSOLE
#include "lcdspi.h"
#define MICROPY_HW_LCDSPI_CON_ID    (AIDEEPEN22SPI)
#define MICROPY_HW_LCDSPI_CON_DIR   (LCDSPI_ROTATE_0)
#define MICROPY_HW_LCDSPI_CON_CH    (1)
#define MICROPY_HW_LCDSPI_CON_CLK   (pin_PC5)
#define MICROPY_HW_LCDSPI_CON_MOSI  (pin_PC6)
#define MICROPY_HW_LCDSPI_CON_MISO  (pin_PC7)
#define MICROPY_HW_LCDSPI_CON_CS    (pin_PC4)
#define MICROPY_HW_LCDSPI_CON_RESET (pin_P52)
#define MICROPY_HW_LCDSPI_CON_RS    (pin_P50)
#endif

#define PYB_SERVO_NUM   (6)
#define BOARD_SERVO1_PIN    (pin_P13)
#define BOARD_SERVO2_PIN    (pin_P15)
#define BOARD_SERVO3_PIN    (pin_P32)
#define BOARD_SERVO4_PIN    (pin_P33)
#define BOARD_SERVO5_PIN    (pin_PB3)
#define BOARD_SERVO6_PIN    (pin_PB5)

#define MICROPY_PY_PYB_LEGACY   1   // if flash rom is short, shoule be 0
