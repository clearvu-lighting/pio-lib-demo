#include "ntc.hpp"

void setup()
{
    Serial.begin(115200);
    cvl::setup();
}

void loop()
{
    const auto temperature = cvl::read();

    if (temperature.status == cvl::Status::Ok)
	{
        Serial.printf(
            "Temp: %.2f C / %.2f F, NTC = %.3f Kohm, ADC = %u\n",
            temperature.celsius,
            temperature.fahrenheit,
            temperature.ntcOhms/1000,
            temperature.rawCounts
        );
    }
	else
	{
        Serial.printf
		(
            "Temperature sensor fault: %s, ADC = %u\n",
            cvl::statusText(temperature.status),
            temperature.rawCounts
        );
    }

    delay(1000);
}