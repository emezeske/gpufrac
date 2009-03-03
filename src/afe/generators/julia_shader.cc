#include <GL/glew.h>
#include <sstream>
#include <boost/assign/list_of.hpp> 

#include "common/string_bimap.h"
#include "common/cegui_utilities.h"
#include "afe/generators/julia_shader.h"

namespace {

const int GUI_SEED_PRECISION = 8;

typedef string_bimap<coloring_method> coloring_method_map;
const coloring_method_map coloring_methods = boost::assign::list_of<coloring_method_map::relation>
    ( "Iterations",     CM_ITERATIONS )
    ( "Radius Squared", CM_RADIUS_SQUARED );

typedef string_bimap<escape_condition> escape_condition_map;
const escape_condition_map escape_conditions = boost::assign::list_of<escape_condition_map::relation>
    ( "Box",    EC_BOX )
    ( "Circle", EC_CIRCLE );

mstring escape_condition_code( const escape_condition condition )
{
    mstring code;

    switch ( condition )
    {
        case EC_BOX:
            code = "if ( z.x > 2.0 && z.y > 2.0 )"; // TODO Size.
            break;
        case EC_CIRCLE:
            code = "if ( radius_squared > 4.0 )"; // TODO Radius.
            break;
        default:
            throw std::runtime_error( "Unknown escape_condition: " + utilities::to_s( condition ) );
    }

    return code;
}

mstring coloring_method_code( const coloring_method method )
{
    mstring code;

    switch ( method )
    {
        case CM_ITERATIONS:
            code = "gl_FragColor = texture2D( palette, vec2( i / float( num_iterations ) + palette_offset, 0.0 ) );";
            break;
        case CM_RADIUS_SQUARED:
            code = "gl_FragColor = texture2D( palette, vec2( radius_squared / 100.0 + palette_offset, 0.0 ) );"; // TODO Scaling.
            break;
            //code = "gl_FragColor = texture2D( palette, radius_squared / 100.0 + palette_offset, radius_squared / 25.0 );"; // TODO Scaling.
            //code = "gl_FragColor = vec4( sin( radius_squared / 5.0 ), cos( radius_squared / 5.0 ), 1.0, 1.0 );"; // TODO Scaling.
        default:
            throw std::runtime_error( "Unknown coloring_method: " + utilities::to_s( method ) );
    }

    return code;
}

void build_basic_program( std::ostream& stream, const coloring_method method, const escape_condition condition )
{
    using std::endl;

    stream      << "uniform vec2 seed;"
        << endl << "uniform sampler2D palette;"
        << endl << "uniform int num_iterations;"
        << endl << "uniform float palette_offset;"
        << endl << ""
        << endl << "void main ()"
        << endl << "{"
        << endl << "    vec2 z = gl_TexCoord[0].st;"
        << endl << ""
        << endl << "    gl_FragColor = texture2D( palette, vec2( palette_offset, 0.0 ) );"
        << endl << ""
        << endl << "    for ( int i = 0; i < num_iterations; ++i )"
        << endl << "    {"
        << endl << "        float"
        << endl << "            z_x_squared = z.x * z.x,"
        << endl << "            z_y_squared = z.y * z.y,"
        << endl << "            radius_squared = z_x_squared + z_y_squared;"
        << endl << ""
        << endl << "        z = vec2( z_x_squared - z_y_squared, 2.0 * z.x * z.y ) + seed;"
        << endl << ""
        << endl << "        " << escape_condition_code( condition )
        << endl << "        {"
        << endl << "            " << coloring_method_code( method )
        << endl << "            break;"
        << endl << "        }"
        << endl << "    }"
        << endl << "}"
        << endl;
}

} // anonymous namespace

cstring JuliaShader::GENERATOR_NAME = "Julia (OpenGL Shader)";

JuliaShader::JuliaShader() :
    palette_( "palette.png" ),
    palette_offset_( 0.0f ),
    num_iterations_( 8 ),
    seed_( 0.0f, 0.0f ),
    mouse_moving_seed_( false ),
    cycle_palette_( false ),
    coloring_method_( CM_ITERATIONS ),
    escape_condition_( EC_CIRCLE )
{
    initialize_gui();
    load_shader_program();
}

JuliaShader::~JuliaShader()
{
    destroy_gui();
}

void JuliaShader::load_shader_program()
{
    // Regular shader program:
    //
    // std::ostringstream stream;
    // build_basic_program( stream, coloring_method_, escape_condition_ );
    // shader_.load_program( stream.str() );

    // The new, broken 3D renderer...
    shader_.load_from_file( "src/afe/generators/julia_normal_map.glsl" );
}

bool JuliaShader::handleMouseMotionEvent( const Vector2Df& position )
{
    if ( mouse_moving_seed_ ) setSeed( position * 4.0f );
    return false;
}

bool JuliaShader::handleKeyDownEvent( const int key )
{
    bool handled = false;

    if ( key == SDLK_LSHIFT ) handled = mouse_moving_seed_ = true;

    return handled;
}

bool JuliaShader::handleKeyUpEvent( const int key )
{
    bool handled = false;

    if ( key == SDLK_LSHIFT )
    {
        handled = true;
        mouse_moving_seed_ = false;
    }

    return handled;
}

void JuliaShader::setSeed( const Vector2Df& seed )
{
    seed_ = seed;
    set_window_text( "afe/julia_shader/seed_real", seed_.x_, GUI_SEED_PRECISION );
    set_window_text( "afe/julia_shader/seed_imaginary", seed_.y_, GUI_SEED_PRECISION );
}

void JuliaShader::initialize_gui()
{
    using namespace CEGUI;

    WindowManager& wm = WindowManager::getSingleton();

    Window* root = wm.getWindow( "root" );
    Window* julia_shader = wm.loadWindowLayout( "afe.julia_shader.layout" );
    root->addChildWindow( julia_shader );
    julia_shader->moveToFront();

    configure_slider( "afe/julia_shader/iterations/slider", 8.0f / static_cast<float>( MAX_ITERATIONS ), Event::Subscriber( &JuliaShader::handleIterationsSlider, this ) );

    configure_dropdown_box( "afe/julia_shader/coloring_method", coloring_methods, Event::Subscriber( &JuliaShader::handleColoringMethod, this ) );

    configure_dropdown_box( "afe/julia_shader/escape_condition", escape_conditions, Event::Subscriber( &JuliaShader::handleEscapeCondition, this ) );

    Editbox* editbox = static_cast<Editbox*>( wm.getWindow( "afe/julia_shader/seed_real" ) );
    editbox->subscribeEvent( Editbox::EventTextAccepted, Event::Subscriber( &JuliaShader::handleSeedReal, this ) );
    editbox->setValidationString( "[0-9]+\\.?[0-9]*" );
    editbox->setMaxTextLength( GUI_SEED_PRECISION );

    wm.getWindow( "afe/julia_shader/cycle_palette" )->subscribeEvent( Checkbox::EventCheckStateChanged, Event::Subscriber( &JuliaShader::handleCyclePalette, this ) );

    setSeed( seed_ ); // Set the initial Editbox values.
}

void JuliaShader::destroy_gui()
{
    using namespace CEGUI;

    WindowManager& wm = WindowManager::getSingleton();

    wm.getWindow( "afe/julia_shader" )->destroy();
}

bool JuliaShader::handleIterationsSlider( const CEGUI::EventArgs& e )
{
    using namespace CEGUI;

    float iterations = static_cast<Slider*>( static_cast<const WindowEventArgs&>(e).window )->getCurrentValue();

    num_iterations_ = static_cast<int>( roundf( iterations * MAX_ITERATIONS ) );

    if ( num_iterations_ < MIN_ITERATIONS ) num_iterations_ = MIN_ITERATIONS;
    else if ( num_iterations_ > MAX_ITERATIONS ) num_iterations_ = MAX_ITERATIONS;

    return true;
}

bool JuliaShader::handleColoringMethod( const CEGUI::EventArgs& e )
{
    using namespace CEGUI;

    const ListboxItem* item = static_cast<Combobox*>( static_cast<const WindowEventArgs&>( e ).window )->getSelectedItem();

    VNOTIFY( FAULT, "coloring_method selected: %s", item->getText().c_str() );
    
    coloring_method_ = coloring_methods.get_specifier( item->getText().c_str() );
    load_shader_program();

    return true;
}

bool JuliaShader::handleEscapeCondition( const CEGUI::EventArgs& e )
{
    using namespace CEGUI;

    const ListboxItem* item = static_cast<Combobox*>( static_cast<const WindowEventArgs&>( e ).window )->getSelectedItem();

    VNOTIFY( FAULT, "escape_condition selected: %s", item->getText().c_str() );
    
    escape_condition_ = escape_conditions.get_specifier( item->getText().c_str() );
    load_shader_program();

    return true;
}

bool JuliaShader::handleSeedReal( const CEGUI::EventArgs& e )
{
    using namespace CEGUI;

    Editbox* editbox = static_cast<Editbox*>( static_cast<const WindowEventArgs&>( e ).window );

    VNOTIFY( FAULT, "seed_real: %s", editbox->getText().c_str() );

    seed_.x_ = boost::lexical_cast<float>( editbox->getText().c_str() );
    
    return true;
}

bool JuliaShader::handleCyclePalette( const CEGUI::EventArgs& e )
{
    using namespace CEGUI;

    cycle_palette_ = static_cast<Checkbox*>( static_cast<const WindowEventArgs&>(e).window )->isSelected();

    return true;
}

void JuliaShader::set_uniform_variables( const float pixel_width )
{
    glUniform2f( shader_.variable_location( "seed" ), seed_.x_, seed_.y_ );
    glUniform1i( shader_.variable_location( "num_iterations" ), num_iterations_ );
    glUniform1f( shader_.variable_location( "palette_offset" ), palette_offset_ );
    glUniform1f( shader_.variable_location( "pixel_width" ), pixel_width );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, palette_.getTexture() );
    glUniform1i( shader_.variable_location( "palette" ), 0 );
}

void JuliaShader::doOneStep( double step_time )
{
    if ( cycle_palette_ )
    {
        const float palette_cycle_speed = 0.1f;
        palette_offset_ += static_cast<float>( palette_cycle_speed * step_time );
        if ( palette_offset_ >= 1.0f ) palette_offset_ = 0.0f; // TODO
    }
}

void JuliaShader::draw( const Vector2Di& screen_size, const Vector2Df& viewport_position, const Vector2Df& viewport_size )
{
    shader_.enable();

    float pixel_width = viewport_size.x_ / static_cast<float>( screen_size.x_ ); // Assumes square pixels.
    set_uniform_variables( pixel_width );
    shader_.draw( screen_size, viewport_position, viewport_size );

    shader_.disable();

    // GLuint intermediate_fbo_;
    // glGenFramebuffersEXT( 1, &intermediate_fbo_ );
    // glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, intermediate_fbo_ );

    // GLuint intermediate_fbo_texture_;
    // glGenTextures( 1, &intermediate_fbo_texture_ );
    // glBindTexture( GL_TEXTURE_2D, intermediate_fbo_texture_ );
    // int width = 512, height = 512;
    // glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8,  width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );

    // glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, intermediate_fbo_texture_, 0 );

    // GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
}
