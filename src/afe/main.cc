#include <CEGUI.h>

#include "common/utilities.h"
#include "common/exceptions.h"
#include "afe/fractal_application.h"

const int
    SCREEN_WIDTH  = 1024,
    SCREEN_HEIGHT = 1024,
    SCREEN_BPP    = 32;

const Uint32 SCREEN_FLAGS = SDL_HWSURFACE | SDL_OPENGL;

int main()
{
    int result = -1;

    utilities::set_notify_level( VERBOSE );

    try
    {
        FractalWindow window( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SCREEN_FLAGS, "Fractal Generator" );
        FractalApplication application( window, 60 );
        application.mainLoop();
        result =  0;
    }
    catch ( CEGUI::Exception &e ) { VNOTIFY( FAULT, "CEGUI Exception: %s.", e.getMessage().c_str() ); }
    catch ( LoadingError &e ) { VNOTIFY( FAULT, "LoadingError: %s.", e.what().c_str() ); }
    catch ( std::exception &e ) { VNOTIFY( FAULT, "std::exception: %s.", e.what() ); }

    return result;
}

