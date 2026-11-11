#pragma once

#include "configuration.h"
#define LOG_DEBUG( format, ... ) ESP_LOGD( "JUNK", format __VA_OPT__(,) __VA_ARGS__ )
#define LOG_INFO( format, ... ) ESP_LOGI( "JUNK", format __VA_OPT__(,) __VA_ARGS__ )
#define LOG_WARN( format, ... ) ESP_LOGW( "JUNK", format __VA_OPT__(,) __VA_ARGS__ )
#define LOG_ERROR( format, ... ) ESP_LOGE( "JUNK", format __VA_OPT__(,) __VA_ARGS__ )