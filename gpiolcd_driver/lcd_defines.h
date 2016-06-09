// соответствия контактов GPIO и LCD
#define HD44780_DB4 RPI_V2_GPIO_P1_11
#define HD44780_DB5 RPI_V2_GPIO_P1_12
#define HD44780_DB6 RPI_V2_GPIO_P1_13
#define HD44780_DB7 RPI_V2_GPIO_P1_15
#define HD44780_RS RPI_V2_GPIO_P1_03
#define HD44780_RW RPI_V2_GPIO_P1_05
#define HD44780_E RPI_V2_GPIO_P1_07


// For RPI v1 use RPI_GPIO_P1_** consts

// this text from bcm2835.h

/// \brief GPIO Pin Numbers
///
/// Here we define Raspberry Pin GPIO pins on P1 in terms of the underlying BCM GPIO pin numbers.
/// These can be passed as a pin number to any function requiring a pin.
/// Not all pins on the RPi 26 bin IDE plug are connected to GPIO pins
/// and some can adopt an alternate function.
/// RPi version 2 has some slightly different pinouts, and these are values RPI_V2_*.
/// At bootup, pins 8 and 10 are set to UART0_TXD, UART0_RXD (ie the alt0 function) respectively
/// When SPI0 is in use (ie after bcm2835_spi_begin()), pins 19, 21, 23, 24, 26 are dedicated to SPI
/// and cant be controlled independently
typedef enum
{
    RPI_GPIO_P1_03        =  0,  ///< Version 1, Pin P1-03
    RPI_GPIO_P1_05        =  1,  ///< Version 1, Pin P1-05
    RPI_GPIO_P1_07        =  4,  ///< Version 1, Pin P1-07
    RPI_GPIO_P1_08        = 14,  ///< Version 1, Pin P1-08, defaults to alt function 0 UART0_TXD
    RPI_GPIO_P1_10        = 15,  ///< Version 1, Pin P1-10, defaults to alt function 0 UART0_RXD
    RPI_GPIO_P1_11        = 17,  ///< Version 1, Pin P1-11
    RPI_GPIO_P1_12        = 18,  ///< Version 1, Pin P1-12, can be PWM channel 0 in ALT FUN 5
    RPI_GPIO_P1_13        = 21,  ///< Version 1, Pin P1-13
    RPI_GPIO_P1_15        = 22,  ///< Version 1, Pin P1-15
    RPI_GPIO_P1_16        = 23,  ///< Version 1, Pin P1-16
    RPI_GPIO_P1_18        = 24,  ///< Version 1, Pin P1-18
    RPI_GPIO_P1_19        = 10,  ///< Version 1, Pin P1-19, MOSI when SPI0 in use
    RPI_GPIO_P1_21        =  9,  ///< Version 1, Pin P1-21, MISO when SPI0 in use
    RPI_GPIO_P1_22        = 25,  ///< Version 1, Pin P1-22
    RPI_GPIO_P1_23        = 11,  ///< Version 1, Pin P1-23, CLK when SPI0 in use
    RPI_GPIO_P1_24        =  8,  ///< Version 1, Pin P1-24, CE0 when SPI0 in use
    RPI_GPIO_P1_26        =  7,  ///< Version 1, Pin P1-26, CE1 when SPI0 in use

    // RPi Version 2
    RPI_V2_GPIO_P1_03     =  2,  ///< Version 2, Pin P1-03
    RPI_V2_GPIO_P1_05     =  3,  ///< Version 2, Pin P1-05
    RPI_V2_GPIO_P1_07     =  4,  ///< Version 2, Pin P1-07
    RPI_V2_GPIO_P1_08     = 14,  ///< Version 2, Pin P1-08, defaults to alt function 0 UART0_TXD
    RPI_V2_GPIO_P1_10     = 15,  ///< Version 2, Pin P1-10, defaults to alt function 0 UART0_RXD
    RPI_V2_GPIO_P1_11     = 17,  ///< Version 2, Pin P1-11
    RPI_V2_GPIO_P1_12     = 18,  ///< Version 2, Pin P1-12, can be PWM channel 0 in ALT FUN 5
    RPI_V2_GPIO_P1_13     = 27,  ///< Version 2, Pin P1-13
    RPI_V2_GPIO_P1_15     = 22,  ///< Version 2, Pin P1-15
    RPI_V2_GPIO_P1_16     = 23,  ///< Version 2, Pin P1-16
    RPI_V2_GPIO_P1_18     = 24,  ///< Version 2, Pin P1-18
    RPI_V2_GPIO_P1_19     = 10,  ///< Version 2, Pin P1-19, MOSI when SPI0 in use
    RPI_V2_GPIO_P1_21     =  9,  ///< Version 2, Pin P1-21, MISO when SPI0 in use
    RPI_V2_GPIO_P1_22     = 25,  ///< Version 2, Pin P1-22
    RPI_V2_GPIO_P1_23     = 11,  ///< Version 2, Pin P1-23, CLK when SPI0 in use
    RPI_V2_GPIO_P1_24     =  8,  ///< Version 2, Pin P1-24, CE0 when SPI0 in use
    RPI_V2_GPIO_P1_26     =  7,  ///< Version 2, Pin P1-26, CE1 when SPI0 in use

    // RPi Version 2, new plug P5
    RPI_V2_GPIO_P5_03     = 28,  ///< Version 2, Pin P5-03
    RPI_V2_GPIO_P5_04     = 29,  ///< Version 2, Pin P5-04
    RPI_V2_GPIO_P5_05     = 30,  ///< Version 2, Pin P5-05
    RPI_V2_GPIO_P5_06     = 31,  ///< Version 2, Pin P5-06

} RPiGPIOPin; 
