//#include <linux/moduleparam.h>
//#include <linux/sched.h>
//#include <linux/kernel.h>	/* printk() */
//#include <linux/errno.h>	/* error codes */
//#include <linux/kdev_t.h>
//#include <linux/slab.h>
//#include <linux/mm.h>
//#include <linux/ioport.h>
//#include <linux/interrupt.h>
//#include <linux/workqueue.h>
//#include <linux/poll.h>
//#include <linux/wait.h>


#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>		/* everything... */

#include <asm/io.h>
#include <asm/gpio.h>
#include <linux/unistd.h>
#include <linux/delay.h>	/* udelay */
#include <asm/uaccess.h>
#include <linux/miscdevice.h>

#include "lcd.h"
#include "charmap.h"
#include "lcd_defines.h"

//#define MY_WRITE


#define LCD_STRINGS	2
#define LCD_COLUMNS	16

//YWROBOT
//last updated on 21/12/2011
//Tim Starling Fix the reset bug (Thanks Tim)
//wiki doc http://www.dfrobot.com/wiki/index.php?title=I2C/TWI_LCD1602_Module_(SKU:_DFR0063)
//Support Forum: http://www.dfrobot.com/forum/
//Compatible with the Arduino IDE 1.0
//Library version:1.1
//converted to c and ported to x86 by Dolin Sergey dolin@gmail.com


// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1
//    S = 0; No shift 
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).


static inline int delayMicroseconds(int value)
{
	//usleep(value); //rootfs
	if (value > 1000)
		msleep(value/1000);
	udelay(value%1000);
	return 0; //kernel
}

void LCD_init(unsigned char lcd_Addr,unsigned char lcd_cols,unsigned char lcd_rows)
{
	_Addr = lcd_Addr;
	_cols = lcd_cols;
	_rows = lcd_rows;
	_backlightval = LCD_NOBACKLIGHT;
	
	init_priv();
	command(1);  

	backlight();
	clear();
	display();
	clear();
}

void init(){
	init_priv();
}

void init_priv()
{
	_displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
	begin(_cols, _rows);  
}

void begin(unsigned char cols, unsigned char lines) 
{
	unsigned char dotsize = LCD_5x8DOTS;
	if (lines > 1) {
		_displayfunction |= LCD_2LINE;
	}
	_numlines = lines;

	// for some 1 line displays you can select a 10 pixel high font
	if ((dotsize != 0) && (lines == 1)) {
		_displayfunction |= LCD_5x10DOTS;
	}

	// SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
	// according to datasheet, we need at least 40ms after power rises above 2.7V
	// before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
	//delay(50); 
	 delayMicroseconds(50000);
	// Now we pull both RS and R/W low to begin commands
	expanderWrite(_backlightval);	// reset expanderand turn backlight off (Bit 8 =1)
	//delay(1000);
	 delayMicroseconds(1000);
  	//put the LCD into 4 bit mode
	// this is according to the hitachi HD44780 datasheet
	// figure 24, pg 46
	
	  // we start in 8bit mode, try to set 4 bit mode
	write4bits(0x03 << 4);
	delayMicroseconds(4500); // wait min 4.1ms
	// second try
	write4bits(0x03 << 4);
	delayMicroseconds(4500); // wait min 4.1ms
	// third go!
	write4bits(0x03 << 4); 
	delayMicroseconds(150);
  
	// finally, set to 4-bit interface
	write4bits(0x02 << 4); 

	// set # lines, font size, etc.
	command(LCD_FUNCTIONSET | _displayfunction);  
	
	// turn the display on with no cursor or blinking default
	_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	display();
	// clear it off
	clear();
	
	// Initialize to default text direction (for roman languages)
	_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
	
	// set the entry mode
	command(LCD_ENTRYMODESET | _displaymode);
	home();
  
}

void write_l(unsigned char value) {
	send(value, Rs);
}

/********** high level commands, for the user! */
void clear(){
	command(LCD_CLEARDISPLAY);// clear display, set cursor position to zero
	delayMicroseconds(2000);  // this command takes a long time!
}

void home(){
	command(LCD_RETURNHOME);  // set cursor position to zero
	delayMicroseconds(2000);  // this command takes a long time!
}

void setCursor(unsigned char col, unsigned char row){
	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
    if ( row >= _numlines ) {
		row = _numlines-1;    // we count rows starting w/0
	}
	command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void noDisplay() {
	_displaycontrol &= ~LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void display() {
	_displaycontrol |= LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void noCursor() {
	_displaycontrol &= ~LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void cursor() {
	_displaycontrol |= LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void noBlink() {
	_displaycontrol &= ~LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void blink() {
	_displaycontrol |= LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void scrollDisplayLeft(void) {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void scrollDisplayRight(void) {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void leftToRight(void) {
	_displaymode |= LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void rightToLeft(void) {
	_displaymode &= ~LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void autoscroll(void) {
	_displaymode |= LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void noAutoscroll(void) {
	_displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void createChar(unsigned char location, unsigned char charmap[]) {
	int i;
	location &= 0x7; // we only have 8 locations 0-7
	command(LCD_SETCGRAMADDR | (location << 3));
	for (i=0; i<8; i++) {
		write_l(charmap[i]);
	}
}

// Turn the (optional) backlight off/on
void noBacklight(void) {
	_backlightval=LCD_NOBACKLIGHT;
	expanderWrite(0);
}

void backlight(void) {
	_backlightval=LCD_BACKLIGHT;
	expanderWrite(0);
}

/*********** mid level commands, for sending data/cmds */

inline void command(unsigned char value) {
	send(value, 0);
}


/************ low level data pushing commands **********/

// write either command or data
void send(unsigned char value, unsigned char mode) {
	unsigned char highnib=value&0xf0;
	unsigned char lownib=(value<<4)&0xf0;
       write4bits((highnib)|mode);
	write4bits((lownib)|mode); 
}

void write4bits(unsigned char value) {
	expanderWrite(value);
	pulseEnable(value);
}


void expanderWrite(unsigned char _data){                                        
	//local_I2C_WriteByte((_data) | _backlightval);
	//outb(((_data) | _backlightval), BASEPORT); //x86 instruction!!!!!
	//outb_p(((_data) | _backlightval),BASEPORT); //x86 instruction kernel!!!!!
		
	/// так тут старшие 4 бита _data это данные на db7-db4
	/// бит 3 не юзается а дальше E/RW/RS = 2/1/0	
	
	gpio_set_value(HD44780_DB4, (_data>>4)&0x01);
	gpio_set_value(HD44780_DB5, (_data>>5)&0x01);
	gpio_set_value(HD44780_DB6, (_data>>6)&0x01);
	gpio_set_value(HD44780_DB7, (_data>>7)&0x01);
	
	gpio_set_value(HD44780_RS, (_data&0x01));
	gpio_set_value(HD44780_RW, (_data>>1)&0x01);
	gpio_set_value(HD44780_E, (_data>>2)&0x01);
}

void pulseEnable(unsigned char _data){
	expanderWrite(_data | En);	// En high
	delayMicroseconds(1);		// enable pulse must be >450ns
	expanderWrite(_data & ~En);	// En low
	delayMicroseconds(50);		// commands need > 37us to settle
} 


// Alias functions

void cursor_on(){
	cursor();
}

void cursor_off(){
	noCursor();
}

void blink_on(){
	blink();
}

void blink_off(){
	noBlink();
}

void load_custom_character(unsigned char char_num, unsigned char *rows){
		createChar(char_num, rows);
}

void setBacklight(unsigned char new_val){
	if(new_val){
		backlight();		// turn backlight on
	}else{
		noBacklight();		// turn backlight off
	}
}

void printstr(const char c[]){
	//This function is not identical to the function used for "real" I2C displays
	//it's here so the user sketch doesn't have to be changed 
	unsigned int i =0;
	for (i=0; c[i] != '\0'; i++) 
		write_l(c[i]);
}

void print_to_string (unsigned char col, unsigned char row, unsigned char c[], unsigned char len)
{
	unsigned char i;
	if (len > _cols)
		len = _cols;
	if (len != _cols)
	{
		setCursor(0, row);
		for (i=0; i<LCD_COLUMNS; i++)
			write_l(' ');
	}
	setCursor(col, row);
	for (i=0; i<len; i++)
		write_l(c[i]);
}

/* End LCD block */


/* FS block */
static int minor = 0;
module_param( minor, int, S_IRUGO );

static char *info_str = "lcdGpio device driver\nAutor Dolin Sergey aka dlinyj dliny@gmail.com\nmodified for RPI: Hoshi\n";         // buffer!

static ssize_t dev_read( struct file * file, char * buf,
						size_t count, loff_t *ppos ) {
	int len = strlen( info_str );
	if( count < len ) return -EINVAL;
	if( *ppos != 0 ) {
		return 0;
	}
	if( copy_to_user( buf, info_str, len ) ) return -EINVAL;
	*ppos = len;
	return len;
}


#ifdef MY_WRITE
static int str_pos = 0;
static int col_pos = 0;

static ssize_t dev_write( struct file *file, const char *buf, size_t count, loff_t *ppos ) {
	int i;
//Начинаем копировать байты.
	for (i=0; i<count;i++) { 
//переводим курсор в текущую позицию
		setCursor(col_pos, str_pos);
//если позиция у нас нулевая по обоим координатам, то очищаем экран 
		if ((col_pos==0) && (str_pos==0)) clear(); 
//если не перевод каретки, то выводим на экран
		if (buf[i] != '\n') { 
			write_l(buf[i]);
			col_pos++;
			}
//если перевод каретки, то делаем позицию курсора максимальной
		else {col_pos=LCD_COLUMNS;}
//при максимальной позиции курсора преходим на следующую строку
		if (col_pos == LCD_COLUMNS) {
			col_pos=0;
			str_pos++;
//если исчерпали лимит строк, то идём в нулевую строку
			if (str_pos == LCD_STRINGS)	{
				str_pos=0;
			}
		}
	}
	return count;
}
#endif

#ifndef MY_WRITE

#define DFLT_DISP_ROWS	2		// Default number of rows the display has
#define DFLT_DISP_COLS	16		// Default number of columns the display has
#define TABSTOP			3		// Length of tabs

#define MAX_DISP_ROWS	2		// The HD44780 supports up to 4 rows
#define MAX_DISP_COLS	16		// The HD44780 supports up to 40 columns
/* input_state states */
#define NORMAL			0
#define ESC				1   	// Escape sequence start
#define DCA_Y			2   	// Direct cursor access, the next input will be the row
#define DCA_X			3   	// Direct cursor access, the next input will be the column
#define CGRAM_SELECT	4		// Selecting which slot to enter new character
#define CGRAM_ENTRY		5		// Adding a new character to the CGRAM
#define CGRAM_GENERATE	6		// Waiting fot the 8 bytes which define a character
#define CHAR_MAP_OLD	7		// Waiting for the original char to map to another
#define CHAR_MAP_NEW	8		// Waiting for the new char to replace the old one with
#define ESC_			10		// Waiting for the [ in escape sequence

static int disp_rows = MAX_DISP_ROWS;
static int disp_cols = MAX_DISP_COLS;
static unsigned char state[ DFLT_DISP_ROWS ][ DFLT_DISP_COLS ];	// The current state of the display
static int disp_row = 0, disp_column = 0; 						// Current actual cursor position
static int row = 0, column = 0; 								// Current virtual cursor position
static int wrap = 0;											// Linewrap on/off
//static char backlight = 0;									// Backlight on-off


/* Send character data to the display, i.e. writing to DDRAM */
static void writeData( unsigned char data){
	/* check and see if we really need to write anything */
	if( state[ row ][ column ] != data )
	{
		state[ row ][ column ] = data;
		/* set the cursor position if need be.
		 * Special case for 16x1 displays, They are treated as two
		 * 8 charcter lines side by side, and dont scroll along to
		 * the second line automaticly.
		 */
		if( disp_row != row || disp_column != column ||
				( disp_rows == 1 && disp_cols == 16 && column == 8 ) )
		{
		/* Some transation done here so 4 line displays work */
			setCursor(column, row);
			disp_row = row;
			disp_column = column;
		}
		write_l(data);
		disp_column++;
	}
	if ( column < disp_cols - 1 )
		column++;
	else if ( wrap && column == disp_cols - 1 && row < disp_rows - 1 )
	{
		column = 0;
		row++;
	}
}

static void stop_symb(char * aut_st, int i) {
	switch (i) {
		case 0: 
			aut_st[12]='_';
			aut_st[13]='_';
			aut_st[14]='_';
			break;
		case 1: 
			aut_st[11]='[';
			aut_st[12]='S';
			aut_st[13]='T';
			aut_st[14]='P';
			aut_st[15]=']';
			break;
		case 2: 
			aut_st[13]='|';
			break;
		case 3: 
			aut_st[13]='|';
			break;
		}
}


static void blacklight(void) {
	unsigned char avto [4][16] = {"                ",
								  "     .--.       ",
								  ".----'   '--.   ",
								  "'-()-----()-'   "};
	unsigned char avto_buff[16];
	int i,j;
	clear();
	for(j=16;j>=0;j--){
		clear();
		for (i=0;i<4;i++) {
			memset(avto_buff, ' ', sizeof(avto_buff));
			strncpy (avto_buff, &avto[i][j], 16-j);
			stop_symb(avto_buff, i);
			print_to_string (0, i, avto_buff, 16);
		}
	delayMicroseconds(500000);
	}
	delayMicroseconds(1000000);
	for(j=1;j<=16;j++){
		clear();
		for (i=0;i<4;i++) {
			memset(avto_buff, ' ', sizeof(avto_buff));
			strncpy (&avto_buff[j], &avto[i][0], 16-j);
			//print_to_string (0, i, avto_buff, 16);
			stop_symb(avto_buff, i);
			print_to_string (0, i, avto_buff, 16);
		}
		delayMicroseconds(500000);
	}
	clear();
}



static void handleInput( unsigned char input )
{

	static int cgram_index = 0;
	static int cgram_row_count;
	static unsigned char cgram_pixels[ 8 ];
	static unsigned char char_map_old;
	static int input_state = NORMAL; 			// the current state of the input handler
	int i;
	int j;
	int temp;

	if ( input_state == NORMAL )
	{
		switch ( input )
		{
			case 0x08: 	// Backspace
				if ( column > 0 )
				{
					column--;
					writeData( ' ' );
					column--;
				}
				break;
			case 0x09: 	// Tabstop
				column = ( ( ( column + 1 ) / TABSTOP ) * TABSTOP ) + TABSTOP - 1;
				break;
			//case 0x0a: 	// Newline
			case '\n':	//new line
				if ( row < disp_rows - 1 )
					row++;
				else
				{
					/* scroll up */
					temp = column;
					for ( i = 0; i < disp_rows - 1; i++ )
					{
						row = i;
						for( j = 0; j < disp_cols; j++ )
						{
							column = j;
							writeData( state[ i + 1 ][ j ] );
						}
					}
					row = disp_rows - 1;
					column = 0;
					for ( i = 0; i < disp_cols; i++ )
					{
						writeData( ' ' );
					}
					column = temp;
				}
				/* Since many have trouble grasping the \r\n concept... */
				column = 0;
				break;
			//case 0x0d: 	// Carrage return
			case '\r':
				column = 0;
				break;
			case 0x1b: 	// esc ie. start of escape sequence
				input_state = ESC_;
				break;
			default:
				/* The character is looked up in the */
				writeData( charmap[ input ] );
		}
	}
	else if ( input_state == ESC_ )
	{
		input_state = ESC;
	}
	else if ( input_state == ESC )
	{
		if( input <= '7' && input >= '0' )
		{
			/* Chararacter from CGRAM */
			writeData( input - 0x30 );
		} else {
			switch ( input )
			{
				case 'A': 		// Cursor up
					if ( row > 0 )
						row--;
					break;
				case 'B': 		// Cursor down
					if ( row < disp_rows - 1 )
						row++;
					break;
				case 'C': 		// Cursor Right
					if ( column < disp_cols - 1 )
						column++;
					break;
				case 'D': 		// Cursor Left
					if ( column > 0 )
						column--;
					break;
				case 'H': 		// Cursor home
					row = 0;
					column = 0;
					break;
				case 'J': 		// Clear screen, cursor doesn't move
					memset( state, ' ', sizeof( state ) );
					clear();
/*					writeCommand( 0x01, 1 );
					if( NUM_CONTROLLERS == 2 )
						writeCommand( 0x01, 2 );
*/
					break;
				case 'K': 		// Erase to end of line, cursor doesn't move
					temp = column;
					for ( i = column; i < disp_cols; i++ )
						writeData( ' ' );
					column = temp;
					break;
				case 'M':		// Charater mapping
					input_state = CHAR_MAP_OLD;
					break;
				case 'Y': 		// Direct cursor access
					input_state = DCA_Y;
					break;
				case 'R':		// CGRAM select
					input_state = CGRAM_SELECT;
					break;
				case 'V':		// Linewrap on
					wrap = 1;
					break;
				case 'W':		// Linewrap off
					wrap = 0;
					break;
				case 'b':       // blacklight
					blacklight();
					break;
				default:
					printk( "LCD: unrecognized escape sequence: %#x ('%c')\n", input, input );
			}
		}
		if ( input_state != DCA_Y &&
				input_state != CGRAM_SELECT &&
				input_state != CHAR_MAP_OLD )
		{
			input_state = NORMAL;
		}
	}
	else if ( input_state == DCA_Y )
	{
		if ( input - 0x1f < disp_rows )
			row = input - 0x1f;
		else
		{
			printk( "LCD: tried to set cursor to off screen location\n" );
			row = disp_rows - 1;
		}
		input_state = DCA_X;
	}
	else if ( input_state == DCA_X )
	{
		if ( input - 0x1f < disp_cols )
			column = input - 0x1f;
		else
		{
			printk( "LCD: tried to set cursor to off screen location\n" );
			column = disp_cols - 1;
		}
		input_state = NORMAL;
	}
	else if ( input_state == CGRAM_SELECT )
	{
		if( input > '7' || input < '0' )
		{
			printk( "LCD: Bad CGRAM index %c\n", input );
			input_state = NORMAL;
		} else {
			cgram_index = input - 0x30;
			cgram_row_count = 0;
			input_state = CGRAM_GENERATE;
		}
	}
	else if( input_state == CGRAM_GENERATE )
	{
		cgram_pixels[ cgram_row_count++ ] = input;
		if( cgram_row_count == 8 )
		{
			//writeCGRAM( cgram_index, cgram_pixels );
			createChar(cgram_index, cgram_pixels);
			input_state = NORMAL;
		}
	}
	else if( input_state == CHAR_MAP_OLD )
	{
		char_map_old = input;
		input_state = CHAR_MAP_NEW;
	}
	else if( input_state == CHAR_MAP_NEW )
	{
		charmap[ char_map_old ] = input;
		input_state = NORMAL;
	}
}

static ssize_t dev_write( struct file *file, const char *buf, size_t count, loff_t *ppos ) {
	int i;
	for (i=0; i<count;i++) 
		handleInput(buf[i]);
	return count;
}
#endif

static const struct file_operations lptlcd_fops = {
	.owner  = THIS_MODULE,
	.read   = dev_read,
	.write  = dev_write,
};

static struct miscdevice lptlcd_dev = {
	MISC_DYNAMIC_MINOR,    // автоматически выбираемое
	"gpiolcd",
	&lptlcd_fops
};

static void GPIO_init( void ) {
	gpio_direction_output(HD44780_DB4, 1);
	gpio_direction_output(HD44780_DB5, 1);
	gpio_direction_output(HD44780_DB6, 1);
	gpio_direction_output(HD44780_DB7, 1);
	
	gpio_direction_output(HD44780_RS, 1);
	gpio_direction_output(HD44780_RW, 1);
	gpio_direction_output(HD44780_E, 1);
}

static int __init dev_init( void ) {
	int ret;
	if( minor != 0 ) lptlcd_dev.minor = minor;
	ret = misc_register( &lptlcd_dev );
	if( ret ) printk( KERN_ERR "=== Unable to register misc device\n" );
	GPIO_init();
	LCD_init(0, LCD_COLUMNS, LCD_STRINGS);
	//print_to_string (0, 0, "lptlcd init     "			, 16);
	return ret;
}

static void __exit dev_exit( void ) {
	misc_deregister( &lptlcd_dev );
}
 
static int __init dev_init( void );
module_init( dev_init );

static void __exit dev_exit( void );
module_exit( dev_exit );

MODULE_LICENSE("GPL");
MODULE_AUTHOR( "Dolin Sergey <dlinyj@gmail.com> Modified for RPI: Hoshi <vovan7773@gmail.com>" );
MODULE_VERSION( "0.1" );
