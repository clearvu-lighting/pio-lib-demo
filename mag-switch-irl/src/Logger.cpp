#include "Logger.hpp"
#include <cstring>

using namespace std;
namespace CVL
{

const char *Logger::default_tag = "UNNAMED LOGGER";

void Logger::set_tag( const char *tag_ )
{
	size_t len = 0;
	if( tag_ == NULL )
	{
		len = strlen(default_tag);
		strcpy( tag_data, default_tag );
	}
	else
	{
		memset( tag_data, ' ', TAG_WIDTH );
		len = strlen(tag_);
		if( len > TAG_WIDTH ) len=TAG_WIDTH-1;
		memcpy( tag_data, tag_, len );
	}
	tag_data[len]=0;
}

void Logger::set_tag( const string &tag_ )
{
	size_t len = tag_.length();
	if( len == 0 )
	{
		len = strlen(default_tag);
		strcpy( tag_data, default_tag );
	}
	else
	{
		memset( tag_data, ' ', TAG_WIDTH-1 );
		if( len > TAG_WIDTH ) len=TAG_WIDTH-1;
		tag_.copy( tag_data, len, 0 );
	}
	tag_data[len]=0;
}

Logger::Logger( const char *tag_ )
	: tag( tag_data )
{
	set_tag( tag_ );
}

Logger::Logger( const std::string &tag_ )
	: tag( tag_data )
{
	set_tag( tag_ );
}

}