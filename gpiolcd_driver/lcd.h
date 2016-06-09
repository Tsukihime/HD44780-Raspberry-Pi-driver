#define BASEPORT 0xC020 /* lp1 */
//#define unsigned char unsigned char


// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En 0x04  // Enable bit
#define Rw 0x02  // Read/Write bit
#define Rs 0x01  // Register select bit

  void LCD_init(unsigned char lcd_Addr,unsigned char lcd_cols,unsigned char lcd_rows);
  void begin(unsigned char cols, unsigned char rows);
  void clear(void);
  void home(void);
  void noDisplay(void);
  void display(void);
  void noBlink(void);
  void blink(void);
  void noCursor(void);
  void cursor(void);
  void scrollDisplayLeft(void);
  void scrollDisplayRight(void);
  void printLeft(void);
  void printRight(void);
  void leftToRight(void);
  void rightToLeft(void);
  void shiftIncrement(void);
  void shiftDecrement(void);
  void noBacklight(void);
  void backlight(void);
  void autoscroll(void);
  void noAutoscroll(void); 
  void createChar(unsigned char, unsigned char[]);
  void setCursor(unsigned char, unsigned char); 

  void write_l(unsigned char);

  void command(unsigned char);
  void init(void);
//static inline delayMicroseconds(unsigned int value);

////compatibility API function aliases
void blink_on(void);						// alias for blink()
void blink_off(void);       					// alias for noBlink()
void cursor_on(void);      	 					// alias for cursor()
void cursor_off(void);      					// alias for noCursor()
void setBacklight(unsigned char new_val);				// alias for backlight() and nobacklight()
void load_custom_character(unsigned char char_num, unsigned char *rows);	// alias for createChar()
void printstr(const char[]);
void print_to_string (unsigned char col, unsigned char row, unsigned char c[], unsigned char len);

void master_init_LCD(void);
void init_priv(void);
void send(unsigned char, unsigned char);
void write4bits(unsigned char);
void expanderWrite(unsigned char);
void pulseEnable(unsigned char);


static unsigned char _Addr;
static unsigned char _displayfunction;
static unsigned char _displaycontrol;
static unsigned char _displaymode;
static unsigned char _numlines;
static unsigned char _cols;
static unsigned char _rows;
static unsigned char _backlightval;


