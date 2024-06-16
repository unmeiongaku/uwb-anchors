/*
 * active_object.c
 *
 *  Created on: Jun 16, 2024
 *      Author: nguye
 */


#include "mode_anchor_active_object.h"
#include "port.h"
#include <stdlib.h>
#include <deca_device_api.h>
#include <deca_spi.h>
#include <port.h>
#include <shared_defines.h>
#include <shared_functions.h>
#include <example_selection.h>
#include <config_options.h>
#include "string.h"
#include "stdio.h"
#include "lcd.h"

extern dwt_config_t config_options;
extern dwt_txconfig_t txconfig_options;
extern dwt_txconfig_t txconfig_options_ch9;

#if  ANCHOR_TYPE == 'A'
	#define ANT_DELAY 16500//16550
#elif ANCHOR_TYPE == 'B'
	#define ANT_DELAY 16500//16550
#elif ANCHOR_TYPE == 'C'
	#define ANT_DELAY 16500
#elif ANCHOR_TYPE == 'D'
	#define ANT_DELAY 16500
#endif

/* Default antenna delay values for 64 MHz PRF. See NOTE 2 below. */
#define TX_ANT_DLY  ANT_DELAY  //16535//16525
#define RX_ANT_DLY  ANT_DELAY //16535//16525

#define RESP_TX_TO_FINAL_RX_DLY_UUS (100 + CPU_COM)


static event_status_t proobject_state_handle_RESET_DWIC_SM(proobject_t *const mobj, event_t const *const e);
static event_status_t proobject_state_handle_INIT_SM(proobject_t *const mobj, event_t const *const e);
static event_status_t proobject_state_handle_ANCHOR_LOOP_SM(proobject_t *const mobj, event_t const *const e);

static uint8_t nextstatesig = 0;

uint8_t getnextstatesig(){
	return nextstatesig;
}

void resetnextstatesig(){
	nextstatesig = 0;
}

void proobject_init(proobject_t *const mobj){
	event_t ee;
	/*First Init*/
	mobj->goodSts = 0;
	mobj->loopCount = 0;
	mobj->messageFlag = 0;
	ee.sig = ENTRY;
	mobj->active_state = ANCHOR_RESET_DWIC_SM;
	/*Display Application on LCD*/
	lcd_clear();
	lcd_display_APP_NAME_();
	proobject_state_machine(mobj,&ee);
}


event_status_t proobject_state_machine(proobject_t *const mobj, event_t const *const e){
	switch (mobj->active_state){
		case ANCHOR_RESET_DWIC_SM:
		{
			return proobject_state_handle_RESET_DWIC_SM(mobj,e);
		}
		case ANCHOR_INIT_SM:
		{
			return proobject_state_handle_INIT_SM(mobj,e);
		}
		case ANCHOR_LOOP_SM:
		{
			return proobject_state_handle_ANCHOR_LOOP_SM(mobj,e);
		}
			break;
		case MAX_SM:
			return EVENT_IGNORED;
			break;
	}
	return EVENT_IGNORED;
}


static event_status_t proobject_state_handle_RESET_DWIC_SM(proobject_t *const mobj, event_t const *const e){
	switch(e->sig){
		case ENTRY:
		{
			/*Reset DWIC function*/
			reset_DWIC();
			return EVENT_HANDLED;
		}
		case EXIT:
		{
			return EVENT_HANDLED;
		}
		case NEXT_SIG:
		{
			mobj->active_state = ANCHOR_INIT_SM;
			return EVENT_TRANSITION;
		}
		case TICK_SIG:
		{
			/*Make sure DW IC is in IDLE mode */
			if(!dwt_checkidlerc()){
				/*False Reset*/
				/*Print to LCD Status*/

			}
			else{
				/*Reset IC Success*/
				nextstatesig = 1;
			}
			return EVENT_HANDLED;
		}
	}
	return EVENT_IGNORED;
}

static event_status_t proobject_state_handle_INIT_SM(proobject_t *const mobj, event_t const *const e){
	switch(e->sig){
		case ENTRY:
		{
			/*Init DW3000*/
			if(dwt_initialise(DWT_DW_IDLE) == DWT_ERROR){
				/*False Init*/
				/*Print to LCD Status*/
			}
			else{
				/*Configure DWIC*/
				if(dwt_configure(&config_options) == DWT_SUCCESS){
					/*Configure the TX Spectrum parameters*/
					if(config_options.chan == 5){
						dwt_configuretxrf(&txconfig_options);
					}
					else{
						dwt_configuretxrf(&txconfig_options_ch9);
					}
					/*ENABLE FRAME FILTERING*/
					#if ANCHOR_TYPE == 'A'
						dwt_configureframefilter(DWT_FF_ENABLE_802_15_4, DWT_FF_DATA_EN);
						dwt_setpanid(0xDECA);
						dwt_setaddress16(0x141);
					#elif ANCHOR_TYPE == 'B'
						dwt_configureframefilter(DWT_FF_ENABLE_802_15_4, DWT_FF_DATA_EN);
						dwt_setpanid(0xDECA);
						dwt_setaddress16(0x242);
					#elif ANCHOR_TYPE == 'C'
						dwt_configureframefilter(DWT_FF_ENABLE_802_15_4, DWT_FF_DATA_EN);
						dwt_setpanid(0xDECA);
						dwt_setaddress16(0x444);
					#elif ANCHOR_TYPE == 'D'
						dwt_configureframefilter(DWT_FF_ENABLE_802_15_4, DWT_FF_DATA_EN);
						dwt_setpanid(0xDECA);
						dwt_setaddress16(0x646);
					#endif
					/*Apply default antenna delay value. See Note 2 */
						dwt_setrxantennadelay(RX_ANT_DLY);
						dwt_setrxantennadelay(TX_ANT_DLY);
					/*Delay Between the response frame and final frame*/
						dwt_setrxaftertxdelay(RESP_TX_TO_FINAL_RX_DLY_UUS);
					/*Enable IC diagnostic calculation and logging*/
					dwt_configciadiag(DW_CIA_DIAG_LOG_ALL);
					nextstatesig = 1;
				}
				else{
					/*False Config DWIC*/
					/*Print to LCD Status*/
				}
			}
			return EVENT_HANDLED;
		}
		case EXIT:
		{
			return EVENT_HANDLED;
		}
		case NEXT_SIG:
		{

			return EVENT_TRANSITION;
		}
		case TICK_SIG:
		{
			return EVENT_HANDLED;
		}
	}
	return EVENT_IGNORED;
}


static event_status_t proobject_state_handle_ANCHOR_LOOP_SM(proobject_t *const mobj, event_t const *const e){
	switch(e->sig){
		case ENTRY:
		{
			return EVENT_HANDLED;
		}
		case EXIT:
		{
			return EVENT_HANDLED;
		}
		case NEXT_SIG:

			return EVENT_TRANSITION;
		}
	return EVENT_IGNORED;
}
