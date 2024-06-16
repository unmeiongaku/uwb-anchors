/*
 * user_define.h
 *
 *  Created on: Jun 16, 2024
 *      Author: nguye
 */

#include "main.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

#define MAX_CALLBACK_FUNC 10
#define UWB_PERIOD_CALLBACK 5



#define TIM_DELAY_US											htim6
#define TIM_DELAY_MS											htim6

#ifndef INC_USER_DEFINE_H_
#define INC_USER_DEFINE_H_

#define WAITSYS_NOT_PASSED						0
#define PASS_FIRST_WAITSYS							1
#define PASS_SECOND_WAITSYS						2
#define PASS_THIRD_WAITSYS							3


#define ANCHOR_SELECT 		0
/*	ANCHOR TYPE A					0
 *  ANCHOR TYPE B					1
 *  ANCHOR TYPE C					2
 *  ANCHOR TYPE D					3
 * */
#if ANCHOR_SELECT == 0
#define ANCHOR_TYPE	'A'
#elif ANCHOR_SELECT == 1
#define ANCHOR_TYPE	'B'
#elif ANCHOR_SELECT == 2
#define ANCHOR_TYPE	'C'
#elif ANCHOR_SELECT == 3
#define ANCHOR_TYPE	'D'
#endif


/*UWB State Machine Define*/
#define NEXT_SIG_DEFINE 				1

#endif /* INC_USER_DEFINE_H_ */
