/* Logger */
#pragma once

#include "cvl/configuration.h"
#include <string>

#ifdef MESHTASTIC_LOG_LEVEL_DEBUG

// Warning. This does does not work without "tag".
#define LOGD( format, ... ) LOG_DEBUG( "[%s] " format, tag __VA_OPT__(,) __VA_ARGS__ )
#define LOGI( format, ... ) LOG_INFO( "[%s] " format, tag __VA_OPT__(,) __VA_ARGS__ )
#define LOGW( format, ... ) LOG_ERROR( "[%s] " format, tag __VA_OPT__(,) __VA_ARGS__ )
#define LOGE( format, ... ) LOG_ERROR( "[%s] " format, tag __VA_OPT__(,) __VA_ARGS__ )

#else

extern const char *tag;
#define LOGD( format, ... ) ESP_LOGD( tag, format __VA_OPT__(,) __VA_ARGS__ )
#define LOGI( format, ... ) ESP_LOGI( tag, format __VA_OPT__(,) __VA_ARGS__ )
#define LOGW( format, ... ) ESP_LOGW( tag, format __VA_OPT__(,) __VA_ARGS__ )
#define LOGE( format, ... ) ESP_LOGE( tag, format __VA_OPT__(,) __VA_ARGS__ )

#endif

namespace CVL
{
class Logger
{
	public:
	static const size_t TAG_WIDTH = 20;
	static const char *default_tag; 
    /*
     * There is a tag that is text used in logging. 
     * It is fixed width. Use set_tag to change the text.
     * It will be padded out with spaces to always be 20 
     * characters wide. There will always be a null terminator 
     * at TAG_WIDTH + 1.
     * 
     * "default_tag" will be used if the tag is not set by the 
     * constructor.
     */

    protected:
    const char               *tag;
    char                      tag_data[TAG_WIDTH+1]; // This is the prefix used for logging (ESP_LOGI).

    public:
    Logger( const char *tag = NULL );
    Logger( const std::string &tag );

	virtual const char *get_tag(void) { return tag; };
	virtual void set_tag( const char *tag );
	virtual void set_tag( const std::string &tag );
};
}