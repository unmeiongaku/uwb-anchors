/*
 * active_object.h
 *
 *  Created on: Jun 16, 2024
 *      Author: nguye
 */

#ifndef MODE_ANCHOR_ACTIVE_OBJECT_H_
#define MODE_ANCHOR_ACTIVE_OBJECT_H_

#include "stdint.h"
#include "stdbool.h"

typedef enum {
    EVENT_HANDLED,
    EVENT_IGNORED,
    EVENT_TRANSITION
}event_status_t;

/* Signals of the application*/
typedef enum{
/* Internal activity signals */
	NEXT_SIG = 1,
	FALSE_SIG,


	OVERLOAD_BUFER_SIG,
	RX_POLL_MSG_SIG,
	RX_FINAL_MSG_SIG,
	RX_NO_MSG_SIG,

	TICK_SIG ,
    ENTRY,
    EXIT
}proobject_signal_t;


typedef enum{
	ANCHOR_RESET_DWIC_SM,
	ANCHOR_INIT_SM,
	ANCHOR_RX_POLLING_CHECKING_SM,
	ANCHOR_FALSE_POLLING_CHECKING_SM,
	ANCHOR_CLASSIFY_RX_BUFFER_SM,

	ANCHOR_OVERLOAD_RX_BUFER_SM,
	ANCHOR_RX_POLL_MSG_SM,
	ANCHOR_RX_FINAL_MSG_SM,
	ANCHOR_RX_NO_MSG_SM,

    MAX_SM,
}proobject_state_t;

//forward declarations
struct proobject_tag;
struct event_tag;

typedef struct proobject_tag {
	int16_t stsQual;
	int goodSts;
	uint16_t stsStatus;
	uint8_t loopCount;
	uint8_t messageFlag;
	uint32_t status_reg;
	bool syswait;
	bool firstcheck;
	bool check_lo_mask;
	bool check_hi_mask;
	bool error;
	uint32_t frame_len;

	/* Timestamps of frames transmission/reception. */
	uint64_t poll_rx_ts;
	uint64_t resp_tx_ts;
	uint8_t frame_seq_nb;

	uint32_t resp_tx_time;
	int ret;

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
