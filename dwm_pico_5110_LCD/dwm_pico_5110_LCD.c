/*
 * File: dwm_pico_5110_LCD.c
 * Project: dwm_pico_5110_lcd
 * -----
 * This source code is released under GPLv3 license.
 * Check LICENSE file for license agreement,
 * copyrights, 3rd party licenses and changes info can be found in COPYING file.
 * -----
 * Copyright 2023 - 2023 M.Kusiak (timax)
 */

#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

#include "dwm_pico_5110_LCD.h"

#define STATE_HIGH 1
#define STATE_LOW 0

struct LCD_att lcd;
struct LCD_GPIO lcd_gpio;

/*-------- SPI CONF --------*/

/**
 * @brief Set SPI interface for LCD.
 *
 * @param spi spi interface (spi0 or spi1).
 */
void LCD_setSPIInstance(spi_inst_t *spi)
{
  lcd.spi = spi;
}

/*----- GPIO Functions -----*/

/**
 * @brief Set RST (reset) pin for LCD.
 *
 * @param PIN RST pin.
 */
void LCD_setRST(uint16_t PIN)
{
  lcd_gpio.RST = PIN;
}

/**
 * @brief Set SCE (slave chip enable) pin for LCD.
 *
 * @param PIN SCE pin.
 */
void LCD_setSCE(uint16_t PIN)
{
  lcd_gpio.SCE = PIN;
}

/**
 * @brief Set D/C (data / command select) pin for LCD.
 *
 * @param PIN D/C pin.
 */
void LCD_setDC(uint16_t PIN)
{
  lcd_gpio.DC = PIN;
}

/**
 * @brief Set DIN (data in) pin for LCD.
 *
 * @param PIN DIN pin.
 */
void LCD_setDIN(uint16_t PIN)
{
  lcd_gpio.DIN = PIN;
}

/**
 * @brief Set SCLK (serial clock) pin for LCD.
 *
 * @param PIN SCLK pin.
 */
void LCD_setSCLK(uint16_t PIN)
{
  lcd_gpio.SCLK = PIN;
}

/*----- Library Functions -----*/

/**
 * @brief Write command to LCD screen.
 *
 * @param command command to be written.
 */
void LCD_writeCommand(uint8_t command)
{
  gpio_put(lcd_gpio.DC, STATE_LOW);
  gpio_put(lcd_gpio.SCE, STATE_LOW);
  spi_write_blocking(lcd.spi, &command, 1);
  gpio_put(lcd_gpio.SCE, STATE_HIGH);
}

/**
 * @brief Write data do LCD screen.
 *
 * @param data data to be written.
 * @param size size of the data.
 */
void LCD_writeData(uint8_t *data, uint16_t size)
{
  gpio_put(lcd_gpio.DC, STATE_HIGH);
  gpio_put(lcd_gpio.SCE, STATE_LOW);
  spi_write_blocking(lcd.spi, data, size);
  gpio_put(lcd_gpio.SCE, STATE_HIGH);
}

/**
 * @brief Initialize the LCD using predetermined values.
 *
 * @attention Must initialise SPI first! Max supported speed is 4MHZ (LCD_SPI_MAX_SPEED)
 */
void LCD_init()
{
  // Setup SCE (slave chip enable), RST and D/C (data / command select) pins as output
  gpio_init(lcd_gpio.SCE);
  gpio_set_dir(lcd_gpio.SCE, GPIO_OUT);

  gpio_init(lcd_gpio.RST);
  gpio_set_dir(lcd_gpio.RST, GPIO_OUT);

  gpio_init(lcd_gpio.DC);
  gpio_set_dir(lcd_gpio.DC, GPIO_OUT);

  // Setup DIN (data in) and SCLK (serial clock) pins for SPI communication
  gpio_set_function(lcd_gpio.DIN, GPIO_FUNC_SPI);
  gpio_set_function(lcd_gpio.SCLK, GPIO_FUNC_SPI);

  // Reset screen registers
  gpio_put(lcd_gpio.RST, STATE_LOW);
  sleep_us(1);
  gpio_put(lcd_gpio.RST, STATE_HIGH);

  // Perform initial commands
  LCD_writeCommand(0x21);               // LCD extended commands.
  LCD_writeCommand(0xB8);               // set LCD Vop(Contrast).
  LCD_writeCommand(0x04);               // set temp coefficent.
  LCD_writeCommand(0x14);               // LCD bias mode 1:40.
  LCD_writeCommand(0x20);               // LCD basic commands.
  LCD_writeCommand(LCD_DISPLAY_NORMAL); // LCD normal.
  LCD_clrScr();

  lcd.invertText = false;
}

/**
 * @brief Invert the color shown on the display.
 *
 * @param mode: true = inverted / false = normal.
 */
void LCD_invert(bool mode)
{
  if (mode)
    LCD_writeCommand(LCD_DISPLAY_INVERTED);
  else
    LCD_writeCommand(LCD_DISPLAY_NORMAL);
}

/**
 * @brief Invert the colour of any text sent to the display.
 *
 * @param mode: true = inverted / false = normal.
 */
void LCD_invertText(bool mode)
{
  lcd.invertText = mode;
}

/**
 * @brief abs function used in LCD_drawLine.
 *
 * @param x any integer.
 * @return  Absolute value of x.
 */
int abs(int x)
{
  if (x < 0)
    return x * -1;
  return x;
}

/**
 * @brief Puts one char on the current position of LCD's cursor.
 *
 * @param c: char to be printed.
 */
void LCD_putChar(char c)
{
  uint8_t letter[FONT_SYMBOL_WIDTH];

  for (int i = 0; i < FONT_SYMBOL_WIDTH; i++)
    if (lcd.invertText)
      letter[i] = ~(ASCII[c - 0x20][i]);
    else
      letter[i] = ASCII[c - 0x20][i];

  LCD_writeData(letter, FONT_SYMBOL_WIDTH);
}

/**
 * @brief Print a string on the LCD.
 *
 * @param str       string to print.
 * @param x0        starting point on the x-axis.
 * @param row       row number (multiple of 8 lines).
 */
void LCD_print(char *str, uint8_t x0, uint8_t row)
{
  LCD_goXY(x0, row);
  while (*str)
    LCD_putChar(*str++);
}

/**
 * @brief Print a string on the LCD, align to center.
 *        Valid only for single line strings.
 *
 * @param str     string to print.
 * @param length  length of the string, without null terminator.
 * @param row     row number (multiple of 8 lines).
 */
void LCD_printCenter(char *str, uint8_t length, uint8_t row)
{
  uint8_t x0 = length > LCD_LETTERS_IN_ROW ? 0 : (LCD_WIDTH - length * FONT_SYMBOL_WIDTH) / 2;

  LCD_print(str, x0, row);
}

/**
 * @brief Updates a square of the screen according to given values.
 *
 * @param x0      starting point on x-axis.
 * @param x1      ending point on x-axis.
 * @param row     starting row (multiple of 8 lines).
 * @param nRow    numer of rows to refresh.
 */
void LCD_refreshArea(uint8_t x0, uint8_t x1, uint8_t row, uint8_t nRow)
{
  for (uint8_t i = 0; i < nRow; i++)
  {
    LCD_goXY(x0, row + i);

    for (uint8_t j = x0; j < x1; j++)
      LCD_writeData(&lcd.buffer[(row + i) * LCD_WIDTH + j], 1);
  }
}

/**
 * @brief     Clears the screen.
 * @attention Does not clear the buffer!
 */
void LCD_clrScr()
{
  static uint8_t zero = 0x00;

  for (int i = 0; i < LCD_SIZE; i++)
    LCD_writeData(&zero, 1);
}

/**
 * @brief Clears lcd.buffer.
 */
void LCD_clrBuff()
{
  for (int i = 0; i < LCD_SIZE; i++)
    lcd.buffer[i] = 0;
}

/**
 * @brief Set LCD's cursor to position X,Y.
 *
 * @param x       position on the x-axis.
 * @param row     row number (multiple of 8 lines).
 */
void LCD_goXY(uint8_t x0, uint8_t row)
{
  LCD_writeCommand(LCD_SETXADDR | x0);  // Column.
  LCD_writeCommand(LCD_SETYADDR | row); // Rows.
}

/**
 * @brief Updates the entire screen according to lcd.buffer.
 */
void LCD_refreshScr()
{
  LCD_goXY(LCD_SETXADDR, LCD_SETYADDR);
  LCD_writeData(lcd.buffer, LCD_SIZE);
}

/**
 * @brief Sets a pixel on the screen.
 *
 * @param x0    pixel location on x axis.
 * @param y0    pixel location on y axis.
 * @param mode  true = lit pixel / false = dim pixel.
 */
void LCD_setPixel(uint8_t x0, uint8_t y0, bool mode)
{
  if (x0 >= LCD_WIDTH)
    x0 = LCD_WIDTH - 1;
  if (y0 >= LCD_HEIGHT)
    y0 = LCD_HEIGHT - 1;

  if (mode)
    lcd.buffer[x0 + (y0 / LCD_COLUMN_HEIGHT) * LCD_WIDTH] |= 1 << (y0 % LCD_COLUMN_HEIGHT);
  else
    lcd.buffer[x0 + (y0 / LCD_COLUMN_HEIGHT) * LCD_WIDTH] &= ~(1 << (y0 % LCD_COLUMN_HEIGHT));
}

/**
 * @brief Get pixel state in the buffer.
 *
 * @param x0 pixel location on x axis.
 * @param y0 pixel location on y axis.
 * @return  true = pixel is lit / false = pixel is dimmed.
 */
bool LCD_getPixel(uint8_t x0, uint8_t y0)
{
  uint8_t shift = y0 / LCD_COLUMN_HEIGHT;
  shift = abs(shift * LCD_COLUMN_HEIGHT - y0);

  return lcd.buffer[x0 + (y0 / LCD_COLUMN_HEIGHT) * LCD_WIDTH] >> shift & 1;
}

/**
 * @brief Draws any line, based on Bresenham's line algorithm.
 *
 * @param x0 starting point on the x-axis.
 * @param y0 starting point on the y-axis.
 * @param x1 ending point on the x-axis.
 * @param y1 ending point on the y-axis.
 */
void LCD_drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
  uint8_t dx = abs(x1 - x0);
  uint8_t dy = abs(y1 - y0);
  int8_t sx = x0 < x1 ? 1 : -1;
  int8_t sy = y0 < y1 ? 1 : -1;
  int8_t err = dx - dy;
  int16_t e2;

  while (x0 != x1 || y0 != y1)
  {
    LCD_setPixel(x0, y0, true);

    e2 = 2 * err;
    if (e2 > -dy)
    {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dx)
    {
      err += dx;
      y0 += sy;
    }
  }

  LCD_setPixel(x0, y0, true);
}

/**
 * @brief Draws a rectangle.
 *
 * @param x0 starting point on the x-axis.
 * @param y0 starting point on the y-axis.
 * @param x1 ending point on the x-axis.
 * @param y1 ending point on the y-axis.
 */
void LCD_drawRectangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
  LCD_drawLine(x0, y0, x1, y0);
  LCD_drawLine(x0, y0, x0, y1);
  LCD_drawLine(x1, y0, x1, y1);
  LCD_drawLine(x0, y1, x1, y1);
}

/**
 * @brief Draws triangle.
 *
 * @param xA position of A corner on x-axis.
 * @param yA position of A corner on y-axis.
 * @param xB position of B corner on x-axis.
 * @param yB position of B corner on y-axis.
 * @param xC position of C corner on x-axis.
 * @param yC position of C corner on y-axis.
 */
void LCD_drawTriangle(uint8_t xA, uint8_t yA, uint8_t xB, uint8_t yB, uint8_t xC, uint8_t yC)
{
  LCD_drawLine(xA, yA, xB, yB);
  LCD_drawLine(xB, yB, xC, yC);
  LCD_drawLine(xC, yC, xA, yA);
}

/**
 * @brief Draws a circle.
 *
 * @param x0      center on x axis.
 * @param y0      center on y axis.
 * @param radius  radius of the circle.
 */
void LCD_drawCircle(uint8_t x0, uint8_t y0, uint8_t radius)
{
  int8_t x = radius;
  int8_t y = 0;
  int8_t err = 0;

  while (x >= y)
  {
    LCD_setPixel(x0 + x, y0 + y, true);
    LCD_setPixel(x0 + y, y0 + x, true);
    LCD_setPixel(x0 - y, y0 + x, true);
    LCD_setPixel(x0 - x, y0 + y, true);
    LCD_setPixel(x0 - x, y0 - y, true);
    LCD_setPixel(x0 - y, y0 - x, true);
    LCD_setPixel(x0 + y, y0 - x, true);
    LCD_setPixel(x0 + x, y0 - y, true);

    if (err <= 0)
    {
      y += 1;
      err += 2 * y + 1;
    }
    else
    {
      x -= 1;
      err -= 2 * x + 1;
    }
  }
}

/**
 * @brief Change pixel state inside closed shape.
 * Uses flood fill algorithm.
 *
 * @param x0    any point on x axis inside the shape.
 * @param y0    any point on y axis inside the shape.
 * @param mode  true = lit pixel / false = dim pixel.
 */
void LCD_fillShape(int8_t x0, int8_t y0, bool mode)
{
  // If out of bounds or pixel does not match, stop
  if (x0 < 0 || x0 >= LCD_WIDTH || y0 < 0 || y0 >= LCD_HEIGHT || LCD_getPixel(x0, y0) == mode)
    return;

  // Set current cell (pixel)
  LCD_setPixel(x0, y0, mode);

  // Recursively fill the neighboring cells
  LCD_fillShape(x0 - 1, y0, mode); // Up
  LCD_fillShape(x0 + 1, y0, mode); // Down
  LCD_fillShape(x0, y0 - 1, mode); // Left
  LCD_fillShape(x0, y0 + 1, mode); // Right
}