#include <Arduino.h>
#include <Wire.h>
#include <math.h>

static constexpr uint8_t MAG_SWITCH=47;
static constexpr uint8_t INDICATOR_LED=48;
static uint8_t loop_count = 0;
static bool led_state = false;
uint32_t last_trigger = 0;
uint32_t backoff_ms = 5000;

void setup()
{
    Serial.begin(115200);
	pinMode( MAG_SWITCH, INPUT_PULLDOWN );
    pinMode( INDICATOR_LED, OUTPUT );
}

void loop()
{
	if( digitalRead( MAG_SWITCH ) != 0 )
	{
		if( ( loop_count % 10 ) == 0 ) Serial.printf("Please press the button at GPIO %u.\n", MAG_SWITCH );
		delay(100);
	}
	else
	{
		led_state = !led_state;
		Serial.printf("led_state=%u\n",led_state);
		digitalWrite( INDICATOR_LED, led_state );
		loop_count = 0;
		delay( 1000 );
	}
	++loop_count;
}