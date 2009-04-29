#include <cstdio>

#include "utilities.h"

namespace utilities
{

notify_level current_notify_level = VERBOSE;

notify_level set_notify_level( notify_level level )
{
	notify_level t = current_notify_level;
	current_notify_level = level;
	return t;
}

void _do_notify( notify_level level, const char *message )
{
	if ( level <= current_notify_level )
	{
        fputs( message, stderr );
    }
}

void _do_notify_fmt( notify_level level, const char *fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );

	if ( level <= current_notify_level )
	{
		vfprintf( stderr, fmt, ap );
	}

	va_end( ap );
}

} // end namespace utilities
