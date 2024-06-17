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
#include "deca_probe_interface.h"



/* Hold the amount of errors that have occurred */
static uint32_t errors[23] = { 0 };

extern dwt_config_t config_options;
extern dwt_txconfig_t txconfig_options;
extern dwt_txconfig_t txconfig_options_ch9;

#if (ANCHOR_TYPE == 'A')
		/*Create a tx_poll_msg send from Anchor A to Tag*/
		static uint8_t rx_poll_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'A', 1,'T', 'U', 0xE0, 0, 0};
		/*After send tx_poll_msg tp tag, Anchor A receive a rx_resp_msg from Tag*/
		static uint8_t tx_resp_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'T', 'U', 'A', 1, 0xE1, 0, 0};
		/*Receive final  Distance and Tof of Tag to Anchor A to display on LCD */
		static uint8_t rx_final_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'A', 1, 'T', 'U', 0xE2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#elif (ANCHOR_TYPE == 'B')
		/*Create a tx_poll_msg send from Anchor B to Tag*/
		static uint8_t rx_poll_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'B', 2,'T', 'U', 0xE0, 0, 0};
		/*After send tx_poll_msg tp tag, Anchor B receive a rx_resp_msg from Tag*/
		static uint8_t tx_resp_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'T', 'U', 'B', 2, 0xE1, 0, 0};
		/*Receive final  Distance and Tof of Tag to Anchor B display on LCD */
		static uint8_t rx_final_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'B', 2, 'T', 'U', 0xE2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#elif (ANCHOR_TYPE == 'C')
		/*Create a tx_poll_msg send from Anchor B to Tag*/
		static uint8_t rx_poll_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'C', 3,'T', 'U', 0xE0, 0, 0};
		/*After send tx_poll_msg tp tag, Anchor B receive a rx_resp_msg from Tag*/
		static uint8_t tx_resp_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'T', 'U', 'C', 3, 0xE1, 0, 0};
		/*Receive final  Distance and Tof of Tag to Anchor C display on LCD */
		static uint8_t rx_final_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'C', 3, 'T', 'U', 0xE2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#elif (ANCHOR_TYPE == 'D')
		/*Create a tx_poll_msg send from Anchor D to Tag*/
		static uint8_t rx_poll_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'D', 4,'T', 'U', 0xE0, 0, 0};
		/*After send tx_poll_msg tp tag, Anchor D receive a rx_resp_msg from Tag*/
		static uint8_t tx_resp_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'T', 'U', 'D', 4, 0xE1, 0, 0};
		/*Receive final  Distance and Tof of Tag to Anchor D display on LCD */
		static uint8_t rx_final_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'D', 4, 'T', 'U', 0xE2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif

/* Length of the common part of the message (up to and including the function code, see NOTE 3 below). */
#define ALL_MSG_COMMON_LEN 10
/* Index to access some of the fields in the frames involved in the process. */
#define ALL_MSG_SN_IDX            2
#define FINAL_MSG_POLL_TX_TS_IDX  10
#define FINAL_MSG_RESP_RX_TS_IDX  14
#define FINAL_MSG_FINAL_TX_TS_IDX 18

#define RX_BUF_LEN 30//Must be less than FRAME_LEN_MAX_EX
static uint8_t rx_buffer[RX_BUF_LEN];

#if NUMBER_OF_TAG!=1
/* for take the initiator adress from poll msg */
		static uint8_t allMSGCOMMONLEN = 7; //tutaj
		static uint8_t initiatorAdress[] = {0,0};
#endif
/*
 * 128-bit STS key to be programmed into CP_KEY register.
 *
 * This key needs to be known and programmed the same at both units performing the SS-TWR.
 * In a real application for security this would be private and unique to the two communicating units
 * and chosen/assigned in a secure manner lasting just for the period of their association.
 *
 * Here we use a default KEY as specified in the IEEE 802.15.4z annex
 */
static dwt_sts_cp_key_t cp_key = { 0x14EB220F, 0xF86050A8, 0xD1D336AA, 0x14148674 };

/*
 * 128-bit initial value for the nonce to be programmed into the CP_IV register.
 *
 * The IV, like the key, needs to be known and programmed the same at both units performing the SS-TWR.
 * It can be considered as an extension of the KEY. The low 32 bits of the IV is the counter.
 * In a real application for any particular key the value of the IV including the count should not be reused,
 * i.e. if the counter value wraps the upper 96-bits of the IV should be changed, e.g. incremented.
 *
 * Here we use a default IV as specified in the IEEE 802.15.4z annex
 */
static dwt_sts_cp_iv_t cp_iv = { 0x1F9A3DE4, 0xD37EC3CA, 0xC44FA8FB, 0x362EEB34 };


/* Default antenna delay values for 64 MHz PRF. See NOTE 2 below. */
#if  ANCHOR_TYPE == 'A'
	#define ANT_DELAY 16385//16550
#elif ANCHOR_TYPE == 'B'
	#define ANT_DELAY 16385//16550
#elif ANCHOR_TYPE == 'C'
	#define ANT_DELAY 16385
#elif ANCHOR_TYPE == 'D'
	#define ANT_DELAY 16385
#endif

/* Default antenna delay values for 64 MHz PRF. See NOTE 2 below. */
#define TX_ANT_DLY  ANT_DELAY  //16535//16525
#define RX_ANT_DLY  ANT_DELAY //16535//16525

/* Delay between frames, in UWB microseconds. See NOTE 1 below. */
#define POLL_RX_TO_RESP_TX_DLY_UUS (500 + CPU_PROCESSING_TIME)

#define RESP_TX_TO_FINAL_RX_DLY_UUS (100 + CPU_PROCESSING_TIME)


static event_status_t proobject_state_handle_RESET_DWIC_SM(proobject_t *const mobj, event_t const *const e);
static event_status_t proobject_state_handle_INIT_SM(proobject_t *const mobj, event_t const *const e);
static event_status_t proobject_state_handle_ANCHOR_RX_POLLING_CHECKING_SM(proobject_t *const mobj, event_t const *const e);
static event_status_t proobject_state_handle_ANCHOR_FALSE_POLLING_CHECKING_SM(proobject_t *const mobj, event_t const *const e);
static event_status_t proobject_state_handle_ANCHOR_CLASSIFY_RX_BUFFER_SM(proobject_t *const mobj, event_t const *const e);
static event_status_t proobject_state_handle_ANCHOR_OVERLOAD_RX_BUFER_SM(proobject_t *const mobj, event_t const *const e);
static event_status_t proobject_state_handle_ANCHOR_RX_POLL_MSG_SM(proobject_t *const mobj, event_t const *const e);
static event_status_t  proobject_state_handle_ANCHOR_RX_FINAL_MSG_SM(proobject_t *const mobj, event_t const *const e);
static event_status_t proobject_state_handle_ANCHOR_RX_NO_MSG_SM(proobject_t *const mobj, event_t const *const e);
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
	mobj->status_reg = 0;
	mobj->frame_seq_nb = 0;
    /* Configure SPI rate, DW3000 supports up to 36 MHz */
#ifdef CONFIG_SPI_FAST_RATE
    port_set_dw_ic_spi_fastrate();
#endif /* CONFIG_SPI_FAST_RATE */
#ifdef CONFIG_SPI_SLOW_RATE
    port_set_dw_ic_spi_slowrate();
#endif /* CONFIG_SPI_SLOW_RATE */

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
		case ANCHOR_RX_POLLING_CHECKING_SM:
		{
			return proobject_state_handle_ANCHOR_RX_POLLING_CHECKING_SM(mobj,e);
		}
			break;
		case ANCHOR_FALSE_POLLING_CHECKING_SM:
		{
			return proobject_state_handle_ANCHOR_FALSE_POLLING_CHECKING_SM(mobj,e);
		}
			break;
		case ANCHOR_CLASSIFY_RX_BUFFER_SM:
		{
			return proobject_state_handle_ANCHOR_CLASSIFY_RX_BUFFER_SM(mobj,e);
		}
		case ANCHOR_OVERLOAD_RX_BUFER_SM:
		{
			return proobject_state_handle_ANCHOR_OVERLOAD_RX_BUFER_SM(mobj,e);
		}
		case ANCHOR_RX_POLL_MSG_SM:
		{
			return proobject_state_handle_ANCHOR_RX_POLL_MSG_SM(mobj,e);
		}
		case ANCHOR_RX_FINAL_MSG_SM:
		{
			return proobject_state_handle_ANCHOR_RX_FINAL_MSG_SM(mobj,e);
		}
		case ANCHOR_RX_NO_MSG_SM:
		{
			return proobject_state_handle_ANCHOR_RX_NO_MSG_SM(mobj,e);
		}
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
		    /* Probe for the correct device driver. */
		    dwt_probe((struct dwt_probe_s *)&dw3000_probe_interf);
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
				nextstatesig = NEXT_SIG_DEFINE;
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
			    /* Enabling LEDs here for debug so that for each TX the D1 LED will flash on DW3000 red eval-shield boards.
			     * Note, in real low power applications the LEDs should not be used. */
			    dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK);
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
					    /* Next can enable TX/RX states output on GPIOs 5 and 6 to help diagnostics, and also TX/RX LEDs */
					    dwt_setlnapamode(DWT_LNA_ENABLE | DWT_PA_ENABLE);
						/*Delay Between the response frame and final frame*/
						dwt_setrxaftertxdelay(RESP_TX_TO_FINAL_RX_DLY_UUS);
					/*Enable IC diagnostic calculation and logging*/
					dwt_configciadiag(DW_CIA_DIAG_LOG_ALL);
					nextstatesig = NEXT_SIG_DEFINE;
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
			mobj->active_state = ANCHOR_RX_POLLING_CHECKING_SM;
			return EVENT_TRANSITION;
		}
	}
	return EVENT_IGNORED;
}


static event_status_t proobject_state_handle_ANCHOR_RX_POLLING_CHECKING_SM(proobject_t *const mobj, event_t const *const e){
	switch(e->sig){
		case ENTRY:
		{
			mobj->check_hi_mask = false;
			mobj->check_lo_mask = false;
			mobj->firstcheck = true;
			mobj->error = false;
			if(!mobj->messageFlag){
				if(!mobj->loopCount){
					/*
					 * On first loop, configure the STS key & IV, then load them.
					 */
					dwt_configurestskey(&cp_key);
					dwt_configurestsiv(&cp_iv);
					dwt_configurestsloadiv();
				}
				else{
					/*
					 * On subsequent loops, we only need to reload the lower 32 bits of STS IV.
					 */
					dwt_configurestsiv(&cp_iv);
					dwt_configurestsloadiv();
				}
			}
			if (!mobj->messageFlag)
			{
				mobj->loopCount++; // increment the loop count only when starting ranging exchange
				/* Activate reception immediately. */
				dwt_rxenable(DWT_START_RX_IMMEDIATE);
			}
			return EVENT_HANDLED;
		}
		case EXIT:
		{
			return EVENT_HANDLED;
		}
		case NEXT_SIG:
		{
			mobj->active_state = ANCHOR_CLASSIFY_RX_BUFFER_SM;
			return EVENT_TRANSITION;
		}
		case FALSE_SIG:
		{
			mobj->active_state = ANCHOR_FALSE_POLLING_CHECKING_SM;
			return EVENT_TRANSITION;
		}
		case TICK_SIG:
		{
			uint8_t rxpolling = 0;
			static uint16_t rxpollcnt = 0;

			/* Poll for reception of a frame or error/timeout. See NOTE 6 below. */
			/*Checking for 100 time*/
			if(rxpollcnt<100){
				rxpolling = mywaitforsysstatus(mobj,&mobj->status_reg, NULL, (DWT_INT_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR), 0);
				rxpollcnt++;
			}
			else{
				rxpollcnt = 0;
				rxpolling=1;
				mobj->error = true;
			}
			/*
			 * Need to check the STS has been received and is good.
			 */
			mobj->goodSts = dwt_readstsquality(&mobj->stsQual);
			if ((rxpolling==1) && (mobj->status_reg & DWT_INT_RXFCG_BIT_MASK) && (mobj->goodSts >= 0) && (dwt_readstsstatus(&mobj->stsStatus, 0) == DWT_SUCCESS)){
				 nextstatesig = NEXT_SIG_DEFINE;
			}
			else if(rxpolling==1 && (mobj->goodSts < 0)){
				if((dwt_readstsstatus(&mobj->stsStatus, 0) != DWT_SUCCESS) | (!(mobj->status_reg & DWT_INT_RXFCG_BIT_MASK))){
					nextstatesig = FALSE_SIG_DEFINE;
				}
			}
			else if(mobj->error==true){
				nextstatesig = FALSE_SIG_DEFINE;
			}
			return EVENT_HANDLED;
		}
	}
	return EVENT_IGNORED;
}


static event_status_t proobject_state_handle_ANCHOR_FALSE_POLLING_CHECKING_SM(proobject_t *const mobj, event_t const *const e){
	switch(e->sig){
	case ENTRY:
	{
		check_for_status_errors(mobj->status_reg, errors);
        if (!(mobj->status_reg & DWT_INT_RXFCG_BIT_MASK))
        {
            errors[BAD_FRAME_ERR_IDX] += 1;
        }
        if (mobj->goodSts < 0)
        {
            errors[PREAMBLE_COUNT_ERR_IDX] += 1;
        }
        if (mobj->stsQual <= 0)
        {
            errors[CP_QUAL_ERR_IDX] += 1;
        }
        /* Clear RX error events in the DW IC status register. */
        dwt_writesysstatuslo(SYS_STATUS_ALL_RX_ERR);

        /*
         * If any error occurs, we can reset the STS count back to default value.
         */
        mobj->messageFlag = 0;
		return EVENT_HANDLED;
	}
	case EXIT:
	{
		return EVENT_HANDLED;
	}
	case NEXT_SIG:
	{
		mobj->active_state = ANCHOR_RX_POLLING_CHECKING_SM;
		return EVENT_TRANSITION;
	}
	}
	return EVENT_IGNORED;
}

static event_status_t proobject_state_handle_ANCHOR_CLASSIFY_RX_BUFFER_SM(proobject_t *const mobj, event_t const *const e){
	switch(e->sig){
		case ENTRY:
		{
			mobj->check_hi_mask = false;
			mobj->check_lo_mask = false;
			mobj->firstcheck = true;
            /* Clear good RX frame event in the DW IC status register. */
            dwt_writesysstatuslo(DWT_INT_RXFCG_BIT_MASK);

            /* A frame has been received, read it into the local buffer. */
			mobj->frame_len = dwt_getframelength();
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
            if (mobj->frame_len <= sizeof(rx_buffer))
            {
           	 dwt_readrxdata(rx_buffer, mobj->frame_len, 0);
                /* Check that the frame is a poll sent by "SS TWR initiator STS" example.
                 * As the sequence number field of the frame is not relevant, it is cleared to simplify the validation of the frame. */
           	 rx_buffer[ALL_MSG_SN_IDX] = 0;
			 if (memcmp(rx_buffer, rx_poll_msg, ALL_MSG_COMMON_LEN) == 0)
			 {

			 }
			 else if (memcmp(rx_buffer, rx_final_msg, ALL_MSG_COMMON_LEN) == 0)
			 {

			 }
			 else{
				errors[BAD_FRAME_ERR_IDX] += 1;
				/*
				 * If any error occurs, we can reset the STS count back to default value.
				 */
				mobj->messageFlag = 0;
			 }
            }
            else{
            	nextstatesig = FALSE_SIG_DEFINE;
            }

			return EVENT_HANDLED;
		}
	}
	return EVENT_IGNORED;
}

static event_status_t proobject_state_handle_ANCHOR_OVERLOAD_RX_BUFER_SM(proobject_t *const mobj, event_t const *const e){
	return EVENT_IGNORED;
}

static event_status_t proobject_state_handle_ANCHOR_RX_POLL_MSG_SM(proobject_t *const mobj, event_t const *const e){
	switch(e->sig){
		case ENTRY:
		{
#if NUMBER_OF_TAG!=1
 				initiatorAdress[0] = rx_buffer[7]; //tutaj
 				initiatorAdress[1] = rx_buffer[8];
 				allMSGCOMMONLEN = 10;
				 // set current initiator address
				tx_resp_msg[5] = initiatorAdress[0];
				tx_resp_msg[6] = initiatorAdress[1];
				//
				rx_poll_msg[7] = initiatorAdress[0];
				rx_poll_msg[8] = initiatorAdress[1];
				//
				rx_final_msg[7] = initiatorAdress[0];
				rx_final_msg[8] = initiatorAdress[1];
#endif
                /* Retrieve poll reception timestamp. */
				mobj->poll_rx_ts = get_rx_timestamp_u64();
				mobj->resp_tx_time = (mobj->poll_rx_ts                                               /* Received timestamp value */
                                   + ((POLL_RX_TO_RESP_TX_DLY_UUS                        /* Set delay time */
                                          + get_rx_delay_time_data_rate()                /* Added delay time for data rate set */
                                          + get_rx_delay_time_txpreamble()               /* Added delay for TX preamble length */
                                          + ((1 << (config_options.stsLength + 2)) * 8)) /* Added delay for STS length */
                                       * UUS_TO_DWT_TIME))
                               >> 8; /* Converted to time units for chip */
                dwt_setdelayedtrxtime(mobj->resp_tx_time);
                /* Response TX timestamp is the transmission time we programmed plus the antenna delay. */
                mobj->resp_tx_ts = (((uint64_t)(mobj->resp_tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;

                /* Write and send the response message. See NOTE 9 below. */
                tx_resp_msg[ALL_MSG_SN_IDX] = mobj->frame_seq_nb;
                dwt_writesysstatuslo(DWT_INT_TXFRS_BIT_MASK);
                dwt_writetxdata(sizeof(tx_resp_msg), tx_resp_msg, 0); /* Zero offset in TX buffer. */
                dwt_writetxfctrl(sizeof(tx_resp_msg), 0, 1);          /* Zero offset in TX buffer, ranging. */

                /*
                 * As described above, we will be delaying the transmission of the RESP message
                 * with a set value that is also with reference to the timestamp of the received
                 * POLL message.
                 */
                dwt_setrxaftertxdelay(100); // receiver can be delayed as Final message will not come immediately
                mobj->ret = dwt_starttx(DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED);
			return EVENT_HANDLED;
		}
		case EXIT:
		{
			return EVENT_HANDLED;
		}
		case TICK_SIG:
		{
			uint8_t txsent = 0;
			/* If dwt_starttx() returns an error, abandon this ranging exchange and proceed to the next one. See NOTE 10 below. */
			if (mobj->ret == DWT_SUCCESS){
                /* Poll DW IC until TX frame sent event set. See NOTE 6 below. */
				txsent = (mywaitforsysstatus(mobj,NULL, NULL, DWT_INT_TXFRS_BIT_MASK, 0));
				if(txsent!=0){
                    /* Clear TXFRS event. */
                    dwt_writesysstatuslo(DWT_INT_TXFRS_BIT_MASK);
                    /* Increment frame sequence number after transmission of the poll message (modulo 256). */
                    mobj->frame_seq_nb++;
                    /*
                     * This flag is set high here so that we do not reset the STS count before receiving
                     * the final message from the initiator. Otherwise, the STS count would be bad and
                     * we would be unable to receive it.
                     */
                    mobj->messageFlag = 1;
				}
			}
			return EVENT_HANDLED;
		}
	}

	return EVENT_IGNORED;
}
static event_status_t  proobject_state_handle_ANCHOR_RX_FINAL_MSG_SM(proobject_t *const mobj, event_t const *const e){
	return EVENT_IGNORED;
}
static event_status_t proobject_state_handle_ANCHOR_RX_NO_MSG_SM(proobject_t *const mobj, event_t const *const e){
	return EVENT_IGNORED;
}


/*****************************************************************************************************************************************************
 * NOTES:
 *
 * 1. The double-sided two-way ranging scheme implemented here has to be considered carefully as the accuracy of the distance measured is highly
 *    sensitive to the clock offset error between the devices and the length of the response delay between frames. To achieve the best possible
 *    accuracy, this response delay must be kept as low as possible. In order to do so, 6.8 Mbps data rate is advised in this example and the response
 *    delay between frames is defined as low as possible. The user is referred to User Manual for more details about the double-sided two-way ranging
 *    process.
 *
 *    Initiator: |Poll TX| ..... |Resp RX| ........ |Final TX|
 *    Responder: |Poll RX| ..... |Resp TX| ........ |Final RX|
 *                   ^|P RMARKER|                                    - time of Poll TX/RX
 *                                   ^|R RMARKER|                    - time of Resp TX/RX
 *                                                      ^|R RMARKER| - time of Final TX/RX
 *
 *                       <--TDLY->                                   - POLL_TX_TO_RESP_RX_DLY_UUS (RDLY-RLEN)
 *                               <-RLEN->                            - RESP_RX_TIMEOUT_UUS   (length of poll frame)
 *                    <----RDLY------>                               - POLL_RX_TO_RESP_TX_DLY_UUS (depends on how quickly responder
 *                                                                                                                      can turn around and reply)
 *
 *
 *                                        <--T2DLY->                 - RESP_TX_TO_FINAL_RX_DLY_UUS (R2DLY-FLEN)
 *                                                  <-FLEN--->       - FINAL_RX_TIMEOUT_UUS   (length of response frame)
 *                                    <----RDLY--------->            - RESP_RX_TO_FINAL_TX_DLY_UUS (depends on how quickly initiator
 *                                                                                                                      can turn around and reply)
 *
 *
 * 2. The sum of the values is the TX to RX antenna delay, experimentally determined by a calibration process. Here we use a hard coded typical value
 *    but, in a real application, each device should have its own antenna delay properly calibrated to get the best possible precision when performing
 *    range measurements.
 * 3. The frames used here are Decawave specific ranging frames, complying with the IEEE 802.15.4 standard data frame encoding. The frames are the
 *    following:
 *     - a poll message sent by the initiator to trigger the ranging exchange.
 *     - a response message sent by the responder to complete the exchange and provide all information needed by the initiator to compute the
 *       time-of-flight (distance) estimate.
 *    The first 10 bytes of those frame are common and are composed of the following fields:
 *     - byte 0/1: frame control (0x8841 to indicate a data frame using 16-bit addressing).
 *     - byte 2: sequence number, incremented for each new frame.
 *     - byte 3/4: PAN ID (0xDECA).
 *     - byte 5/6: destination address, see NOTE 4 below.
 *     - byte 7/8: source address, see NOTE 4 below.
 *     - byte 9: function code (specific values to indicate which message it is in the ranging process).
 *    The remaining bytes are specific to each message as follows:
 *    Poll message:
 *     - no more data
 *    Response message:
 *     - byte 10 -> 13: poll message reception timestamp.
 *     - byte 14 -> 17: response message transmission timestamp.
 *    All messages end with a 2-byte checksum automatically set by DW IC.
 * 4. Source and destination addresses are hard coded constants in this example to keep it simple but for a real product every device should have a
 *    unique ID. Here, 16-bit addressing is used to keep the messages as short as possible but, in an actual application, this should be done only
 *    after an exchange of specific messages used to define those short addresses for each device participating to the ranging exchange.
 * 5. In a real application, for optimum performance within regulatory limits, it may be necessary to set TX pulse bandwidth and TX power, (using
 *    the dwt_configuretxrf API call) to per device calibrated values saved in the target system or the DW IC OTP memory.
 * 6. We use polled mode of operation here to keep the example as simple as possible but all status events can be used to generate interrupts. Please
 *    refer to DW IC User Manual for more details on "interrupts". It is also to be noted that STATUS register is 5 bytes long but, as the event we
 *    use are all in the first bytes of the register, we can use the simple dwt_read32bitreg() API call to access it instead of reading the whole 5
 *    bytes.
 * 7. As we want to send final TX timestamp in the final message, we have to compute it in advance instead of relying on the reading of DW IC
 *    register. Timestamps and delayed transmission time are both expressed in device time units so we just have to add the desired response delay to
 *    response RX timestamp to get final transmission time. The delayed transmission time resolution is 512 device time units which means that the
 *    lower 9 bits of the obtained value must be zeroed. This also allows to encode the 40-bit value in a 32-bit words by shifting the all-zero lower
 *    8 bits.
 * 8. In this operation, the high order byte of each 40-bit timestamps is discarded. This is acceptable as those time-stamps are not separated by
 *    more than 2**32 device time units (which is around 67 ms) which means that the calculation of the round-trip delays (needed in the
 *    time-of-flight computation) can be handled by a 32-bit subtraction.
 * 9. dwt_writetxdata() takes the full size of the message as a parameter but only copies (size - 2) bytes as the check-sum at the end of the frame is
 *    automatically appended by the DW IC. This means that our variable could be two bytes shorter without losing any data (but the sizeof would not
 *    work anymore then as we would still have to indicate the full length of the frame to dwt_writetxdata()).
 * 10. When running this example on the Dw3000 eval board platform with the POLL_RX_TO_RESP_TX_DLY response delay provided, the dwt_starttx() is always
 *     successful. However, in cases where the delay is too short (or something else interrupts the code flow), then the dwt_starttx() might be issued
 *     too late for the configured start time. The code below provides an example of how to handle this condition: In this case it abandons the
 *     ranging exchange and simply goes back to awaiting another poll message. If this error handling code was not here, a late dwt_starttx() would
 *     result in the code flow getting stuck waiting subsequent RX event that will will never come. The companion "initiator" example (ex_06a) should
 *     timeout from awaiting the "response" and proceed to send another poll in due course to initiate another ranging exchange.
 * 11. The user is referred to DecaRanging ARM application (distributed with EVK1000 product) for additional practical example of usage, and to the
 *     DW IC API Guide for more details on the DW IC driver functions.
 * 12. In this example, the DW IC is put into IDLE state after calling dwt_initialise(). This means that a fast SPI rate of up to 20 MHz can be used
 *     thereafter.
 * 13. This example uses STS with a packet configuration of mode 1 which looks like so:
 *    ---------------------------------------------------
 *    | Ipatov Preamble | SFD | STS | PHR | PHY Payload |
 *    ---------------------------------------------------
 *    There is a possibility that the TX and RX units in this example will go out of sync as their STS IV values may be misaligned. The STS IV value
 *    changes upon each receiving and transmitting event by the chip. While the TX and RX devices in this example start at the same STS IV values, it
 *    is possible that they can go out sync if a signal is not received correctly, devices are out of range, etc. To combat this, the 'poll message'
 *    that the initiator sends to the responder contains a plain-text STS counter value. The responder receives this message and first checks if
 *    the received frame is out of sync with it's own counter. If so, it will use this received counter value to update it's own counter. When out
 *    of sync with each other, the STS will not align correctly - thus we get no secure timestamp values.
 * 14. Desired configuration by user may be different to the current programmed configuration. dwt_configure is called to set desired
 *     configuration.
 * 15. In this operation, the high order byte of each 40-bit timestamps is discarded. This is acceptable as those time-stamps are not separated by
 *     more than 2**32 device time units (which is around 67 ms) which means that the calculation of the round-trip delays (needed in the
 *     time-of-flight computation) can be handled by a 32-bit subtraction.
 * 16. This example will set the STS key and IV upon each iteration of the main while loop. While this has the benefit of keeping the STS count in
 *     sync with the responder device (which does the same), it should be noted that this is not a 'secure' implementation as the count is reset upon
 *     each iteration of the loop. An attacker could potentially recognise this pattern if the signal was being monitored. While it serves it's
 *     purpose in this simple example, it should not be utilised in any final solution.
 ****************************************************************************************************************************************************/
