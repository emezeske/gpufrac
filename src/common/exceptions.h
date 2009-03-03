#ifndef __EXCEPTIONS_H__
#define __EXCEPTIONS_H__

#include <string>

class LoadingError 
{
	public:
		LoadingError( const std::string &w ) :
			what_( w )
		{
		}

        std::string what() const { return what_; }

	private:
        std::string what_;
};

#endif // __EXCEPTIONS_H__

