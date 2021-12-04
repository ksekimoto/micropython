#define MICROPY_HW_BOARD_NAME       "GR-MANGO_BETA"
#define MICROPY_HW_MCU_NAME         "RZA2M"
#define MICROPY_HW_MCU_SYSCLK       528000000
#define MICROPY_HW_MCU_PCLK0        33000000
#define MICROPY_HW_MCU_PCLK1        66000000
#define MICROPY_HW_MCU_PCLK1C       66000000
#define MICROPY_HW_MCU_PCLK         (MICROPY_HW_MCU_PCLK0)

#define MICROPY_HW_HAS_SWITCH       (1)
#define MICROPY_HW_HAS_FLASH        (1)
#define MICROPY_HW_ENABLE_SDCARD    (1)
#define MICROPY_HW_SDCARD_MOUNT_AT_BOOT (1)
#define MICROPY_HW_ENABLE_RTC       (1)
#define MICROPY_HW_ENABLE_SERVO     (1)
#define MICROPY_HW_ENABLE_RZ_USB    (0)
#define MICROPY_HW_ETH_RZ           (1)
#define MICROPY_HW_ENABLE_RNG       (1)

// UART config
#define MICROPY_HW_UART1_RX         (pin_P41)
#define MICROPY_HW_UART1_TX         (pin_P42)
#define MICROPY_HW_UART1_SCK        (pin_P40)
#define MICROPY_HW_UART1_RTS        (pin_P43)
#define MICROPY_HW_UART1_CTS        (pin_P44)
#define MICROPY_HW_UART5_RX         (pin_P91)
#define MICROPY_HW_UART5_TX         (pin_P90)
#define MICROPY_HW_UART_REPL        PYB_UART_5  // SCI4 (P90:TXD4, P91:RXD4)
#define MICROPY_HW_UART_REPL_BAUD   115200

// I2C busses
//#define MICROPY_HW_I2C1_SCL (pin_PD2)
//#define MICROPY_HW_I2C1_SDA (pin_PD3)
//#define MICROPY_HW_I2C2_SCL (pin_PD4)
//#define MICROPY_HW_I2C2_SDA (pin_PD5)

// MMA accelerometer config
//#define MICROPY_HW_MMA_AVDD_PIN     (pin_P43)

// SPI busses
#define MICROPY_HW_SPI1_SCK  (pin_P87)
#define MICROPY_HW_SPI1_MISO (pin_P85)
#define MICROPY_HW_SPI1_MOSI (pin_P86)

// USRSW is pulled low. Pressing the button makes the input go high.
#define MICROPY_HW_USRSW_PIN        (pin_PD7)
#define MICROPY_HW_USRSW_PULL       (GPIO_NOPULL)
#define MICROPY_HW_USRSW_EXTI_MODE  (GPIO_MODE_IT_FALLING)
#define MICROPY_HW_USRSW_PRESSED    (0)
#define MICROPY_HW_USRSW1_PIN        (pin_PD6)
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

// SD card detect switch
#define MICROPY_HW_SDCARD_DETECT_PIN        (pin_P64)
#define MICROPY_HW_SDCARD_DETECT_PULL       (GPIO_PULLUP)
#define MICROPY_HW_SDCARD_DETECT_PRESENT    (0)
#define MICROPY_HW_SDCARD_SPI_CH    (0)
#define MICROPY_HW_SDCARD_SPI_CS    (pin_PC0)
#define MICROPY_HW_SDCARD_SPI_CK    (pin_PC5)
#define MICROPY_HW_SDCARD_SPI_MOSI  (pin_PC6)
#define MICROPY_HW_SDCARD_SPI_MISO  (pin_PC7)

// Add MICRO_PY_LWIP=1 as a parameter of make command when adding LWIP
//#define MICROPY_PY_LWIP (1)
#define MICROPY_HW_ETH_MAC_ADDRESS_0    0x00
#define MICROPY_HW_ETH_MAC_ADDRESS_1    0x0E
#define MICROPY_HW_ETH_MAC_ADDRESS_2    0x2A
#define MICROPY_HW_ETH_MAC_ADDRESS_3    0x07
#define MICROPY_HW_ETH_MAC_ADDRESS_4    0x06
#define MICROPY_HW_ETH_MAC_ADDRESS_5    0x05

#define MICROPY_HW_ENABLE_ADC   (1)
#define MICROPY_HW_ENABLE_DAC   (0)

#define MICROPY_HW_HAS_ESP8266          (1)
#define MICROPY_HW_ESP8266_UART_CH      0
#define MICROPY_HW_ESP8266_UART_BAUD    115200
#define MICROPY_HW_ESP8266_RE           (pin_P44)

#define MICROPY_PY_PYB_FONT         (1)
#define MICROPY_PY_PYB_UNICODE_FONT (1)
#define MICROPY_PY_PYB_LCDSPI       (1)

#define MICROPY_HW_LCDSPI_CLK   (pin_P87)   // GPIO11 Header23
#define MICROPY_HW_LCDSPI_MOSI  (pin_P86)   // GPIO10 Header19
#define MICROPY_HW_LCDSPI_MISO  (pin_P85)   // GPIO9  Header21
#define MICROPY_HW_LCDSPI_CS    (pin_P84)   // GPIO8  Header24
#define MICROPY_HW_LCDSPI_RESET (pin_P45)   // GPIO27 Header13
#define MICROPY_HW_LCDSPI_RS    (pin_PH6)   // GPIO25 Header22

//#define PYB_SERVO_NUM       (1)
//#define BOARD_SERVO1_PIN    (pin_PH0)

#define MICROPY_PY_PYB_CAMERA_DV    (1)
#define MICROPY_PY_PYB_CAMERA_MIPI  (1)

