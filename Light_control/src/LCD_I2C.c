#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_i2c.h"
#include "delay.h"
#include "I2C.h"
#include "LCD_I2C.h"


void LCDI2C_write(uint8_t value) {
    LCDI2C_send(value, Rs);
}

LiquidCrystal_I2C_Def lcdi2c;

//Initialization LCD_I2C
void LCDI2C_init(uint8_t lcd_Addr,uint8_t lcd_cols,uint8_t lcd_rows)
{
    lcdi2c.Addr = lcd_Addr;
    lcdi2c.cols = lcd_cols;
    lcdi2c.rows = lcd_rows;
    lcdi2c.backlightval = LCD_NOBACKLIGHT;

    init_I2C1(); //Initialization I2C1
    lcdi2c.displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
    LCDI2C_begin(lcd_cols, lcd_rows);
}

//Start LCD_I2C
void LCDI2C_begin(uint8_t cols, uint8_t lines) {
    if (lines > 1) {
        lcdi2c.displayfunction |= LCD_2LINE;
    }
    lcdi2c.numlines = lines;
    Delay(50);

    // Pull both RS and R/W low to begin commands
    LCDI2C_expanderWrite(lcdi2c.backlightval);	// reset expanderand turn backlight off (Bit 8 =1)
    Delay(1000);

    //Start in 8bit mode, try to set 4 bit mode
    LCDI2C_write4bits(0x03 << 4);
    DelayMC(4500); // wait min 4.1ms

    //Second
    LCDI2C_write4bits(0x03 << 4);
    DelayMC(4500); // wait min 4.1ms

    //Third
    LCDI2C_write4bits(0x03 << 4);
    DelayMC(150);

    //Set to 4-bit interface
    LCDI2C_write4bits(0x02 << 4);


    //Set # lines, font size, etc.
    LCDI2C_command(LCD_FUNCTIONSET | lcdi2c.displayfunction);

    //Turn the display on with no cursor or blinking default
    lcdi2c.displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    LCDI2C_display();
    LCDI2C_clear();

    //Initialize to default text direction
    lcdi2c.displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;

    //Set the entry mode
    LCDI2C_command(LCD_ENTRYMODESET | lcdi2c.displaymode);

    LCDI2C_home();

}

// Clear display, set cursor position to zero
void LCDI2C_clear() {
    LCDI2C_command(LCD_CLEARDISPLAY);
    DelayMC(3000);
}
// Set cursor position to zero
void LCDI2C_home() {
    LCDI2C_command(LCD_RETURNHOME);
    DelayMC(3000);
}
// Set cursor position
void LCDI2C_setCursor(uint8_t col, uint8_t row) {
    int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
    if ( row > lcdi2c.numlines ) {
        row = lcdi2c.numlines-1;
    }
    LCDI2C_command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off
void LCDI2C_noDisplay() {
    lcdi2c.displaycontrol &= ~LCD_DISPLAYON;
    LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}
void LCDI2C_display() {
    lcdi2c.displaycontrol |= LCD_DISPLAYON;
    LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}

// Turns the underline cursor on/off
void LCDI2C_noCursor() {
    lcdi2c.displaycontrol &= ~LCD_CURSORON;
    LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}
void LCDI2C_cursor() {
    lcdi2c.displaycontrol |= LCD_CURSORON;
    LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}

// Turn on and off the blinking cursor
void LCDI2C_noBlink() {
    lcdi2c.displaycontrol &= ~LCD_BLINKON;
    LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}
void LCDI2C_blink() {
    lcdi2c.displaycontrol |= LCD_BLINKON;
    LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}

// These commands scroll the display without changing the RAM
void LCDI2C_scrollDisplayLeft(void) {
    LCDI2C_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void LCDI2C_scrollDisplayRight(void) {
    LCDI2C_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void LCDI2C_leftToRight(void) {
    lcdi2c.displaymode |= LCD_ENTRYLEFT;
    LCDI2C_command(LCD_ENTRYMODESET | lcdi2c.displaymode);
}

// This is for text that flows Right to Left
void LCDI2C_rightToLeft(void) {
    lcdi2c.displaymode &= ~LCD_ENTRYLEFT;
    LCDI2C_command(LCD_ENTRYMODESET | lcdi2c.displaymode);
}

// This will 'right justify' text from the cursor
void LCDI2C_autoscroll(void) {
    lcdi2c.displaymode |= LCD_ENTRYSHIFTINCREMENT;
    LCDI2C_command(LCD_ENTRYMODESET | lcdi2c.displaymode);
}

// This will 'left justify' text from the cursor
void LCDI2C_noAutoscroll(void) {
    lcdi2c.displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
    LCDI2C_command(LCD_ENTRYMODESET | lcdi2c.displaymode);
}

// Turn the (optional) backlight off/on
void LCDI2C_noBacklight(void) {
    lcdi2c.backlightval=LCD_NOBACKLIGHT;
    LCDI2C_expanderWrite(0);
}
void LCDI2C_backlight(void) {
    lcdi2c.backlightval=LCD_BACKLIGHT;
    LCDI2C_expanderWrite(0);
}

void LCDI2C_command(uint8_t value) {
    LCDI2C_send(value, 0);
}

// Write either command or data
void LCDI2C_send(uint8_t value, uint8_t mode) {
    uint8_t highnib=value&0xf0;
    uint8_t lownib=(value<<4)&0xf0;
    LCDI2C_write4bits((highnib)|mode);
    LCDI2C_write4bits((lownib)|mode);
}

void LCDI2C_write4bits(uint8_t value) {
    LCDI2C_expanderWrite(value);
    LCDI2C_pulseEnable(value);
}

void LCDI2C_expanderWrite(uint8_t _data) {
    I2C_StartTransmission (I2C1, I2C_Direction_Transmitter, lcdi2c.Addr);
    I2C_WriteData(I2C1, (int)(_data) | lcdi2c.backlightval);
    I2C_GenerateSTOP(I2C1, ENABLE);
}

void LCDI2C_pulseEnable(uint8_t _data) {
    LCDI2C_expanderWrite(_data | En);	// En high
    DelayMC(1);		// Enable pulse must be >450ns

    LCDI2C_expanderWrite(_data & ~En);	// En low
    DelayMC(50);	// Commands need > 37us to settle
}

void LCDI2C_write_String(char* str) {
    uint8_t i=0;
    while(str[i])
    {
        LCDI2C_write(str[i]);
        i++;
    }
}

// Alias functions

void LCDI2C_cursor_on() {
    LCDI2C_cursor();
}

void LCDI2C_cursor_off() {
    LCDI2C_noCursor();
}

void LCDI2C_blink_on() {
    LCDI2C_blink();
}

void LCDI2C_blink_off() {
    LCDI2C_noBlink();
}


void LCDI2C_setBacklight(uint8_t new_val) {
    if(new_val) {
        LCDI2C_backlight();		// Turn backlight on
    } else {
        LCDI2C_noBacklight();	// Turn backlight off
    }
}




