/*
 * lcd.c
 *
 *  Created on: 21-Mar-2026
 *      Author: sunbeam
 */

#include "lcd.h"

extern I2C_HandleTypeDef hi2c1;

void lcd_init(void)
{
	// LCD initialization sequence
	HAL_Delay(20);
	lcd_write_nibble(LCD_CMD, 0x03);   //cursor nove
	HAL_Delay(5);
	lcd_write_nibble(LCD_CMD, 0x03);
	HAL_Delay(1);
	lcd_write_nibble(LCD_CMD, 0x03);
	HAL_Delay(1);
	lcd_write_nibble(LCD_CMD, 0x02);
	HAL_Delay(1);

	// LCD initization commands
	lcd_write_byte(LCD_CMD, FUNCTION_SET);
	lcd_write_byte(LCD_CMD, DISPLAY_ON_OFF_CONTROL);
	lcd_write_byte(LCD_CMD, ENTRY_MODE_SET);
	lcd_write_byte(LCD_CMD, LCD_CLEAR);
	HAL_Delay(20);
}

void lcd_write_nibble(uint8_t rs, uint8_t val)
{
	// change RS to 0 for instruction or RS to 1 for data
	uint8_t rs_flag = (rs == LCD_DATA)? BV(LCD_RS_Pos) : 0;
	// write data with RW = 0, EN = 1, and rs flag
	uint8_t data = (val << LCD_DB4_Pos) | rs_flag | BV(LCD_BL_Pos) | BV(LCD_EN_Pos);
	HAL_I2C_Master_Transmit(&hi2c1, LCD_SLAVE_ADDR, &data, 1, HAL_MAX_DELAY);
	// delay
	HAL_Delay(1);
	// write data again with RW = 0, EN = 0, and rs flag
	data = (val << LCD_DB4_Pos) | rs_flag | BV(LCD_BL_Pos);
	HAL_I2C_Master_Transmit(&hi2c1, LCD_SLAVE_ADDR, &data, 1, HAL_MAX_DELAY);
}

void lcd_write_byte(uint8_t rs, uint8_t val)
{
	uint8_t high = val >> 4, low = val & 0x0F;
	lcd_write_nibble(rs, high);
	lcd_write_nibble(rs, low);
	HAL_Delay(3);
}

void lcd_puts(uint8_t line, char str[])
{
	//set line address
	lcd_write_byte(LCD_CMD, line);
	// send characters to lcd one by one
	for(int i = 0 ; str[i] != '\0' ; i++)
		lcd_write_byte(LCD_DATA, str[i]);
}

void lcd_clear(void)
{
    lcd_write_byte(LCD_CMD, LCD_CLEAR);
    HAL_Delay(2);      // LCD needs time after clear command
}
