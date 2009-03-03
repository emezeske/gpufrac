#include <SDL/SDL_opengl.h>
#include <CEGUIDefaultResourceProvider.h>
#include <boost/assign/list_of.hpp> 

#include "common/utilities.h"
#include "common/cegui_utilities.h"
#include "common/string_bimap.h"
#include "afe/fractal_application.h"
#include "afe/generators/julia_shader.h"
#include "afe/generators/mandelbrot_shader.h"

namespace {

pFractalGenerator make_JuliaShader() { return pFractalGenerator( new JuliaShader() ); }
pFractalGenerator make_MandelbrotShader() { return pFractalGenerator( new MandelbrotShader() ); }

typedef pFractalGenerator( *generator_maker )();
typedef string_bimap<generator_maker> generator_map;
const generator_map generators = boost::assign::list_of<generator_map::relation>
    ( JuliaShader::GENERATOR_NAME,      &make_JuliaShader )
    ( MandelbrotShader::GENERATOR_NAME, &make_MandelbrotShader );

pFractalGenerator get_generator( cstring name )
{
    return generators.get_specifier( name )();
}

} // anonymous namespace

////////////////////////////////////////////////////////////////////////////////
// Member function definitions for FractalWindow:
////////////////////////////////////////////////////////////////////////////////

FractalWindow::FractalWindow( const int w, const int h, const int bpp, const Uint32 flags, const std::string &title ) : 
    SDL_GL_Window( w, h, bpp, flags, title )
{
}

void FractalWindow::initGL()
{
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
}

void FractalWindow::createWindow()
{
    SDL_GL_Window::createWindow();

    SDL_ShowCursor( SDL_DISABLE );
    SDL_EnableUNICODE( 1 );
    SDL_EnableKeyRepeat (SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
}

void FractalWindow::reshapeWindow() // Override the default reshapeWindow(), to use an orthogonal projection matrix.
{
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    
    glOrtho( 0, screen_width_, 0, screen_height_, 0, 1.0 );
    
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
}

////////////////////////////////////////////////////////////////////////////////
// Member function definitions for FractalApplication:
////////////////////////////////////////////////////////////////////////////////

FractalApplication::FractalApplication( SDL_GL_Window &initializer, const int fps ) :
    SDL_GL_Interface( initializer, fps ),
    viewport_( Vector2Df( -1.0f, -1.0f ), Vector2Df( 2.0f, 2.0f ) ),
    frames_( 0 )
{
    initialize_gui();
}

FractalApplication::~FractalApplication()
{
}

void FractalApplication::initialize_gui()
{
    using namespace CEGUI;

    cegui_renderer_.reset( new OpenGLRenderer( 0, window_.width(), window_.height() ) );
    new System( cegui_renderer_.get() );

    // initialize the required dirs for the DefaultResourceProvider
    DefaultResourceProvider* rp = static_cast<DefaultResourceProvider*>( System::getSingleton().getResourceProvider() );
    
    rp->setResourceGroupDirectory( "schemes", "datafiles/schemes/" );
    rp->setResourceGroupDirectory( "imagesets", "datafiles/imagesets/" );
    rp->setResourceGroupDirectory( "fonts", "datafiles/fonts/" );
    rp->setResourceGroupDirectory( "layouts", "datafiles/layouts/" );
    rp->setResourceGroupDirectory( "looknfeels", "datafiles/looknfeel/" );
    rp->setResourceGroupDirectory( "lua_scripts", "datafiles/lua_scripts/" );

    CEGUI::Imageset::setDefaultResourceGroup( "imagesets" );
    CEGUI::Font::setDefaultResourceGroup( "fonts" );
    CEGUI::Scheme::setDefaultResourceGroup( "schemes" );
    CEGUI::WidgetLookManager::setDefaultResourceGroup( "looknfeels" );
    CEGUI::WindowManager::setDefaultResourceGroup( "layouts" );

    SchemeManager::getSingleton().loadScheme( "SleekSpace.scheme" );
    System::getSingleton().setDefaultMouseCursor( "SleekSpace", "MouseArrow" );

    WindowManager& wm = WindowManager::getSingleton();
    Window* root = wm.loadWindowLayout( "afe.layout" );
    System::getSingleton().setGUISheet( root );
    root->addChildWindow( root );

    wm.getWindow( "afe/general/fullscreen" )->subscribeEvent( Checkbox::EventCheckStateChanged, Event::Subscriber( &FractalApplication::handleFullscreen, this ) );

    wm.getWindow( "afe/general/exit" )->subscribeEvent( PushButton::EventClicked, Event::Subscriber( &FractalApplication::handleExit, this ) );

    configure_slider( "afe/general/opacity/slider", 0.85f, Event::Subscriber( &FractalApplication::handleOpacitySlider, this ) );

    configure_dropdown_box( "afe/general/generator", generators, Event::Subscriber( &FractalApplication::handleGenerator, this ) );

    generator_.reset( new JuliaShader() ); // FIXME
}

bool FractalApplication::handleFullscreen( const CEGUI::EventArgs& e )
{
    using namespace CEGUI;

    // bool fullscreen = static_cast<Checkbox*>( static_cast<const WindowEventArgs&>(e).window )->isSelected();

    SDL_Surface* s = SDL_GetVideoSurface();

    if( !s || ( SDL_WM_ToggleFullScreen( s ) != 1 ) )
    {
        VNOTIFY( FAULT, "Unable to toggle fullscreen: %s", SDL_GetError() );
    }

    return true;
}

bool FractalApplication::handleExit( const CEGUI::EventArgs& e )
{
    run_ = false;
    return true;
}

bool FractalApplication::handleOpacitySlider( const CEGUI::EventArgs& e )
{
    using namespace CEGUI;

    float opacity = static_cast<Slider*>( static_cast<const WindowEventArgs&>(e).window )->getCurrentValue();

    if ( opacity < 0.1f ) opacity = 0.1f;

    WindowManager::getSingleton().getWindow( "root" )->setAlpha( opacity );

    return true;
}

bool FractalApplication::handleGenerator( const CEGUI::EventArgs& e )
{
    using namespace CEGUI;

    const ListboxItem* item = static_cast<Combobox*>( static_cast<const WindowEventArgs&>( e ).window )->getSelectedItem();

    VNOTIFY( FAULT, "generator selected: %s", item->getText().c_str() );

    // Ensure the destructor for the old FractalGenerator is called before creating the new one.
    // TODO: It would be nice to enforce this behavior somehow...
    generator_.reset();
    generator_ = get_generator( item->getText().c_str() );

    return true;
}

void FractalApplication::handleEvent( SDL_Event &e )
{
    SDL_GL_Interface::handleEvent( e );

    if ( e.type == SDL_VIDEORESIZE )
    {
        cegui_renderer_->setDisplaySize( CEGUI::Size( static_cast<float>( e.resize.w ), static_cast<float>( e.resize.h ) ) );
    }
}

void FractalApplication::handleKeyDownEvent( SDL_Event& e )
{
    bool handled = false;

    if ( generator_.get() ) handled = generator_->handleKeyDownEvent( e.key.keysym.sym );

    if ( !handled )
    {
        if ( e.key.keysym.sym == SDLK_SPACE )
        {
            using namespace CEGUI;
            Window* root = WindowManager::getSingleton().getWindow( "root" );
            bool visible = root->isVisible();
            root->setVisible( !visible );
        }
        else
        {
            CEGUI::System::getSingleton().injectKeyDown( e.key.keysym.scancode );

            if ( ( e.key.keysym.unicode & 0xFF80 ) == 0 )
            {
                CEGUI::System::getSingleton().injectChar( e.key.keysym.unicode & 0x7F );
            }

            if ( e.key.keysym.sym == SDLK_ESCAPE ) run_ = false;

            depressed_keys_[e.key.keysym.sym] = true;
            setPanVelocity();
        }
    }
}

void FractalApplication::handleKeyUpEvent( SDL_Event& e )
{
    bool handled = false;

    if ( generator_.get() ) handled = generator_->handleKeyUpEvent( e.key.keysym.sym );

    if ( !handled )
    {
        CEGUI::System::getSingleton().injectKeyUp( e.key.keysym.scancode );
        depressed_keys_[e.key.keysym.sym] = false;
        setPanVelocity();
    }
}

void FractalApplication::handleMouseMotionEvent( SDL_Event& e )
{
    bool handled = false;

    if ( generator_.get() ) handled = generator_->handleMouseMotionEvent( getMousePosition( e.motion.x, e.motion.y ) );
    if ( !handled ) CEGUI::System::getSingleton().injectMousePosition( static_cast<float>( e.motion.x ), static_cast<float>( e.motion.y ) );
}

void FractalApplication::handleMouseDownEvent( SDL_Event& e )
{
    switch ( e.button.button )
    {
        case SDL_BUTTON_LEFT:
            CEGUI::System::getSingleton().injectMouseButtonDown( CEGUI::LeftButton );
            break;
        case SDL_BUTTON_MIDDLE:
            CEGUI::System::getSingleton().injectMouseButtonDown( CEGUI::MiddleButton );
            break;
        case SDL_BUTTON_RIGHT:
            CEGUI::System::getSingleton().injectMouseButtonDown( CEGUI::RightButton );
            break;
        case SDL_BUTTON_WHEELDOWN:
            CEGUI::System::getSingleton().injectMouseWheelChange( -1 );
            break;
        case SDL_BUTTON_WHEELUP:
            CEGUI::System::getSingleton().injectMouseWheelChange( +1 );
            break;
    }
}

void FractalApplication::handleMouseUpEvent( SDL_Event& e )
{
    switch ( e.button.button )
    {
        case SDL_BUTTON_LEFT:
            CEGUI::System::getSingleton().injectMouseButtonUp( CEGUI::LeftButton );
            break;
        case SDL_BUTTON_MIDDLE:
            CEGUI::System::getSingleton().injectMouseButtonUp( CEGUI::MiddleButton );
            break;
        case SDL_BUTTON_RIGHT:
            CEGUI::System::getSingleton().injectMouseButtonUp( CEGUI::RightButton );
            break;
    }
}

void FractalApplication::doOneStep( const double step_time )
{
    static double accumulator = 0.0;

    if ( ( accumulator += step_time ) > 1.0 )
    {
        VNOTIFY( FAULT, "FPS: %d", frames_ );
        accumulator = 0.0;
        frames_ = 0;
    }

    viewport_.doOneStep( static_cast<float>( step_time ) );
    if ( generator_.get() ) generator_->doOneStep( step_time );

    CEGUI::System::getSingleton().injectTimePulse( static_cast<float>( step_time ) );
}   

void FractalApplication::draw()
{
    ++frames_;

    if ( generator_.get() ) generator_->draw( Vector2Di( window_.width(), window_.height() ), viewport_.position(), viewport_.size() );

    CEGUI::System::getSingleton().renderGUI();

    glFinish();
}

Vector2Df FractalApplication::getMousePosition( const int x, const int y )
{
    return Vector2Df( static_cast<float>( x ) / static_cast<float>( window_.width() ) - 0.5f,
                      static_cast<float>( window_.height() - 1 - y ) / static_cast<float>( window_.height() ) - 0.5f );
}

void FractalApplication::setPanVelocity()
{
    Vector2Df pan_velocity( 0.0f, 0.0f );

    if ( depressed_keys_[SDLK_d] ) pan_velocity.x_ += 1.0f;
    if ( depressed_keys_[SDLK_a] ) pan_velocity.x_ -= 1.0f;
    if ( depressed_keys_[SDLK_w] ) pan_velocity.y_ += 1.0f;
    if ( depressed_keys_[SDLK_s] ) pan_velocity.y_ -= 1.0f;

    viewport_.setDesiredPanVelocity( pan_velocity );

    float zoom_velocity = 0.0f;

    if ( depressed_keys_[SDLK_e] ) zoom_velocity += 1.0f;
    if ( depressed_keys_[SDLK_q] ) zoom_velocity -= 1.0f;

    viewport_.setDesiredZoomVelocity( zoom_velocity );
}
