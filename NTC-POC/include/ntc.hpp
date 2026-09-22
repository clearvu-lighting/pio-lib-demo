#pragma once

#include <Arduino.h>
#include <math.h>

namespace cvl
{

// Schematic: NTC from 3V3 to divider node; R5 from divider node to GND.
// Divider node -> R30 (220R) -> ESP32-S3 GPIO7 / NTC_ADC.
constexpr gpio_num_t NTC_ADC_GPIO = GPIO_NUM_6;

constexpr float R_FIXED_OHMS = 10000.0f;  // R5
constexpr float NTC_R25_OHMS = 10000.0f;  // PANW 103395-395
constexpr float NTC_BETA_K   = 3950.0f;
constexpr float T25_K        = 298.15f;
constexpr float ADJUST_K     = 0.0f;
//                           = 3.0f;

constexpr uint16_t ADC_MAX_COUNTS    = 4095;
constexpr uint32_t REF_MILLIVOLTS    = 3300; 
constexpr uint16_t ADC_ADJUST        = 0; // 166;
constexpr uint8_t  SAMPLE_COUNT      = 32;
constexpr uint32_t SAMPLE_RATE_MICRO = 100;

// Conservative circuit/wiring diagnosis thresholds.
// Tune after collecting actual raw readings on production hardware.
constexpr uint16_t ADC_LOW_FAULT_COUNTS  = 15;
constexpr uint16_t ADC_HIGH_FAULT_COUNTS = 4080;

enum class Status : uint8_t
{
    Ok,
    OpenOrLowSideFault,
    ShortTo3V3OrOverrange,
    Invalid
};

struct Reading
{
    Status status;
    uint16_t rawCounts;
    uint32_t rawMillivolts;
    float ntcOhms;
    float ntcOhmsCal;
    float celsius;
    float celsiusCal;
    float fahrenheit;
    float fahrenheitCal;
};

inline const char* statusText(Status status)
{
    switch (status) {
        case Status::Ok:
            return "OK";
        case Status::OpenOrLowSideFault:
            return "NTC open or node shorted low";
        case Status::ShortTo3V3OrOverrange:
            return "NTC shorted high or ADC overrange";
        default:
            return "invalid";
    }
}

void setup()
{
    analogReadResolution(12);

    // ESP32 Arduino core function. GPIO 7 is an ADC-capable input on ESP32-S3.
    // 11 dB is intended to cover the approximately 0 to 3.3 V NTC range.
	#if defined(ESP32)
    analogSetPinAttenuation(static_cast<uint8_t>(NTC_ADC_GPIO), ADC_11db);
	#endif

    pinMode(static_cast<uint8_t>(NTC_ADC_GPIO), INPUT);
}

inline void readRawAveraged(Reading &reading)
{
    reading = {};

    // Discard the first conversion after switching from another ADC input.
    analogRead(NTC_ADC_GPIO);
    analogReadMilliVolts(NTC_ADC_GPIO);

    uint32_t sum = 0, sumMillivolts = 0;
    for (uint8_t i = 0; i < SAMPLE_COUNT; ++i)
	{
        sum += analogRead(NTC_ADC_GPIO) + ADC_ADJUST;
        sumMillivolts += analogReadMilliVolts(NTC_ADC_GPIO);
        delayMicroseconds(SAMPLE_RATE_MICRO);
    }

    //turn static_cast<uint16_t>((sum + SAMPLE_COUNT / 2) / SAMPLE_COUNT);
    reading.rawCounts = static_cast<uint16_t>(sum / SAMPLE_COUNT);
    reading.rawMillivolts = static_cast<uint32_t>(sumMillivolts / SAMPLE_COUNT);
}

inline Reading read()
{
    Reading result {};
    readRawAveraged(result);

    // In this circuit, an open probe produces a low ADC value because R5
    // pulls NTC_ADC to ground.
    if (result.rawCounts <= ADC_LOW_FAULT_COUNTS) {
        result.status = Status::OpenOrLowSideFault;
        result.ntcOhms = INFINITY;
        result.ntcOhmsCal = INFINITY;
        result.celsius = result.celsiusCal = NAN;
        result.fahrenheit = result.fahrenheitCal = NAN;
        return result;
    }

    // A near-full-scale ADC reading can be caused by an NTC-side short to 3V3,
    // an unusually hot sensor, or an ADC configuration/range issue.
    if (result.rawCounts >= ADC_HIGH_FAULT_COUNTS) {
        result.status = Status::ShortTo3V3OrOverrange;
        result.ntcOhms = 0.0f;
        result.ntcOhmsCal = 0.0f;
        result.celsius = result.celsiusCal = NAN;
        result.fahrenheit = result.fahrenheitCal = NAN;
        return result;
    }

    const float adc = static_cast<float>(result.rawCounts);
    const float mV = static_cast<float>(result.rawMillivolts);

    // Correct equation for this schematic:
    //
    // 3V3 -- R_NTC -- node -- R5(10k) -- GND
    //
    // ADC / MAX = R5 / (R_NTC + R5)
    //
    // R_NTC = R5 * (MAX - ADC) / ADC
    /*
    ------------------------
    If reversed circuit
    ------------------------
    result.ntcOhms=
        R_FIXED_OHMS *
        (static_cast<float>(ADC_MAX_COUNTS) - adc) / adc;
    */

    result.ntcOhms =
    (
        ( R_FIXED_OHMS ) * adc / (static_cast<float>(ADC_MAX_COUNTS) - adc )
    );
    result.ntcOhmsCal =
    (
        ( R_FIXED_OHMS ) * mV / (static_cast<float>(REF_MILLIVOLTS) - mV )
    );

    if (!isfinite(result.ntcOhms) || result.ntcOhms <= 0.0f)
    {
        result.status = Status::Invalid;
        result.celsius = result.celsiusCal = NAN;
        result.fahrenheit = result.fahrenheitCal = NAN;
        return result;
    }

    const float inverseTemperatureK =
        (1.0f / T25_K) +
        (logf(result.ntcOhms / NTC_R25_OHMS) / NTC_BETA_K);

    const float inverseTemperatureK_Cal =
        (1.0f / T25_K) +
        (logf(result.ntcOhmsCal / NTC_R25_OHMS) / NTC_BETA_K);

    const float kelvin = 1.0f / inverseTemperatureK + ADJUST_K;
    const float kelvinCal = 1.0f / inverseTemperatureK_Cal;

    result.status = Status::Ok;
    result.celsius = kelvin - 273.15f;
    result.celsiusCal = kelvinCal - 273.15f;
    result.fahrenheit = result.celsius * 1.8f + 32.0f;
    result.fahrenheitCal = result.celsiusCal * 1.8f + 32.0f;

    return result;
}

} // namespace cvl