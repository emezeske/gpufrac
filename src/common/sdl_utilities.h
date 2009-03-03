#ifndef SDL_UTILITIES_H
#define SDL_UTILITIES_H

#include <SDL/SDL.h>
#include <string>

#include "utilities.h"

#define SCOPE_TIMER_BEGIN { SDL_ScopeTimer __scope_timer( POSITION_STRING );
#define SCOPE_TIMER_END }

class SDL_ScopeTimer
{
    public:
        SDL_ScopeTimer( const std::string position ) : 
            position_( position ),
            began_at_( SDL_GetTicks() )
        {}

        ~SDL_ScopeTimer() { VPNOTIFY( VERBOSE, position_, "Timer took %d ms.", SDL_GetTicks() - began_at_ ); }

    private:
        std::string position_;
        long began_at_;
};

#endif // SDL_UTILITIES_H

