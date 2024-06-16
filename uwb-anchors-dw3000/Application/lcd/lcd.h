/*
 * lcd.h
 *
 *  Created on: Jun 15, 2023
 *      Author: nguye
 */
#include "ssd1306.h"
#include "fonts.h"

#ifndef APPLICATION_LCD_LCD_H_
#define APPLICATION_LCD_LCD_H_

typedef enum{
	DISPLAY_APP_NAME,
	DISPLAY_RESET,
}lcd_display_t;

void lcd_clear();
void lcd_display_APP_NAME_();


#endif /* APPLICATION_LCD_LCD_H_ */
