#include <Arduino.h>
#include <Wire.h>
#include <math.h>

// For ESP32, set these to the pins actually wired on your board.
// Generic ESP32 defaults are typically SDA=21 and SCL=22.
constexpr int I2C_SDA_PIN = SDA_PIN;
constexpr int I2C_SCL_PIN = SCL_PIN;
constexpr int USER_BUTTON = 0;

static constexpr uint8_t LED_COUNT = 4;
static constexpr uint8_t led_list[LED_COUNT] = { 3, 4, 17, 33 };
static uint8_t led_state[LED_COUNT] = { 1, 1, 1, 1 };
static uint8_t loop_count = 0;

void device_found( byte address )
{
	Serial.print("  Device found at 0x");
	if (address < 0x10)
	{
		Serial.print('0');
	}
	Serial.println(address, HEX);
}

void report_error( byte address, byte error )
{
	Serial.printf("  Error %x at %x\n", error, address );
}

void scanI2CBus()
{
	byte error;
	byte address;
	int devicesFound = 0;

	Serial.println();
	Serial.println("Scanning I2C bus...");

	// Valid 7-bit I2C addresses are normally 0x08 through 0x77.
	for (address = 0x08; address <= 0x77; address++)
	{
		Wire.beginTransmission(address);
		error = Wire.endTransmission();

		if (error == 0)
		{
			device_found( address );
			devicesFound++;
		}
		else if( error != 2 )
		{
			report_error( address, error );
		}
	}
	if (devicesFound == 0)
	{
		Serial.println("No I2C devices found.");
	}
	else
	{
		Serial.print("Scan complete. Devices found: ");
		Serial.println(devicesFound);
	}
}

void setup()
{
    Serial.begin(115200);
	pinMode( USER_BUTTON, INPUT_PULLUP );
    pinMode( VEXT_CTRL, OUTPUT );
	for( uint8_t l; digitalRead( USER_BUTTON ) != 0; ++l )
	{
		if( ( l % 10 ) == 0 ) Serial.printf("You must press the button at GPIO %u\n", USER_BUTTON );
		delay(100);
	}
	
    Serial.println("Turn on VEXT power.\n");
    digitalWrite( VEXT_CTRL, 0 );
    delay(500);

    // ESP32: explicit pin assignment is recommended.
    Serial.println("Start I2C system.\n");
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, 100000);
    Serial.println("I2C Scanner started...");

	for( uint8_t i = 0; i < LED_COUNT; ++i ) pinMode( led_list[i], OUTPUT );
}

void loop()
{
	uint8_t led_id = loop_count % LED_COUNT;
	digitalWrite( led_list[led_id], led_state[led_id] );
	led_state[led_id] = static_cast<uint8_t>( rand() & 0x1 );
	if( !( loop_count % ( LED_COUNT << 2 ) ) ) scanI2CBus();
	delay(500);
	++loop_count;
}