/*
 * File: dwm_pico_5110_LCD.h
 * Project: dwm_pico_5110_lcd
 * -----
 * This source code is released under GPLv3 license.
 * Check LICENSE file for license agreement,
 * copyrights, 3rd party licenses and changes info can be found in COPYING file.
 * -----
 * Copyright 2023 - 2023 M.Kusiak (timax)
 */

/* --------------------
 * Source library author: Caio Rodrigo
 * Github: https://github.com/Zeldax64?tab=repositories
 * --------------------
 */

#ifndef DWM_PICO_5110_LCD
#define DWM_PICO_5110_LCD

#include <stdbool.h>
#include "font.h"
#include "hardware/spi.h"

#define LCD_SPI_MAX_SPEED 4000000

#define LCD_SETYADDR 0x40
#define LCD_SETXADDR 0x80
#define LCD_DISPLAY_BLANK 0x08
#define LCD_DISPLAY_NORMAL 0x0C
#define LCD_DISPLAY_ALL_ON 0x09
#define LCD_DISPLAY_INVERTED 0x0D

#define LCD_COLUMN_HEIGHT 8
#define LCD_ROW_NUMBER 6
#define LCD_LETTERS_IN_ROW LCD_WIDTH / FONT_SYMBOL_WIDTH

#define LCD_WIDTH 84
#define LCD_HEIGHT 48
#define LCD_SIZE (LCD_WIDTH * LCD_HEIGHT) / LCD_COLUMN_HEIGHT

/**
 * @brief LCD parameters
 */
struct LCD_att
{
	spi_inst_t *spi;
	uint8_t buffer[LCD_SIZE];
	bool invertText;
};

/**
 * @brief GPIO ports used
 */
struct LCD_GPIO
{
	uint16_t RST;
	uint16_t SCE;
	uint16_t DC;
	uint16_t DIN;
	uint16_t SCLK;
};

/*----- SPI CONF ------*/

void LCD_setSPIInstance(spi_inst_t *spi);

/*----- GPIO Pins -----*/

void LCD_setRST(uint16_t PIN);
void LCD_setSCE(uint16_t PIN);
void LCD_setDC(uint16_t PIN);
void LCD_setDIN(uint16_t PIN);
void LCD_setSCLK(uint16_t PIN);

/*----- Library Functions -----*/

void LCD_init();
void LCD_invert(bool mode);
void LCD_invertText(bool mode);
void LCD_putChar(char c);
void LCD_print(char *str, uint8_t x0, uint8_t row);
void LCD_printCenter(char *str, uint8_t length, uint8_t row);
void LCD_clrScr();
void LCD_goXY(uint8_t x0, uint8_t row);

/*---- Helper functions -----*/

bool LCD_getPixel(uint8_t x0, uint8_t y0);

/*----- Draw Functions -----*/
/*
 * These functions draw in a buffer variable. It's necessary to use LCD_refreshScr() or LCD_refreshArea()
 * in order to send data to the LCD.
 */

void LCD_clrBuff();
void LCD_refreshScr();
void LCD_refreshArea(uint8_t x0, uint8_t x1, uint8_t row, uint8_t nRow);
void LCD_setPixel(uint8_t x0, uint8_t y0, bool mode);
void LCD_drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void LCD_drawRectangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void LCD_drawTriangle(uint8_t xA, uint8_t yA, uint8_t xB, uint8_t yB, uint8_t xC, uint8_t yC);
void LCD_drawCircle(uint8_t x0, uint8_t y0, uint8_t radius);
void LCD_fillShape(int8_t x0, int8_t y0, bool mode);

#endif
