#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <stdarg.h>
#include <string>
#include <sstream>
#include <boost/lexical_cast.hpp>

// These macros are tricky, but useful.  There is some evil here;  study the quoting issues well before modifying.
#define __utilities_str( s ) #s
#define __utilities_xstr( s ) __utilities_str( s )

#define POSITION_STRING ( std::string( __FILE__ ":" __utilities_xstr( __LINE__ ) ":" ) + __func__ )

#define PNOTIFY( l, p, s ) utilities::_do_notify( l, ( p + ": " + s + "\n" ).c_str() )
#define VPNOTIFY( l, p, s, ... ) utilities::_do_notify( l, ( p + ": " + s + "\n" ).c_str(), __VA_ARGS__ )

#define NOTIFY( l, s ) PNOTIFY( l, POSITION_STRING, s )
#define VNOTIFY( l, s, ... ) VPNOTIFY( l, POSITION_STRING, s, __VA_ARGS__ )

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

void _do_notify( notify_level l, const char *fmt, ... );

template< class T > // TODO: This is probably not generic enough to be here.  
void free_contained( T &c )
{
	for ( typename T::iterator i = c.begin(); i != c.end(); ++i )
		delete ( *i );
}

notify_level set_notify_level( notify_level l );

void die( int code ); // TODO: Macrofy this?  Currently doesn't print line on which die() is called, which is typically useful.

typedef std::basic_string< unsigned char > ubasic_string; // For notational convenience.

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This class makes dealing with textual strings (signed) and networks (unsigned) much more pleasant.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ustring : public ubasic_string // TODO: Come up with a solution that doesn't hide all of ubasic_string's constructors...
{
	public:
		
		ustring( const char *s ) : // Shhhh... I said that it's ok to do this!
			ubasic_string( reinterpret_cast< const unsigned char * >( s ) ) {}

		const char *c_str() const { return reinterpret_cast< const char * >( ubasic_string::c_str() ); }
		const char *data() const { return reinterpret_cast< const char * >( ubasic_string::data() ); }

		const unsigned char *udata() const { return ubasic_string::data();  }
		const unsigned char *uc_str() const { return ubasic_string::c_str();  }
};

} // end namespace utilities

#endif // __UTILITIES_H__
