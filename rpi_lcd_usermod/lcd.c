// gcc -o lcd lcd.c HD44780.c -lrt -lbcm2835

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "HD44780.h"
#include <bcm2835.h>
#include "charmap.h"

#define DFLT_DISP_ROWS	4		// Default number of rows the display has
#define DFLT_DISP_COLS	20		// Default number of columns the display has
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
static int wrap = 0;	

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
			
			HD44780_goto(row, column);
			
			disp_row = row;
			disp_column = column;
		}
		HD44780_put_char(data);
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

void handleInput( unsigned char input )
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
				writeData(  charmap[ input ]  ); 
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
					HD44780_clear_scr();

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
					//blacklight();
					break;
				case 'i':       // lcd init
					HD44780_init();
					break;
				//default:
					//printk( "LCD: unrecognized escape sequence: %#x ('%c')\n", input, input );
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
			//printk( "LCD: tried to set cursor to off screen location\n" );
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
			//printk( "LCD: tried to set cursor to off screen location\n" );
			column = disp_cols - 1;
		}
		input_state = NORMAL;
	}
	else if ( input_state == CGRAM_SELECT )
	{
		if( input > '7' || input < '0' )
		{
			//printk( "LCD: Bad CGRAM index %c\n", input );
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
			HD44780_write_CGRAM(cgram_pixels, cgram_index);
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

int main(int argc, char *argv[])
{
	if (!bcm2835_init()) // Инициализация GPIO
	{
		fprintf(stderr, "ERROR initialize bcm2835.\n");
		return 1;
	}
  
	unsigned char c;
	FILE *instream;
	int bytes_read = 0;
 
	instream = fopen("/dev/stdin","r");
	if(instream!=NULL){
	while((bytes_read=fread(&c, 1, 1, instream))==1)
		{
			handleInput(c);
		}
	}
	else
	{
		fprintf(stderr, "ERROR opening stdin.\n");
		return 1;
	}	
	return 0;
}
