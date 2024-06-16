/*
 * uwb.h
 *
 *  Created on: Apr 26, 2024
 *      Author: nguye
 */

#ifndef UWB_H_
#define UWB_H_

#include "stdbool.h"

typedef struct{
    int goodSts;           /* Used for checking STS quality in received signal */
    int16_t stsQual;           /* This will contain STS quality index */
    uint16_t stsStatus;        /* Used to check for good STS status (no errors). */
    uint8_t firstLoopFlag; /* Used for checking if the program has gone through the main loop for the first time */
    /* ====> Addresses of anchors and current anchor number <==== */
    uint8_t current_anchor;
    uint16_t	waitresult;	/*Using for call Define function at time callback*/
    uint8_t Firstuwb;
    uint8_t passcnt;
    bool runfirsttime;
    uint16_t sysvalue;
    int8_t ret;
    /*waitsys*/
    uint32_t lo_result;
    uint32_t hi_result;
    uint32_t lo_mask;
    uint32_t hi_mask;

    bool firstwaitsys;
    bool secondwaitsys;
    bool thirdwaitsys;
}uwb_t;

typedef enum{
	FIRSTWAITSYS,
	SECONDWAITSYS,
	THIRDWAITSYS,
}waitsys_t;

typedef uint8_t(*uwb_func_t)(uwb_t*);
typedef uint8_t(*send_tx_poll_msg_t)(uwb_t*);
typedef uint16_t(*uwbwaitsys_t)(uwb_t*,uint32_t *lo_result, uint32_t *hi_result, uint32_t lo_mask, uint32_t hi_mask,waitsys_t waitsys);


typedef struct{
	uwb_func_t						txrxmsg;
	send_tx_poll_msg_t			send_tx_poll_msg;
	uwbwaitsys_t						uwbwaitsys;
}struct_func_uwb_t;

typedef enum{
	BUZZER_NOTIFI_SUCCESS,
	BUZZER_NOTIFI_ERROR,
}uwb_buzzer_notification_t;


void uwb_init(struct_func_uwb_t *func_uwb,uwb_func_t txrxmsg,send_tx_poll_msg_t send_tx_poll_msg,uwbwaitsys_t uwbwaitsys);
void uwb_buzzer_notification(uwb_buzzer_notification_t buzzer,uint32_t ms);
void test_run_info(unsigned char *data);

#endif /* UWB_H_ */
