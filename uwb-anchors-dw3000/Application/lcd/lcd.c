/*
 * lcd.c
 *
 *  Created on: Jun 16, 2024
 *      Author: nguye
 */

#include "lcd.h"
#include "ssd1306.h"
#include "fonts.h"
#include "mode_anchor_active_object.h"

void lcd_clear(){
	SSD1306_Clear();
}

void lcd_display_APP_NAME_(){
	SSD1306_GotoXY(0, 0);
#if (ANCHOR_TYPE == 'A')
	SSD1306_Puts("ACHOR TYPE A",&Font_7x10, 1);
#elif (ANCHOR_TYPE == 'B')
	SSD1306_Puts("ACHOR TYPE B",&Font_7x10, 1);
#elif (ANCHOR_TYPE == 'C')
	SSD1306_Puts("ACHOR TYPE C",&Font_7x10, 1);
#elif (ANCHOR_TYPE == 'D')
	SSD1306_Puts("ACHOR TYPE D",&Font_7x10, 1);
#endif
	SSD1306_UpdateScreen();
}

void lcd_display_MENU_(){
	lcd_display_APP_NAME_();

}

void lcd_display_parameters(float temp, float press, float humi, float rssi, float distance){

}

