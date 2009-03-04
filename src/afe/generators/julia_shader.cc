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
    ( "Iterative",  CM_ITERATIVE )
    ( "Continuous", CM_CONTINUOUS );

typedef string_bimap<escape_condition> escape_condition_map;
const escape_condition_map escape_conditions = boost::assign::list_of<escape_condition_map::relation>
    ( "Box",    EC_BOX )
    ( "Circle", EC_CIRCLE );

} // anonymous namespace

cstring JuliaShader::GENERATOR_NAME = "Julia (OpenGL Shader)";

JuliaShader::JuliaShader() :
    palette_( "palette.png" ),
    palette_offset_( 0.0f ),
    palette_cycle_speed_( 0.0f ),
    color_exponent_( 1.0 ),
    num_iterations_( 8 ),
    seed_( 0.0f, 0.0f ),
    mouse_moving_seed_( false ),
    enable_multisampling_( false ),
    enable_normal_mapping_( false ),
    coloring_method_( CM_ITERATIVE ),
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
    google::TemplateDictionary dictionary( "julia" );

    dictionary.ShowSection( "ESCAPE_CONDITION_" + escape_conditions.get_specifier( escape_condition_ ) );
    dictionary.ShowSection( "COLORING_METHOD_" + coloring_methods.get_specifier( coloring_method_ ) );

    if ( enable_multisampling_ ) dictionary.ShowSection( "ENABLE_MULTISAMPLING" );
    if ( enable_normal_mapping_ ) dictionary.ShowSection( "ENABLE_NORMAL_MAPPING" );

    shader_.load_from_template( "src/afe/generators/julia.glsl.tpl", dictionary );
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

    configure_slider( "afe/julia_shader/iterations/slider", 64.0f / static_cast<float>( MAX_ITERATIONS ), Event::Subscriber( &JuliaShader::handleIterationsSlider, this ) );

    configure_slider( "afe/julia_shader/palette_cycle_speed/slider", 0.5f, Event::Subscriber( &JuliaShader::handlePaletteCycleSpeedSlider, this ) );

    configure_slider( "afe/julia_shader/color_exponent/slider", 1.0f, Event::Subscriber( &JuliaShader::handleColorSlider, this) );

    configure_dropdown_box( "afe/julia_shader/coloring_method", coloring_methods, Event::Subscriber( &JuliaShader::handleColoringMethod, this ) );

    configure_dropdown_box( "afe/julia_shader/escape_condition", escape_conditions, Event::Subscriber( &JuliaShader::handleEscapeCondition, this ) );

    Editbox* editbox = static_cast<Editbox*>( wm.getWindow( "afe/julia_shader/seed_real" ) );
    editbox->subscribeEvent( Editbox::EventTextAccepted, Event::Subscriber( &JuliaShader::handleSeedReal, this ) );
    editbox->setValidationString( "[0-9]+\\.?[0-9]*" );
    editbox->setMaxTextLength( GUI_SEED_PRECISION );
    // FIXME Editable seed boxes don't work

    wm.getWindow( "afe/julia_shader/enable_multisampling" )->subscribeEvent( Checkbox::EventCheckStateChanged, Event::Subscriber( &JuliaShader::handleEnableMultisampling, this ) );

    wm.getWindow( "afe/julia_shader/enable_normal_mapping" )->subscribeEvent( Checkbox::EventCheckStateChanged, Event::Subscriber( &JuliaShader::handleEnableNormalMapping, this ) );

    // TODO Add a slider for palette stretching factor

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

    float position = static_cast<Slider*>( static_cast<const WindowEventArgs&>(e).window )->getCurrentValue();

    num_iterations_ = static_cast<int>( roundf( position * MAX_ITERATIONS ) );

    if ( num_iterations_ < MIN_ITERATIONS ) num_iterations_ = MIN_ITERATIONS;
    else if ( num_iterations_ > MAX_ITERATIONS ) num_iterations_ = MAX_ITERATIONS;

    return true;
}

bool JuliaShader::handleColorSlider( const CEGUI::EventArgs& e )
{
  using namespace CEGUI;

    float position = static_cast<Slider*>( static_cast<const WindowEventArgs&>(e).window )->getCurrentValue();
 
    color_exponent_ = position;
    
    if ( color_exponent_ < MIN_COLOR_EXPONENT ) color_exponent_ = MIN_COLOR_EXPONENT;
    else if ( color_exponent_ > MAX_COLOR_EXPONENT ) color_exponent_ = MAX_COLOR_EXPONENT;

    return true;
}

bool JuliaShader::handlePaletteCycleSpeedSlider( const CEGUI::EventArgs& e )
{
    using namespace CEGUI;

    float position = static_cast<Slider*>( static_cast<const WindowEventArgs&>(e).window )->getCurrentValue();

    palette_cycle_speed_ = 2.0f * ( position - 0.5f ) * MAX_PALETTE_CYCLE_SPEED;

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

bool JuliaShader::handleEnableMultisampling( const CEGUI::EventArgs& e )
{
    using namespace CEGUI;

    enable_multisampling_ = static_cast<Checkbox*>( static_cast<const WindowEventArgs&>(e).window )->isSelected();

    if ( !enable_multisampling_ ) enable_normal_mapping_ = false;

    // TODO Update normal mapping checkbox

    load_shader_program();

    return true;
}

bool JuliaShader::handleEnableNormalMapping( const CEGUI::EventArgs& e )
{
    using namespace CEGUI;

    bool enable = static_cast<Checkbox*>( static_cast<const WindowEventArgs&>(e).window )->isSelected();

    if ( enable )
    {
        enable_multisampling_ = true;
        enable_normal_mapping_ = true;
    }
    else enable_normal_mapping_ = false;

    // TODO Update multisampling checkbox

    load_shader_program();

    return true;
}

void JuliaShader::set_uniform_variables( const float pixel_width )
{
    shader_.set_uniform_vec2d( "seed", seed_ );
    shader_.set_uniform_int( "num_iterations", num_iterations_ );
    shader_.set_uniform_float( "palette_offset", palette_offset_ );
    shader_.set_uniform_float( "pixel_width", pixel_width );
    shader_.set_uniform_float( "color_exponent", color_exponent_);

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, palette_.getTexture() );
    shader_.set_uniform_int( "palette", 0 );
}

void JuliaShader::doOneStep( double step_time )
{
    if ( fabs( palette_cycle_speed_ ) > PALETTE_CYCLE_SPEED_DEADZONE )
    {
        palette_offset_ += static_cast<float>( palette_cycle_speed_ * step_time );
        if ( palette_offset_ >= 1.0f ) palette_offset_ = 0.0f;
    }
}

void JuliaShader::draw( const Vector2Di& screen_size, const Vector2Df& viewport_position, const Vector2Df& viewport_size )
{
    shader_.enable();

    float pixel_width = viewport_size.x_ / static_cast<float>( screen_size.x_ ); // Assumes square pixels.
    set_uniform_variables( pixel_width );
    shader_.draw( screen_size, viewport_position, viewport_size );

    shader_.disable();

    // TODO Example code for drawing into a framebuffer object, to allow for possible post-processing

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
