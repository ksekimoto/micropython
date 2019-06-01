#define MICROPY_HW_BOARD_NAME       "GR-SAKURA"
#define MICROPY_HW_MCU_NAME         "RX63N" /* R5F563NBDDF */
#define MICROPY_HW_MCU_SYSCLK       96000000
#define MICROPY_HW_MCU_PCLK         48000000

#define MICROPY_HW_HAS_SWITCH       (1)
#define MICROPY_HW_HAS_FLASH        (1)
#define MICROPY_HW_ENABLE_SDCARD    (0)
#define MICROPY_HW_SDCARD_MOUNT_AT_BOOT (0)
#define MICROPY_HW_ENABLE_RTC       (1)
#define MICROPY_HW_ENABLE_SERVO     (1)
#define MICROPY_HW_ENABLE_RX_USB    (1)
#define MICROPY_HW_ETH_RX           (1)

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
//#define MICROPY_HW_UART_REPL        PYB_UART_0
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
#define MICROPY_HW_SPI2_NAME "Y"
#define MICROPY_HW_SPI2_NSS  (pin_PE4)
#define MICROPY_HW_SPI2_SCK  (pin_PE1)
#define MICROPY_HW_SPI2_MISO (pin_PE3)
#define MICROPY_HW_SPI2_MOSI (pin_PE2)
#define MICROPY_HW_SPI3_NAME "Z"
#define MICROPY_HW_SPI3_NSS  (pin_PD4)
#define MICROPY_HW_SPI3_SCK  (pin_PD3)
#define MICROPY_HW_SPI3_MISO (pin_PD2)
#define MICROPY_HW_SPI3_MOSI (pin_PD1)

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
// SD card detect switch
#define MICROPY_HW_SDCARD_DETECT_PIN        (pin_P15)
#define MICROPY_HW_SDCARD_DETECT_PULL       (GPIO_PULLUP)
#define MICROPY_HW_SDCARD_DETECT_PRESENT    (0)
#define MICROPY_HW_SDCARD_SPI_CH    (0)
#define MICROPY_HW_SDCARD_SPI_CS    (pin_PC0)
#define MICROPY_HW_SDCARD_SPI_CK    (pin_PC5)
#define MICROPY_HW_SDCARD_SPI_MOSI  (pin_PC6)
#define MICROPY_HW_SDCARD_SPI_MISO  (pin_PC7)

// Add MICRO_PY_LWIP=1 as a parameter of make command when adding LWIP
//#define MICROPY_PY_LWIP (1)

#define MICROPY_HW_ENABLE_DAC   (1)

#define MICROPY_HW_HAS_ESP8266      (0)
#define MICROPY_PY_PYB_TWITTER      (0)
#define MICROPY_PY_PYB_UNICODE_FONT (0)
#define MICROPY_PY_PYB_LCDSPI       (1)

#define PYB_SERVO_NUM   (6)
#define BOARD_SERVO1_PIN    (pin_P13)
#define BOARD_SERVO2_PIN    (pin_PC4)
#define BOARD_SERVO3_PIN    (pin_P22)
#define BOARD_SERVO4_PIN    (pin_P23)
#define BOARD_SERVO5_PIN    (pin_P24)
#define BOARD_SERVO6_PIN    (pin_P25)

