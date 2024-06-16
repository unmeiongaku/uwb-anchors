/*
 * buzzer.c
 *
 *  Created on: Jun 16, 2024
 *      Author: nguye
 */

#include "buzzer.h"
#include "delay_us.h"
#include "main.h"

void buzzer_sys_start(){
	HAL_GPIO_WritePin(MCU_BUZZER_GPIO_Port, MCU_BUZZER_Pin, SET);
	delay_ms(50);
	HAL_GPIO_WritePin(MCU_BUZZER_GPIO_Port, MCU_BUZZER_Pin, RESET);
	delay_ms(50);
}

void buzzer_notification(buzzer_t buz,uint32_t ms){
	switch(buz){
	case BUZZER_SUCCESS:
	{
		HAL_GPIO_WritePin(MCU_BUZZER_GPIO_Port, MCU_BUZZER_Pin, SET);
		delay_ms(ms);
		HAL_GPIO_WritePin(MCU_BUZZER_GPIO_Port, MCU_BUZZER_Pin, RESET);
		delay_ms(ms);
	}
		break;
	case BUZZER_ERROR:
	{
		HAL_GPIO_WritePin(MCU_BUZZER_GPIO_Port, MCU_BUZZER_Pin, SET);
		delay_ms(ms*4);
		HAL_GPIO_WritePin(MCU_BUZZER_GPIO_Port, MCU_BUZZER_Pin, RESET);
		delay_ms(ms*4);
	}
		break;
	}
}
