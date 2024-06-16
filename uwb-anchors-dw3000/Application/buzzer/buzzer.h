/*
 * buzzer.h
 *
 *  Created on: Jun 16, 2024
 *      Author: nguye
 */

#ifndef BUZZER_BUZZER_H_
#define BUZZER_BUZZER_H_

#include <stdint.h>

typedef enum{
	BUZZER_SUCCESS,
	BUZZER_ERROR,
}buzzer_t;

void buzzer_sys_start();
void buzzer_notification(buzzer_t buz,uint32_t ms);

#endif /* BUZZER_BUZZER_H_ */
