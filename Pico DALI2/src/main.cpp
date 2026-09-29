#include <Arduino.h>
#include "WS_DALI.h"
#include "WS_WIFI.h"

uint8_t uplight_addr = 255, downlight_addr = 255;

void setup()
{
	vTaskDelay(pdMS_TO_TICKS(10000));
	printf("Startup delay is complete!\r\n");

	WIFI_Init();
	printf("The WIFI initialization is complete!\r\n");

	DALI_Init();
	printf("DALI device scan is complete.\r\n");

	for( uint8_t d = DALI_NUM; d-- > 0;  )
	{
		if( DALI_Addr[d] == d ) 
		{
			if( uplight_addr == 255 ) uplight_addr = d;
			else 
			if( downlight_addr == 255 ) downlight_addr = d;
		}
	}
	printf( "Downlight is addr %x\n", downlight_addr );
	printf( "Uplight is addr %x\n", uplight_addr );
}

uint32_t Time_count=0;
uint32_t Time=0;
uint32_t Timeout=2;
uint8_t uplight, downlight;
uint8_t intensity = WS_DALI_LEVEL;
void loop()
{
	printf( "Count=%u\n", Time_count );
	WIFI_Loop();
	Timeout = 2000;
	if (DALI_Loop && DALI_NUM)
	{
		if( uplight_addr == 255 ) printf( "No uplight?\n" );
		else
		{
			uplight = Time_count & 0x1;
			printf( "Uplight state=%u\n", uplight );
			Luminaire_Brightness( intensity * uplight , uplight_addr );
		}
		if( downlight_addr == 255 ) printf( "No downlight?\n" );
		else
		{
			downlight = ( Time_count & 0x2 ) >> 1;
			printf( "Downlight state=%u\n", downlight );
			Luminaire_Brightness( intensity * downlight , downlight_addr );
		}
	}
	else
	{
		printf("DALI device scan found no devices.\r\n");
	}
	vTaskDelay(pdMS_TO_TICKS(Timeout));
	Time_count++;
}