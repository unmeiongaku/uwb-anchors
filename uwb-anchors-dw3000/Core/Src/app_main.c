/*
 * app_main.c
 *
 *  Created on: Jun 16, 2024
 *      Author: nguye
 */

#include "app_main.h"
#include "buzzer.h"
#include "user_define.h"
#include "timer.h"
#include "mode_anchor.h"

typedef void (*func_t)(void);

typedef struct{
	func_t 		 	init;
	func_t 		 	deinit;
}app_t;


void app_reset(app_t *app, func_t init, func_t deinit){
	app->init = init;
	app->deinit = deinit;
}


void app_init(app_t *app){
	app->init();
}

void app_run_deinit(app_t *app){
	app->deinit();
}

static app_t g_app;


static void led_callback(){
	HAL_GPIO_TogglePin(MCU_LED_0_GPIO_Port, MCU_LED_0_Pin);
}

void app_main(){
	HAL_TIM_Base_Start(&TIM_DELAY_US);
	buzzer_sys_start();
	timer_register_callback(led_callback, 500, 0, TIMER_MODE_REPEAT);
	// Run default mode
	app_reset(&g_app, mode_anchor_init, mode_run_deinit);
	app_run_init(&g_app);
}


