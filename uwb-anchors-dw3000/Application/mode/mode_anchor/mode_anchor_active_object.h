/*
 * active_object.h
 *
 *  Created on: Jun 16, 2024
 *      Author: nguye
 */

#ifndef MODE_ANCHOR_ACTIVE_OBJECT_H_
#define MODE_ANCHOR_ACTIVE_OBJECT_H_

#include "stdint.h"

typedef enum {
    EVENT_HANDLED,
    EVENT_IGNORED,
    EVENT_TRANSITION
}event_status_t;

/* Signals of the application*/
typedef enum{
/* Internal activity signals */
	NEXT_SIG = 1,
	TICK_SIG ,
    ENTRY,
    EXIT
}proobject_signal_t;


typedef enum{
	ANCHOR_RESET_DWIC_SM,
	ANCHOR_INIT_SM,
    ANCHOR_LOOP_SM,
    MAX_SM,
}proobject_state_t;

//forward declarations
struct proobject_tag;
struct event_tag;

typedef struct proobject_tag {
	int16_t stsQual;
	int goodSts;
	uint8_t loopCount;
	uint8_t messageFlag;
    proobject_state_t active_state;
}proobject_t;

/*Generic(Super) event structure */
typedef struct event_tag{
    uint8_t sig;
}event_t;

/* For user generated events */
typedef struct{
    event_t super;
    uint8_t ss;
}proobject_user_event_t;

/* For tick event */
typedef struct{
    event_t super;
    uint8_t ss;
}proobject_tick_event_t;

uint8_t getnextstatesig();
void resetnextstatesig();

void proobject_init(proobject_t *const mobj);
event_status_t proobject_state_machine(proobject_t *const mobj, event_t const * const e);

#endif /* MODE_ANCHOR_ACTIVE_OBJECT_H_ */
