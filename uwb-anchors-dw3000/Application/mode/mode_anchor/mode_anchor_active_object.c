/*
 * active_object.c
 *
 *  Created on: Jun 16, 2024
 *      Author: nguye
 */


#include "mode_anchor_active_object.h"

static event_status_t proobject_state_handle_RESET_DWIC_SM(proobject_t *const mobj, event_t const *const e);
static event_status_t proobject_state_handle_INIT_SM(proobject_t *const mobj, event_t const *const e);
static event_status_t proobject_state_handle_WAIT_TAG_SEND_POLL_MSG_SM(proobject_t *const mobj, event_t const *const e);



void proobject_init(proobject_t *const mobj){
	event_t ee;
	ee.sig = ENTRY;
	/*First Init*/
	mobj->goodSts = 0;
	mobj->loopCount = 0;
	mobj->messageFlag = 0;
	mobj->active_state = ANCHOR_RESET_DWIC_SM;
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
		case WAIT_TAG_SEND_POLL_MSG_SM:
		{
			return proobject_state_handle_WAIT_TAG_SEND_POLL_MSG_SM(mobj,e);
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
			return EVENT_HANDLED;
		}
	}
	return EVENT_IGNORED;
}

static event_status_t proobject_state_handle_INIT_SM(proobject_t *const mobj, event_t const *const e){
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


static event_status_t proobject_state_handle_WAIT_TAG_SEND_POLL_MSG_SM(proobject_t *const mobj, event_t const *const e){
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
