#ifndef FRACTAL_APPLICATION_H
#define FRACTAL_APPLICATION_H

#include <vector>

#include <CEGUI.h>
#include <RendererModules/OpenGLGUIRenderer/openglrenderer.h>

#include "common/sdl_gl_interface.h"
#include "afe/viewport.h"
#include "afe/generators/fractal_generator.h"

////////////////////////////////////////////////////////////////////////////////
// The FractalWindow class takes care of the SDL/OpenGL window initialization.
////////////////////////////////////////////////////////////////////////////////

class FractalWindow : public SDL_GL_Window
{
    public:
        FractalWindow( const int w, const int h, const int bpp, const Uint32 flags, const std::string &title );
        virtual ~FractalWindow() {}

        virtual void initGL();
        virtual void createWindow();

    protected:
        virtual void reshapeWindow();
};

////////////////////////////////////////////////////////////////////////////////
// The FractalApplication class represents a running instance of the game.
////////////////////////////////////////////////////////////////////////////////

class FractalApplication : public SDL_GL_Interface
{
    public:
        FractalApplication( SDL_GL_Window &initializer, const int fps );
        virtual ~FractalApplication();

    protected:
		virtual void handleEvent( SDL_Event& event );
        virtual void handleKeyDownEvent( SDL_Event& e );
        virtual void handleKeyUpEvent( SDL_Event& e );
        virtual void handleMouseMotionEvent( SDL_Event& e );
        virtual void handleMouseDownEvent( SDL_Event& e );
        virtual void handleMouseUpEvent( SDL_Event& e );
        virtual void doOneStep( const double stepTime );
        virtual void draw();

        virtual Vector2Df getMousePosition( const int x, const int y );
        virtual void setPanVelocity();

        void initialize_gui();
        bool handleFullscreen( const CEGUI::EventArgs& e );
        bool handleExit( const CEGUI::EventArgs& e );
        bool handleOpacitySlider( const CEGUI::EventArgs& e );
        bool handleGenerator( const CEGUI::EventArgs& e );

        typedef std::map<int, bool> keymap;
        keymap depressed_keys_;

        Viewport viewport_;

        pFractalGenerator generator_;

        boost::shared_ptr<CEGUI::OpenGLRenderer> cegui_renderer_;

        long frames_;
};

#endif // FRACTAL_APPLICATION_H
