/*
 * timer.h
 *
 *  Created on: Mar 14, 2023
 *      Author: nguye
 */

#ifndef APPLICATION_TIMER_TIMER_H_
#define APPLICATION_TIMER_TIMER_H_

#include <stdint.h>
#include "stdio.h"
#include <stdbool.h>
//#include "tim.h"

typedef void(*timer_callback_func_t)(void *cxt); //void(*)(void *)
typedef int8_t timer_id_t;

typedef void (*timer_callback_delay_func_t)(void *ctx);
typedef int8_t timer_delay_id_t;


#define TID_INVALID 		-1
#define TID(x) 			static timer_id_t x=TID_INVALID;

#define TIDD_INVALID		-1
#define TIDD(y)			static 	timer_delay_id_t  y = TIDD_INVALID;

typedef enum{
	TIMER_MODE_REPEAT,
	TIMER_MODE_ONE_SHOT
}timer_mode_t;

typedef struct{
	timer_callback_func_t timer_callback_func;
	uint16_t period_ms;
	uint16_t cnt;
	void *context;
	uint8_t id;
	timer_mode_t mode;
}callback_t;

typedef struct{
	timer_callback_delay_func_t timer_callback_delay_func;
	uint16_t period_ms;
	uint16_t cnt;
	void *context;
	uint8_t id;
	timer_mode_t mode;
}callback_delay_t;

typedef struct{
	uint32_t fndelay;
	bool isdone;
	bool pointer1;
	bool pointer2;
	bool pointer3;
}dwm_manager_t;

int timer_init();
timer_id_t timer_register_callback(timer_callback_func_t timer_callback_func, uint16_t period_ms, void* cxt, timer_mode_t mode);
timer_id_t timer_unregister_callback(timer_id_t id);


timer_delay_id_t 	timer_register_delay_callback(timer_callback_delay_func_t timer_callback_delay_func, uint16_t period_ms, void* cxt, timer_mode_t mode);
timer_delay_id_t		timer_unregister_delay_callback(timer_delay_id_t id);

uint64_t milis();
void user_systick();
void user_delay_systick();
//void timer_os_simulation_start(timer_isr_mode_t isr_mode,uint16_t prescaler_t, uint16_t peroid_t);
void timer_os_simulation_deinit();

#endif /* APPLICATION_TIMER_TIMER_H_ */
