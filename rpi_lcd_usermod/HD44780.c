#include "HD44780.h"
#include <bcm2835.h>

unsigned char CP1251_TO_CPHD44780[0x60] =
{
0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,/*Ё*/0xA2,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,/*ё*/0xB5,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
// ---- data ---- 'А' - 'я'
0x41,0xA0,0x42,0xA1,0xE0,0x45,0xA3,0xA4,0xA5,0xA6,0x4B,0xA7,0x4D,0x48,0x4F,0xA8,
0x50,0x43,0x54,0xA9,0xAA,0x58,0xE1,0xAB,0xAC,0xE2,0xAD,0xAE,0xAD,0xAF,0xB0,0xB1,
0x61,0xB2,0xB3,0xB4,0xE3,0x65,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0x6F,0xBE,
0x70,0x63,0xBF,0x79,0xE4,0x78,0xE5,0xC0,0xC1,0xE6,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7
};

void HD44780_SetPin(int pin, int val)
{
	bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP); // настроим порт на запись
	if (val)
		bcm2835_gpio_write(pin, HIGH);
	else
		bcm2835_gpio_write(pin, LOW);
}

int HD44780_GetPin(int pin)
{
    // ворнинг: в моей схеме экран питается от 5 вольт, 
    // а малина на входе ждёт 3.3в 
    // чтобы читать экран нужно прикрутить к экрану преобразователь уровней
    // иначе пипец малине
	return 0; // stub 
}

void HD44780_Strobe() // синхроимпульс
{
	HD44780_SetPin(HD44780_E, 1);
	//bcm2835_delay(1); // 1200 ns = 0,0012ms ~ 1ms ;)
	HD44780_SetPin(HD44780_E, 0);
	bcm2835_delay(2);// Wait for more than 1.53ms - самая долгая команда(очистка дисплея)	
}

void HD44780_Write(char data)
{
	HD44780_SetPin(HD44780_RW, 0); // запись в дисплей (RW=0)	
#ifdef HD44780_4_BIT // используем только первые 4 бита из data
	HD44780_SetPin(HD44780_DB7, (data & 0x08));
	HD44780_SetPin(HD44780_DB6, (data & 0x04));
	HD44780_SetPin(HD44780_DB5, (data & 0x02));
	HD44780_SetPin(HD44780_DB4, (data & 0x01));
#else // иначе все 8
	HD44780_SetPin(HD44780_DB7, (data & 0x80));
	HD44780_SetPin(HD44780_DB6, (data & 0x40));
	HD44780_SetPin(HD44780_DB5, (data & 0x20));
	HD44780_SetPin(HD44780_DB4, (data & 0x10));
	HD44780_SetPin(HD44780_DB3, (data & 0x08));
	HD44780_SetPin(HD44780_DB2, (data & 0x04));
	HD44780_SetPin(HD44780_DB1, (data & 0x02));
	HD44780_SetPin(HD44780_DB0, (data & 0x01));		
#endif
}

void HD44780_SendByte(char data)
{
#ifdef HD44780_4_BIT // в 4битном режиме за 2 захода
	HD44780_Write(data >> 4);
	HD44780_Strobe();
#endif
	HD44780_Write(data);
	HD44780_Strobe();
}

void HD44780_SendCommand(char data)
{
	HD44780_SetPin(HD44780_RS, 0); // передаём команду (RS=0)
	HD44780_SendByte(data);
}

void HD44780_SendData(char data)
{
	HD44780_SetPin(HD44780_RS, 1); // передаём данные (RS=1)
	HD44780_SendByte(data);
}

void HD44780_goto(int row, int col)
{
	//1 AD AD AD AD AD AD AD   Переключить адресацию на DDRAM и задать адрес в DDRAM
	char cmd = SET_DDRAM_ADDR | (0x40 * row + col); // 0x40 - адрес второй строки
	HD44780_SendCommand(cmd);
}

void HD44780_write_CGRAM(char* char_pic, char char_code)
{
	char cmd = (SET_CGRAM_ADDR | char_code * 8); // 8 - char height
	HD44780_SendCommand(cmd);
	int i;
	for(i = 0; i < 8; ++i)
		HD44780_SendData(char_pic[i]);
	//HD44780_goto(0,0);
}

int HD44780_init()
{
	HD44780_SetPin(HD44780_RS, 0); // передаём команду (RS=0)
#ifdef HD44780_4_BIT
	HD44780_Write(INIT_COMM); // 0011
	HD44780_Strobe();
	HD44780_Write(INIT_COMM >> 4); // 0010
	HD44780_Strobe();
	HD44780_Strobe(); // и ещё раз	
	HD44780_Write(LINE_2x16_MODE); //1000
	HD44780_Strobe();
#else

#endif
	HD44780_SendCommand(DISPL_ENABLE); // Включили дисплей (D=1) // 1100
}

void HD44780_clear_scr() //  Очистка экрана
{
	HD44780_SendCommand(CLR_SCR);
}

void HD44780_put_char(char c)
{
    if(c >= 0xA0)
    {
	    c = CP1251_TO_CPHD44780[c]; // перевели кодировку CP1251 в экранную
    }
	HD44780_SendData(c);
}
