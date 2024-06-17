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
#include "math.h"

/*
 * Calculate RSSI
 */
float getRSSI(uint32_t C, uint16_t N, uint8_t D, float A)
{
	float rssi;
	rssi = 10 * log10( (C * pow(2.0, 21.0))/pow((double) N, 2.0) ) + (6*D) - A;
	return rssi;
}



