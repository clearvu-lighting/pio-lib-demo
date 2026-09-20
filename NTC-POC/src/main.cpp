#include <Arduino.h>
#include <cmath>
#include "ntc.h"

int ntc_pin = NTC_PIN;
const unsigned ntc_window_size = min(NTC_WINDOW_SIZE,NTC_WINDOW_SIZE_MAX);
unsigned readings[ntc_window_size];
unsigned i_readings = 0;

const float ref_voltage = NTC_REF_VOLTAGE;
const float ntc_ref_res = NTC_REF_RES;
const float ntc_div_res = NTC_DIV_RES;
const float ntc_ref_temp = NTC_REF_TEMP;
const float zero_kelvin = 273.15;
const float ntc_beta = NTC_BETA;
const float adc_max = ADC_MAX_VALUE;
const float ntc_temp_offset = NTC_TEMP_OFFSET;

unsigned average_adc_values( unsigned adc_value )
{
  readings[i_readings%ntc_window_size] = adc_value;
  unsigned long ntc_sum = 0;
  for( int i = 0; i < ntc_window_size; ++i ) ntc_sum += readings[i];
  return ++i_readings, (unsigned)( ntc_sum / ntc_window_size );
}

void setup()
{
  Serial.begin(115200);
  pinMode(ntc_pin,INPUT);
}

void loop()
{
  unsigned adc_value = analogRead(ntc_pin);
  unsigned avg_adc_value = average_adc_values( adc_value );
  float voltage = avg_adc_value * ( NTC_REF_VOLTAGE / ADC_MAX_VALUE );
  float res = ntc_div_res / ( adc_max / avg_adc_value - 1.0 ) / 2;
  float kelvin = 1.0 / ( ( log( res / ntc_ref_res ) / ntc_beta ) + ( 1.0 / ( ntc_ref_temp + zero_kelvin ) ) );
  float temp_c = kelvin - zero_kelvin + ntc_temp_offset;
  if( i_readings%ntc_window_size == 0 ) Serial.printf
  (
    "count = %u, ACD Value = %05d, Avg = %05d, voltage=%f, Resistance = %f, Offset C = %f, Temp C = %f\n", 
    i_readings, adc_value, avg_adc_value, voltage, res, ntc_temp_offset, temp_c
  );
  vTaskDelay( pdMS_TO_TICKS(NTC_RATE_MS) );
}

void loop_save2()
{
  unsigned adc_value = analogRead(ntc_pin);
  unsigned avg_adc_value = average_adc_values( adc_value );
  float voltage = avg_adc_value * ( NTC_REF_VOLTAGE / ADC_MAX_VALUE );
  float res = NTC_DIV_RES * ( NTC_REF_VOLTAGE / voltage - 1.0 );
  float logRes = log(res);
  float kelvin = 1.0 / ( ( logRes / ntc_beta ) + ( 1.0 / ( ntc_ref_temp + zero_kelvin ) ) );
  float temp_c = kelvin - zero_kelvin + ntc_temp_offset;
  if( i_readings%ntc_window_size == 0 ) Serial.printf
  (
    "count = %u, ACD Value = %05d, Avg = %05d, voltage=%f, Resistance = %f, Offset C = %f, Temp C = %f\n", 
    i_readings, adc_value, avg_adc_value, voltage, res, ntc_temp_offset, temp_c
  );
  vTaskDelay( pdMS_TO_TICKS(NTC_RATE_MS) );
}