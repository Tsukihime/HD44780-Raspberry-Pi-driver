// http://en.wikipedia.org/wiki/Hitachi_HD44780_LCD_controller
#ifndef HD44780_H
#define HD44780_H

#include <bcm2835.h>

// соответствия контактов GPIO и LCD
//#define HD44780_DB0
//#define HD44780_DB1
//#define HD44780_DB2
//#define HD44780_DB3
#define HD44780_DB4 RPI_V2_GPIO_P1_11
#define HD44780_DB5 RPI_V2_GPIO_P1_12
#define HD44780_DB6 RPI_V2_GPIO_P1_13
#define HD44780_DB7 RPI_V2_GPIO_P1_15
#define HD44780_RS RPI_V2_GPIO_P1_03
#define HD44780_RW RPI_V2_GPIO_P1_05
#define HD44780_E RPI_V2_GPIO_P1_07

// выбор битности интерфейса
#define HD44780_4_BIT
//#define HD44780_8_BIT

// определения команд
#define INIT_COMM	0b00100011
#define CLR_SCR	0b00000001
#define DISPL_ENABLE	0b00001100
#define LINE_2x16_MODE	0b00001000
#define SET_CGRAM_ADDR	0b01000000
#define SET_DDRAM_ADDR	0b10000000

void HD44780_goto(int row, int col);
void HD44780_write_CGRAM(char* char_pic, char char_code);
int HD44780_init();
void HD44780_clear_scr();
void HD44780_put_char(char c);

#endif
