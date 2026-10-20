// Ansteuerung eines HD44780 kompatiblen LCD im 4-Bit-Interfacemodus
// http://www.mikrocontroller.net/articles/HD44780
// http://www.mikrocontroller.net/articles/AVR-GCC-Tutorial/LCD-Ansteuerung
//
// Pins can be configured in lcd.h
 
//#include <avr/io.h>
#include "lcd.h"
#include "delay.h"
//#include <avr/pgmspace.h>

#define USE_BSRR 0

#if !USE_NON_BLOCKING_LCD


//static uint8_t lcd_turnOn_state = 0;
static uint8_t lcd_command_state = 0;
static uint8_t lcd_data_state = 0;
static uint8_t lcd_clear_state = 0;


////////////////////////////////////////////////////////////////////////////////
void _delay_us(uint16_t us)
{
	//TODO *2 can probably be removed, now that LCD driver seems to work -> check!
	DWT_Delay_us(us*2);
}

void _delay_ms(uint32_t Delay)
{
	HAL_Delay(Delay*2);
}
////////////////////////////////////////////////////////////////////////////////
// Erzeugt einen Enable-Puls
static void lcd_enable( void )
{
    // Enable auf 1 setzen
	LCD_E_GPIO_Port->BSRR = LCD_E_Pin;
    _delay_us( LCD_ENABLE_US );  // kurze Pause
    // Enable auf 0 setzen
	LCD_E_GPIO_Port->BSRR = (uint32_t)LCD_E_Pin<<16U;
}
////////////////////////////////////////////////////////////////////////////////
// Sendet eine 4-bit Ausgabeoperation an das LCD
static void lcd_out( uint8_t data )
{
    data &= 0xF0;
    data = data>>4;// obere 4 Bit maskieren

#if USE_BSRR //BSRR version not working in release build due to mysterious reasons
    LCD_D4_GPIO_Port->BSRR = (uint32_t)(LCD_D4_Pin | LCD_D5_Pin | LCD_D6_Pin | LCD_D7_Pin)<<16U ;
    LCD_D4_GPIO_Port->BSRR = (((uint32_t)data)<<7);
#else

    for(int i=0;i<4;i++)
    {
    	if((data>>i) & 0x1)
    	{
    		LCD_D4_GPIO_Port->ODR |= (LCD_D4_Pin<<i);
    	}
    	else
    	{
    		LCD_D4_GPIO_Port->ODR &= ~(LCD_D4_Pin<<i);
    	}
    }
#endif
    lcd_enable();
}

////////////////////////////////////////////////////////////////////////////////
// Initialisierung: muss ganz am Anfang des Programms aufgerufen werden.
void lcd_init( void )
{
	/*
    // verwendete Pins auf Ausgang schalten
    uint8_t pins = (0x0F << LCD_DB);           // 4 Datenleitungen
	
    uint8_t rsen_pins = (1<<LCD_RS) |                // R/S Leitung
                        (1<<LCD_EN);                 // Enable Leitung
    LCD_DDR |= pins;
    LCD_RSEN_DDR |= rsen_pins;
 
    // INIT ALL OUTPUTS TO 0
    LCD_PORT &= (uint8_t)~pins;
    LCD_RSEN_PORT &= (uint8_t)~rsen_pins;
    */
 
    // warten auf die Bereitschaft des LCD
    //_delay_ms( LCD_BOOTUP_MS );
	HAL_Delay( LCD_BOOTUP_MS );
    
#if 1
    // Set 8 bit mode 3 times (soft reset)
    // If the LCD is in 8 bit mode it will be set again 3 times.
    // If the LCD is in 4 bit mode expecting the 2nd nibble the 
    // first write will be lost, the next two set 8 bit mode
    // Soft-Reset muss 3mal hintereinander gesendet werden zur Initialisierung


    lcd_out( LCD_SOFT_RESET );
    HAL_Delay( LCD_SOFT_RESET_MS1 );
 
    lcd_enable();
    HAL_Delay( LCD_SOFT_RESET_MS2 );
 
    lcd_enable();
    HAL_Delay( LCD_SOFT_RESET_MS3 );

    // Initialization sequence is not quite as documented by Winstar.
    // Documented sequence only works on initial power-up.
    // An additional step of putting back into 8-bit mode first is
    // required to handle a warm-restart.
    lcd_command(LCD_FUNCTION_8BIT);
    HAL_Delay( LCD_SOFT_RESET_MS3 );
    lcd_command(LCD_FUNCTION_8BIT);
        HAL_Delay( LCD_SOFT_RESET_MS3 );
        lcd_command(LCD_FUNCTION_8BIT);
            HAL_Delay( LCD_SOFT_RESET_MS3 );


    // 4-bit Modus aktivieren
    lcd_out( LCD_SET_FUNCTION |
             LCD_FUNCTION_4BIT );
    HAL_Delay( LCD_SET_4BITMODE_MS );

    // 4-bit Modus aktivieren
    lcd_out( LCD_SET_FUNCTION |
             LCD_FUNCTION_4BIT );
    HAL_Delay( LCD_SET_4BITMODE_MS );

	/*
	LCD_RS_GPIO_Port->ODR &= ~LCD_RS_Pin;
	HAL_Delay(100);
	 lcd_out(0x00);
	 HAL_Delay(100);
	  lcd_out(0x00);
	  HAL_Delay(100);
	  lcd_out(0x02);
	  HAL_Delay(100);
 */

    // 4-bit Modus aktivieren 
    lcd_out( LCD_SET_FUNCTION |
             LCD_FUNCTION_4BIT );
    HAL_Delay( LCD_SET_4BITMODE_MS );
    
    lcd_command(LCD_SET_SHIFT | OLED_POWER_OFF);	//0x13
    lcd_command(LCD_SET_SHIFT | OLED_POWER_ON);	//0x17

 
    // 4-bit Modus / 2 Zeilen / 5x7
    lcd_command( LCD_SET_FUNCTION |
                 LCD_FUNCTION_4BIT |
                 LCD_FUNCTION_2LINE |
                 LCD_FUNCTION_5X7 |
                 LCD_FUNCTION_RUSSIAN);
                 
    // Display off / Cursor off / Blinken LCD_BLINKING_OFF
    // This additional "Display Off" seems to prevent OLED soft reset scrambling
    // No Idea why! It's not in the datasheet but seems to help.
    lcd_command( LCD_SET_DISPLAY |
                 LCD_DISPLAY_OFF |
                 LCD_CURSOR_OFF |
                 LCD_BLINKING_OFF); 

    lcd_clear();
                 

 
    // Cursor inkrement / kein Scrollen
    lcd_command( LCD_SET_ENTRY |
                     LCD_ENTRY_INCREASE |
                     LCD_ENTRY_NOSHIFT );

    HAL_Delay(1);

// extra steps required for OLED initialisation (no effect on LCD)
    //lcd_command(0x17);
   // HAL_Delay(1);
    //lcd_byte(0x17,LCD_CMD, DEL_INITNEXT)    # character mode, power on

    // Display ein / Cursor aus / Blinken aus
        lcd_command( LCD_SET_DISPLAY |
                     LCD_DISPLAY_ON |
                     LCD_CURSOR_OFF |
                     LCD_BLINKING_OFF);
 
    lcd_clear();
#else



    lcd_out( LCD_SOFT_RESET );
    HAL_Delay( LCD_SOFT_RESET_MS1 );

    lcd_enable();
    HAL_Delay( LCD_SOFT_RESET_MS2 );

    lcd_enable();
    HAL_Delay( LCD_SOFT_RESET_MS3 );
    // Initialise display into 4 bit mode, using recommended delays
    lcd_out( 0x33 );
    HAL_Delay( 10 );

    lcd_out( 0x32 );
    HAL_Delay( 10 );

    // Now perform remainder of display init in 4 bit mode - IMPORTANT!
    // These steps MUST be exactly as follows, as OLEDs in particular are rather fussy
    lcd_out( 0x28 );
    HAL_Delay( 10 );

    lcd_out( 0x08 );
    HAL_Delay( 10 );

    lcd_out( 0x01 );
    HAL_Delay( 10 );

    lcd_out( 0x06 );
    HAL_Delay( 10 );

    // extra steps required for OLED initialisation (no effect on LCD)
    lcd_out( 0x17 );
    HAL_Delay( 10 );

    // now turn on the display, ready for use - IMPORTANT!
    lcd_out( 0x0C );
    HAL_Delay( 10 );
#endif
}
  
////////////////////////////////////////////////////////////////////////////////
// Sendet ein Datenbyte an das LCD
void lcd_data( uint8_t data )
{
    //LCD_RSEN_PORT |= (1<<LCD_RS);    // RS auf 1 setzen
	LCD_RS_GPIO_Port->BSRR = LCD_RS_Pin;
 
    lcd_out( data );            // zuerst die oberen, 
    lcd_out( (uint8_t)(data<<4) );         // dann die unteren 4 Bit senden
 
    _delay_us( LCD_WRITEDATA_US );
}
////////////////////////////////////////////////////////////////////////////////
uint8_t lcd_data_nonBlock( uint8_t data )
{
	switch(lcd_data_state)
	{
		case 0:
			// RS auf 1 setzen
				//LCD_RS_GPIO_Port->BSRR = LCD_RS_Pin;
			LCD_RS_GPIO_Port->ODR |= LCD_RS_Pin;
			//lcd out data upper 4 bit
			data &= 0xF0;

			data = data>>4;
		#if USE_BSRR //BSRR version not working in release build due to mysterious reasons
			LCD_D4_GPIO_Port->BSRR = (uint32_t)(LCD_D4_Pin | LCD_D5_Pin | LCD_D6_Pin | LCD_D7_Pin)<<16U ;
			LCD_D4_GPIO_Port->BSRR = (((uint32_t)data)<<7);
		#else

			for(int i=0;i<4;i++)
			{
				if((data>>i) & 0x1)
				{
					LCD_D4_GPIO_Port->ODR |= (LCD_D4_Pin<<i);
				}
				else
				{
					LCD_D4_GPIO_Port->ODR &= ~(LCD_D4_Pin<<i);
				}
			}
		#endif

			//LCD_D4_GPIO_Port->BSRR = (uint32_t)(LCD_D4_Pin | LCD_D5_Pin | LCD_D6_Pin | LCD_D7_Pin)<<16u;
			//LCD_D4_GPIO_Port->BSRR = (data>>4)<<7;
			//enable
			//LCD_E_GPIO_Port->BSRR = LCD_E_Pin; //E on
			LCD_E_GPIO_Port->ODR |= LCD_E_Pin;
			DWT_init_nonblock(LCD_ENABLE_US );
			lcd_data_state=1;
			break;

		case 3:
		case 1:
			if(DWT_check_nonblock())
			{
				//LCD_E_GPIO_Port->BSRR = LCD_E_Pin<<16; //enable off
				LCD_E_GPIO_Port->ODR &= ~LCD_E_Pin;
				lcd_data_state++;
			}
			break;

		case 2:
			//lcd out data lower 4 bit
			//LCD_D4_GPIO_Port->BSRR = (uint32_t)(LCD_D4_Pin | LCD_D5_Pin | LCD_D6_Pin | LCD_D7_Pin)<<16U;
			//LCD_D4_GPIO_Port->BSRR = (data&0xf)<<7;

			data = data&0xf;
			#if USE_BSRR //BSRR version not working in release build due to mysterious reasons
				LCD_D4_GPIO_Port->BSRR = (uint32_t)(LCD_D4_Pin | LCD_D5_Pin | LCD_D6_Pin | LCD_D7_Pin)<<16U ;
				LCD_D4_GPIO_Port->BSRR = (((uint32_t)data)<<7);
			#else

				for(int i=0;i<4;i++)
				{
					if((data>>i) & 0x1)
					{
						LCD_D4_GPIO_Port->ODR |= (LCD_D4_Pin<<i);
					}
					else
					{
						LCD_D4_GPIO_Port->ODR &= ~(LCD_D4_Pin<<i);
					}
				}
			#endif

			//LCD_E_GPIO_Port->BSRR = LCD_E_Pin; //E on
			LCD_E_GPIO_Port->ODR |= LCD_E_Pin;
			DWT_init_nonblock(LCD_ENABLE_US );
			lcd_data_state=3;
			break;

		case 4:
			DWT_init_nonblock( LCD_WRITEDATA_US );
			lcd_data_state = 5;
			break;

		case 5:
			if(DWT_check_nonblock())
			{
				lcd_data_state=0;
				return 1;
			}
	}
	return 0;
}

 
////////////////////////////////////////////////////////////////////////////////
// Sendet einen Befehl an das LCD
void lcd_command( uint8_t data )
{
    //LCD_RSEN_PORT &= (uint8_t)~(1<<LCD_RS);    // RS auf 0 setzen
#if USE_BSRR
	LCD_RS_GPIO_Port->BSRR = (uint32_t)LCD_RS_Pin<<16U;
#else
	LCD_RS_GPIO_Port->ODR &= ~LCD_RS_Pin;
#endif
 
    lcd_out( data );             // zuerst die oberen, 
    lcd_out( (uint8_t)(data<<4) );           // dann die unteren 4 Bit senden
 
    _delay_us( LCD_COMMAND_US );
}
////////////////////////////////////////////////////////////////////////////////
// Sendet einen Befehl an das LCD
uint8_t lcd_command_nonblock( uint8_t data )
{
	switch(lcd_command_state)
	{
		case 0:
			// RS auf 0 setzen
			//LCD_RS_GPIO_Port->BSRR = (uint32_t)LCD_RS_Pin<<16U;
			LCD_RS_GPIO_Port->ODR &= ~(LCD_RS_Pin);
			//lcd out data upper 4 bit
			data &= 0xF0;
			//LCD_D4_GPIO_Port->BSRR = (uint32_t)(LCD_D4_Pin | LCD_D5_Pin | LCD_D6_Pin | LCD_D7_Pin)<<16U;
			//LCD_D4_GPIO_Port->BSRR = (data>>4)<<7;

			data = data>>4;
			#if USE_BSRR //BSRR version not working in release build due to mysterious reasons
				LCD_D4_GPIO_Port->BSRR = (uint32_t)(LCD_D4_Pin | LCD_D5_Pin | LCD_D6_Pin | LCD_D7_Pin)<<16U ;
				LCD_D4_GPIO_Port->BSRR = (((uint32_t)data)<<7);
			#else

				for(int i=0;i<4;i++)
				{
					if((data>>i) & 0x1)
					{
						LCD_D4_GPIO_Port->ODR |= (LCD_D4_Pin<<i);
					}
					else
					{
						LCD_D4_GPIO_Port->ODR &= ~(LCD_D4_Pin<<i);
					}
				}
			#endif

			//enable
			//LCD_E_GPIO_Port->BSRR = LCD_E_Pin; //E on
				LCD_E_GPIO_Port->ODR |= LCD_E_Pin;
			DWT_init_nonblock(LCD_ENABLE_US );
			lcd_command_state=1;
			break;

		case 3:
		case 1:
			if(DWT_check_nonblock())
			{
				//LCD_E_GPIO_Port->BSRR = (uint32_t)LCD_E_Pin<<16U; //enable off
				LCD_E_GPIO_Port->ODR &= ~LCD_E_Pin;
				lcd_command_state++;
			}
			break;

		case 2:
			//lcd out data lower 4 bit
			//LCD_D4_GPIO_Port->BSRR = (uint32_t)(LCD_D4_Pin | LCD_D5_Pin | LCD_D6_Pin | LCD_D7_Pin)<<16U;
			//LCD_D4_GPIO_Port->BSRR = (data&0xf)<<7;
			data = data&0xf;
			#if USE_BSRR //BSRR version not working in release build due to mysterious reasons
				LCD_D4_GPIO_Port->BSRR = (uint32_t)(LCD_D4_Pin | LCD_D5_Pin | LCD_D6_Pin | LCD_D7_Pin)<<16U ;
				LCD_D4_GPIO_Port->BSRR = (((uint32_t)data)<<7);
			#else

				for(int i=0;i<4;i++)
				{
					if((data>>i) & 0x1)
					{
						LCD_D4_GPIO_Port->ODR |= (LCD_D4_Pin<<i);
					}
					else
					{
						LCD_D4_GPIO_Port->ODR &= ~(LCD_D4_Pin<<i);
					}
				}
			#endif


			//LCD_E_GPIO_Port->BSRR = LCD_E_Pin; //E on
				LCD_E_GPIO_Port->ODR |= LCD_E_Pin;
			DWT_init_nonblock(LCD_ENABLE_US );
			lcd_command_state=3;
			break;

		case 4:
			DWT_init_nonblock( LCD_COMMAND_US );
			lcd_command_state = 5;
			break;

		case 5:
			if(DWT_check_nonblock())
			{
				lcd_command_state=0;
				return 1;
			}
			break;
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////
uint8_t lcd_turnOn_nonblock(uint8_t isOn, uint8_t cursorOn)
{
	uint8_t data = 	(uint8_t)(LCD_SET_DISPLAY |
            				(LCD_DISPLAY_ON * isOn) |
							LCD_CURSOR_ON * cursorOn |
							LCD_BLINKING_OFF);


	return lcd_command_nonblock( data );
/*
	switch(lcd_turnOn_state)
	{
		case 0:
			// RS auf 0 setzen
			LCD_RS_GPIO_Port->BSRR = LCD_RS_Pin<<16;
			//lcd out data upper 4 bit
			data &= 0xF0;
			LCD_D4_GPIO_Port->BSRR = (LCD_D4_Pin | LCD_D5_Pin | LCD_D6_Pin | LCD_D7_Pin)<<16;
			LCD_D4_GPIO_Port->BSRR = (data>>4)<<7;
			//enable
			LCD_E_GPIO_Port->BSRR = LCD_E_Pin; //E on
			lcd_turnOn_state=1;
			break;

		case 3:
		case 1:
			LCD_E_GPIO_Port->BSRR = LCD_E_Pin<<16; //enable off
			lcd_turnOn_state++;
			break;
		case 2:

			//lcd out data lower 4 bit
			LCD_D4_GPIO_Port->BSRR = (LCD_D4_Pin | LCD_D5_Pin | LCD_D6_Pin | LCD_D7_Pin)<<16;
			LCD_D4_GPIO_Port->BSRR = (data&0xf)<<7;
			LCD_E_GPIO_Port->BSRR = LCD_E_Pin; //E on
			lcd_turnOn_state=3;
			break;

		case 4:
			lcd_turnOn_state=0;
			return 1;
			break;

	}

	return 0;
	*/

}
////////////////////////////////////////////////////////////////////////////////
void lcd_turnOn(uint8_t isOn, uint8_t cursorOn)
{
	
	    // Display ein / Cursor aus / Blinken aus
    lcd_command( (uint8_t)(LCD_SET_DISPLAY |
                 (LCD_DISPLAY_ON * isOn) |
                 LCD_CURSOR_ON * cursorOn |
                 LCD_BLINKING_OFF));
}

////////////////////////////////////////////////////////////////////////////////
// Sendet den Befehl zur L�schung des Displays
void lcd_clear( void )
{
    lcd_command( LCD_CLEAR_DISPLAY );
    _delay_ms( LCD_CLEAR_DISPLAY_MS );
}


uint8_t lcd_clear_nonblock( void )
{
	if(lcd_clear_state==0)
	{
		if(lcd_command_nonblock( LCD_CLEAR_DISPLAY ))
		{
			DWT_init_nonblock( LCD_CLEAR_DISPLAY_MS*1000 );
			lcd_clear_state++;

		}
	}
	else
	{
		if(DWT_check_nonblock())
		{
			lcd_clear_state = 0;
			return 1;
		}
	}
    return 0;

}
 
////////////////////////////////////////////////////////////////////////////////
// Sendet den Befehl: Cursor Home
void lcd_home( void )
{
    lcd_command( LCD_CURSOR_HOME );
    _delay_ms( LCD_CURSOR_HOME_MS );
}
 
////////////////////////////////////////////////////////////////////////////////
// Setzt den Cursor in Spalte x (0..15) Zeile y (1..4) 
 
void lcd_setcursor( uint8_t x, uint8_t y )
{
    uint8_t data;
 
    switch (y)
    {
        case 1:    // 1. Zeile
            data = (uint8_t)(LCD_SET_DDADR + LCD_DDADR_LINE1 + x);
            break;
 
        case 2:    // 2. Zeile
            data = (uint8_t)(LCD_SET_DDADR + LCD_DDADR_LINE2 + x);
            break;
 
        case 3:    // 3. Zeile
            data = (uint8_t)(LCD_SET_DDADR + LCD_DDADR_LINE3 + x);
            break;
 
        case 4:    // 4. Zeile
            data = (uint8_t)(LCD_SET_DDADR + LCD_DDADR_LINE4 + x);
            break;
 
        default:
            return;                                   // f�r den Fall einer falschen Zeile
    }
 
    lcd_command( data );
}
////////////////////////////////////////////////////////////////////////////////
uint8_t lcd_setcursor_nonBlock(  uint8_t x, uint8_t y )
{
    uint8_t data;

    //TODO switch could be outsourced to own state
    switch (y)
    {
    	default:
        case 1:    // 1. row
            data = (uint8_t)(LCD_SET_DDADR + LCD_DDADR_LINE1 + x);
            break;

        case 2:    // 2. row
            data = (uint8_t)(LCD_SET_DDADR + LCD_DDADR_LINE2 + x);
            break;

        case 3:    // 3. row
            data = (uint8_t)(LCD_SET_DDADR + LCD_DDADR_LINE3 + x);
            break;

        case 4:    // 4. row
            data = (uint8_t)(LCD_SET_DDADR + LCD_DDADR_LINE4 + x);
            break;
    }

    return lcd_command_nonblock( data );

}
 
////////////////////////////////////////////////////////////////////////////////
// Schreibt einen String auf das LCD
 
void lcd_string( const char *data )
{
    while( *data != '\0' )
    {
        lcd_data( *data++ );
    }
}
////////////////////////////////////////////////////////////////////////////////
/*
void lcd_string_F( const char *data )
{
  while (pgm_read_byte(&(*data))!= '\0')
   lcd_data(pgm_read_byte(&(*data++)));
};
*/
 
////////////////////////////////////////////////////////////////////////////////
// Schreibt ein Zeichen in den Character Generator RAM
 
void lcd_generatechar( uint8_t code, const uint8_t *data )
{
    // Startposition des Zeichens einstellen
    lcd_command( LCD_SET_CGADR | (uint8_t)(code<<3) );
 
    // Bitmuster uebertragen
    for ( uint8_t i=0; i<8; i++ )
    {
        lcd_data( data[i] );
    }
}

#endif
