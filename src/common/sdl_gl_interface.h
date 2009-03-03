#ifndef SDL_GL_INTERFACE_H
#define SDL_GL_INTERFACE_H

#include <SDL/SDL.h>
#include <string>

#include "utilities.h"

class SDL_GL_Window
{
    public:
        SDL_GL_Window( const int w, const int h, const int bpp, const Uint32 flags, const std::string &title ); // May throw a LoadingError().
        virtual ~SDL_GL_Window() {}

        virtual void createWindow();
        virtual void reshapeWindow( const int w, const int h );

        virtual int width() const { return screen_width_; }
        virtual int height() const { return screen_height_; }

    protected:
        virtual void initGL();
        virtual void reshapeWindow();

		SDL_Surface *screen_;

		int
			screen_width_,
			screen_height_,
			screen_bpp_,
			sdl_video_flags_;

        std::string title_;
};

class SDL_GL_Interface
{
	public:
        SDL_GL_Interface( SDL_GL_Window &initializer, const int fps_limit );
		virtual ~SDL_GL_Interface();

		virtual void toggleFullscreen();
		virtual void mainLoop();

	protected:
		virtual void processEvents( const bool doHandle );
		virtual void handleEvent( SDL_Event& event );
		virtual void handleKeyDownEvent( SDL_Event& event ) = 0;
		virtual void handleKeyUpEvent( SDL_Event& event ) = 0;
		virtual void handleMouseMotionEvent( SDL_Event& event ) = 0;
		virtual void handleMouseDownEvent( SDL_Event& event ) = 0;
		virtual void handleMouseUpEvent( SDL_Event& event ) = 0;
		virtual void doOneStep( const double stepTime ) = 0;
		virtual void draw() = 0;

		bool run_;

		long last_step_time_;

        int fps_limit_;

        SDL_GL_Window &window_;
};

#endif // SDL_GL_INTERFACE_H
