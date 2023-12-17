/*
 * myLCD.h
 *
 *  Created on: Aug 25, 2023
 *      Author: ab
 */

#ifndef INC_LCD_KEYPAD_H_
#define INC_LCD_KEYPAD_H_

#include "stm32l4xx_hal.h"

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// keypad codes
#define KPAD_KEYNONE 0x00
#define KPAD_KEYRIGHT 0x01
#define KPAD_KEYUP 0x02
#define KPAD_KEYDOWN 0x03
#define KPAD_KEYLEFT 0x04
#define KPAD_KEYSELECT 0x05 // keypad shield modification required!


void LCD_init();
void LCD_print(char *s);
void LCD_clear();
int KPAD_getkey();
void KPAD_init();
char KPAD_IsKeyPressed();


#endif /* INC_LCD_KEYPAD_H_ */
