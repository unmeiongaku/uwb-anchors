#include "main.h"
#include "tim.h"
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
#include "user_define.h"

/* Example application name */
#define APP_NAME "DS TWR RESP v1.0"

#define RNG_DELAY_MS 30

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

/* Frames used in the ranging process. See NOTE 3 below. */

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

/* for take the initiator adress form poll msg */
		static uint8_t allMSGCOMMONLEN = 7; //tutaj
		static uint8_t initiatorAdress[] = {0,0};
/* Length of the common part of the message (up to and including the function code, see NOTE 3 below). */
		#define ALL_MSG_COMMON_LEN 10

		/* Index to access some of the fields in the frames involved in the process. */
		#define ALL_MSG_SN_IDX 2
		#define FINAL_MSG_POLL_TX_TS_IDX 10
		#define FINAL_MSG_RESP_RX_TS_IDX 14
		#define FINAL_MSG_FINAL_TX_TS_IDX 18

