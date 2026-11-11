/*
   This is to be used similar to the Meshtasic src/configuration.h file
   but it is scoped to only consider the src/cvl modules.
   This keeps changes in this from forcing a total recompile. 
*/

#pragma once

// We rely on the main configuraiton file for much.
#include <configuration.h>

#ifdef CLEAR_VU_MESHTASTIC
#define CVL_DALI_TX_GPIO 3
#define CVL_DALI_RX_GPIO 4
#define CVL_NTC_ADC_GPIO 6
#define CVL_SWITCH_GPIO 47
#define CVL_INDICATOR_LED_GPIO 48

#define CVL_NTC_ADC_POLL_MS 1000
#define CVL_NTC_ALARM_TEMP_C 30
#define CVL_NTC_ALARM_BACKOFF_MS 10000
#define CVL_DALI_MAX_DEVICE 4
#define CVL_DALI_HEARTBEAT_RATE_MS  5000
#define CVL_BLINK_DELAY_MS 500
#define CVL_FLASH_RATE_MS 100
#define CVL_HEARTBEAT_RATE_MS 5000
#define CVL_SWITCH_BACKOFF_MS 10000

#define CVL_HEARTBEAT_BLINKS 1
#define CVL_STARTUP_BLINKS 4
#define CVL_MESSAGE_BLINKS 4
#define CVL_SWITCH_BLINKS 6
#define CVL_NTC_ALARM_BLINKS 8
#endif