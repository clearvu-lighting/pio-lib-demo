#include "cvl/configuration.h"
#include "Button.hpp"
#include <math.h>

static constexpr uint8_t MAG_SWITCH=47;
static constexpr uint8_t INDICATOR_LED=48;
static uint8_t loop_count = 0;
static bool led_state = false;
uint32_t last_trigger = 0;
uint32_t backoff_ms = 5000;
const char *tag = "MAG-SWITCH";

#define ISR_TEST
CVL::Button *mag_switch = NULL;

void setup()
{
    Serial.begin(115200);
	delay(5000);
	LOGW("Start");
	#ifndef ISR_TEST
	pinMode( MAG_SWITCH, INPUT_PULLDOWN );
	#else
	mag_switch = new CVL::Button( static_cast<gpio_num_t>(MAG_SWITCH), CVL::Button::INVERTED_FLAG );
	assert( mag_switch );
	LOGI("Button created");
	mag_switch->start();
	LOGI("Button started");
	#endif
    pinMode( INDICATOR_LED, OUTPUT );
	LOGW("Please press the button at GPIO %u.", MAG_SWITCH );
}

void loop()
{
	bool button_state = false;
	#ifndef ISR_TEST
	button_state = ( digitalRead( MAG_SWITCH ) != 0 );
	#else
	button_state = ( mag_switch->get_level() == 1 );
	#endif
	if( !button_state )
	{
		if( ( loop_count % 100 ) == 0 ) LOGI("Please press the button at GPIO %u.", MAG_SWITCH );
		delay(100);
	}
	else
	{
		led_state = !led_state;
		LOGI("led_state=%u",led_state);
		digitalWrite( INDICATOR_LED, led_state );
		loop_count = 0;
		delay( 1000 );
	}
	++loop_count;
}