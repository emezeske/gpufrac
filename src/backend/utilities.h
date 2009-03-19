#ifndef UTILITIES_H
#define UTILITIES_H

#include <stdarg.h>
#include <string>
#include <sstream>

// These macros are tricky, but useful.  There is some evil here;  study the quoting issues well before modifying.
#define __utilities_str( s ) #s
#define __utilities_xstr( s ) __utilities_str( s )

#define POSITION_STRING ( std::string( __FILE__ ":" __utilities_xstr( __LINE__ ) ":" ) + __func__ )

#define PNOTIFY( level, p, s ) utilities::_do_notify( level, ( p + ": " + s + "\n" ).c_str() )
#define VPNOTIFY( level, p, s, ... ) utilities::_do_notify_fmt( level, ( p + ": " + s + "\n" ).c_str(), __VA_ARGS__ )

#define NOTIFY( level, s ) PNOTIFY( level, POSITION_STRING, s )
#define VNOTIFY( level, s, ... ) VPNOTIFY( level, POSITION_STRING, s, __VA_ARGS__ )

enum notify_level // I'd put this in the utilities namepace where it belongs, but this is convenient (no "using" needed).
{
	OFF = 0,
	FAULT,
	INFO,
	VERBOSE
};

typedef std::string mstring; // Mutable string.
typedef const std::string cstring; // Constant string.

namespace utilities
{

template <typename T>
mstring to_s( const T& v )
{
    std::ostringstream s;
    s << v;
    return s.str();
}

void _do_notify( notify_level level, const char *message ); // Without this, g++ complains about the format argument not being a string literal.

void _do_notify_fmt( notify_level level, const char *fmt, ... )
    __attribute__( ( format( printf, 2, 3 ) ) ); // Allows g++ to warn about arguments that do not agree with the format string.

notify_level set_notify_level( notify_level level );

} // end namespace utilities

#endif // UTILITIES_H
