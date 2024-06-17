/*
 * mode_anchor.c
 *
 *  Created on: Jun 16, 2024
 *      Author: nguye
 */

#include "mode_anchor.h"
#include "timer.h"
#include "mode_anchor_active_object.h"
#include "user_define.h"
#include "lcd.h"
//#include "ssd1306.h"

TID(gtid_uwb_anchors);

static proobject_t A0s;
static void proobject_event_dispatcher(proobject_t *const mobj,event_t const *const e);


static void uwb_callback(void* ctx){
	static uint8_t proevent;
	static uint16_t tickcnt = 0;
	proobject_user_event_t ue;
	static proobject_tick_event_t te;
	proevent = getnextstatesig();
	/*Make Event*/
	switch(proevent){
	case NEXT_SIG_DEFINE:
	{
		ue.super.sig = NEXT_SIG;
	}
		break;
	case FALSE_SIG_DEFINE:
	{
		ue.super.sig = FALSE_SIG;
	}
		break;
	case OVERLOAD_RX_BUFFER_DEFINE:
	{
		ue.super.sig = OVERLOAD_BUFER_SIG;
	}
		break;
	case RX_POLL_MSG_DEFINE:
	{
		ue.super.sig = RX_POLL_MSG_SIG;
	}
		break;
	case RX_FINAL_MSG_DEFINE:
	{
		ue.super.sig = RX_FINAL_MSG_SIG;
	}
		break;
	case RX_NO_MSG_DEFINE:
	{
		ue.super.sig = RX_NO_MSG_SIG;
	}
		break;
	}
	resetnextstatesig();
	proobject_event_dispatcher(&A0s,&ue.super);
	//4. dispatch the time tick event for every  UWB_PERIOD_CALLBACK*10ms
	if(tickcnt==UWB_PERIOD_CALLBACK*PRESCALER_TIME_TICK_PERIOD){
		te.super.sig = TICK_SIG;
		proobject_event_dispatcher(&A0s,&te.super);
		tickcnt = 0;
	}
	else{
		tickcnt++;
	}
}

void mode_anchor_init(){
	proobject_init(&A0s);
	gtid_uwb_anchors = timer_register_callback(uwb_callback, UWB_PERIOD_CALLBACK, 0, TIMER_MODE_REPEAT);

}

void mode_run_deinit(){
	timer_unregister_callback(gtid_uwb_anchors);
}

static void proobject_event_dispatcher(proobject_t *const mobj,event_t const *const e){

  event_status_t status;
  proobject_state_t source, target;

  source = mobj->active_state;
  status = proobject_state_machine(mobj,e);

  if(status == EVENT_TRANSITION){
    target = mobj->active_state;
    event_t ee;
    //1. run the exit action for the source state
    ee.sig = EXIT;
    mobj->active_state = source;
    proobject_state_machine(mobj,&ee);

    //2. run the entry action for the target state
    ee.sig = ENTRY;
    mobj->active_state = target;
    proobject_state_machine(mobj,&ee);
  }
}
