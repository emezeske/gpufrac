#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <errno.h>

using std::string;

#include "utilities.h"

namespace utilities
{

notify_level current_notify_level = FAULT;

notify_level set_notify_level( notify_level l )
{
	notify_level t = current_notify_level;
	current_notify_level = l;
	return t;
}

void die( int code )
{
	if ( code ) NOTIFY( INFO, "Exiting abnormally!" );
	else NOTIFY( INFO, "Exiting." );
	exit( code );
}

void _do_notify( notify_level l, const char *fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );

	if ( l <= current_notify_level )
	{
		size_t len = strlen( fmt );

		for ( size_t i = 0; i < len - 1; ++i )
		{
			if ( fmt[ i ] == '%' )
			{
				if ( fmt[ i + 1 ] == '%' ) ++i;
				else if ( fmt[ i + 1 ] == 'm' ) 
				{
					string new_fmt = string( fmt, i ); 
					new_fmt += strerror( errno );
					if ( i + 2 < len ) new_fmt += string( &fmt[ i + 2 ] );
					fmt = new_fmt.c_str();
					break;
				}
			}
		}

		vfprintf( stderr, fmt, ap );
	}

	va_end( ap );
}

} // end namespace utilities
