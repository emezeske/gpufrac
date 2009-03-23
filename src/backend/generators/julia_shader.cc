#include <GL/glew.h>
#include <boost/assign/list_of.hpp> 
#include <map>
#include <stdexcept>

#include "backend/generators/julia_shader.h"

namespace {

const std::map<ColoringMethod, cstring> coloring_methods = boost::assign::map_list_of
    ( CM_ITERATIVE,      "COLORING_METHOD_ITERATIVE" )
    ( CM_CONTINUOUS,     "COLORING_METHOD_CONTINUOUS" )
    ( CM_RADIUS_SQUARED, "COLORING_METHOD_RADIUS_SQUARED" )
    ( CM_ANGLE,          "COLORING_METHOD_ANGLE" );

const std::map<EscapeCondition, cstring> escape_conditions = boost::assign::map_list_of
    ( EC_BOX,    "ESCAPE_CONDITION_BOX" )
    ( EC_CIRCLE, "ESCAPE_CONDITION_CIRCLE" );

const std::map<PaletteMode, cstring> palette_modes = boost::assign::map_list_of
    ( PM_TEXTURE,   "PALETTE_MODE_TEXTURE" )
    ( PM_TRIG,      "PALETTE_MODE_TRIG" )
    ( PM_MAGNITUDE, "PALETTE_MODE_MAGNITUDE" );

const std::map<MultisamplingMode, cstring> multisampling_modes = boost::assign::map_list_of
    ( MS_NONE, "MULTISAMPLING_NONE" )
    ( MS_4X,   "MULTISAMPLING_4X" )
    ( MS_8X,   "MULTISAMPLING_8X" );

template <typename enum_type>
cstring map_lookup( const std::map<enum_type, cstring>& m, const enum_type e )
{
    typename std::map<enum_type, cstring>::const_iterator it = m.find( e );

    if ( it == m.end() ) throw std::runtime_error( "Not a valid map entry: " + utilities::to_s( e ) );

    return it->second;
}

} // anonymous namespace

JuliaShader::JuliaShader() :
    palette_texture_( 0 ),
    seed_( 0.0f, 0.0f ),
    palette_offset_( 0.0f ),
    palette_stretch_( 1.0f ),
    julia_exponent_( 2.0f ),
    height_scale_( 1.0f ),
    red_phase_( 0.0f ),
    blue_phase_( 0.0f ),
    green_phase_( 0.0f ),
    red_amplitude_( 1.0f ),
    blue_amplitude_( 1.0f ),
    green_amplitude_( 1.0f ),
    red_frequency_( 1.0f ),
    blue_frequency_( 1.0f ),
    green_frequency_( 1.0f ),
    max_iterations_( 128 ),
    mandelbrot_mode_enabled_( false ),
    normal_mapping_enabled_( false ),
    arbitrary_exponent_enabled_( false ),
    coloring_method_( CM_CONTINUOUS ),
    escape_condition_( EC_CIRCLE ),
    palette_mode_( PM_TRIG ),
    multisampling_mode_( MS_NONE )
{
    load_shader_program();
}

void JuliaShader::set_mandelbrot_mode_enabled( const bool mandelbrot_mode_enabled )
{
    mandelbrot_mode_enabled_ = mandelbrot_mode_enabled;
    load_shader_program();
}

void JuliaShader::set_normal_mapping_enabled( const bool normal_mapping_enabled )
{
    normal_mapping_enabled_ = normal_mapping_enabled;
    load_shader_program();
}

void JuliaShader::set_arbitrary_exponent_enabled( const bool arbitrary_exponent_enabled )
{
    arbitrary_exponent_enabled_ = arbitrary_exponent_enabled;
    load_shader_program();
}

void JuliaShader::set_coloring_method( const ColoringMethod coloring_method )
{
    coloring_method_ = coloring_method;
    load_shader_program();
}

void JuliaShader::set_escape_condition( const EscapeCondition escape_condition )
{
    escape_condition_ = escape_condition;
    load_shader_program();
}

void JuliaShader::set_palette_mode( const PaletteMode palette_mode )
{
    palette_mode_ = palette_mode;
    load_shader_program();
}

void JuliaShader::set_multisampling_mode( const MultisamplingMode multisampling_mode )
{
    multisampling_mode_ = multisampling_mode;
    load_shader_program();
}

void JuliaShader::set_palette_texture( const ByteVector& image_data, const unsigned width, const unsigned height )
{
    if ( image_data.size() < width * height * 3 ) throw std::length_error( "image_data is too short for width and height" );
    glEnable( GL_TEXTURE_2D );
    glGenTextures( 1, &palette_texture_ );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glBindTexture( GL_TEXTURE_2D, palette_texture_ );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, &image_data[0] );
}

void JuliaShader::load_shader_program()
{
    google::TemplateDictionary dictionary( "julia" );

    dictionary.ShowSection( map_lookup( escape_conditions, escape_condition_ ) );
    dictionary.ShowSection( map_lookup( coloring_methods, coloring_method_ ) );
    dictionary.ShowSection( map_lookup( palette_modes, palette_mode_ ) );
    dictionary.ShowSection( map_lookup( multisampling_modes, multisampling_mode_ ) );

    if ( mandelbrot_mode_enabled_ ) dictionary.ShowSection( "MANDELBROT_MODE_ENABLED" );
    else dictionary.ShowSection( "MANDELBROT_MODE_DISABLED" );

    if ( normal_mapping_enabled_ ) dictionary.ShowSection( "NORMAL_MAPPING_ENABLED" );
    else dictionary.ShowSection( "NORMAL_MAPPING_DISABLED" );

    if ( arbitrary_exponent_enabled_ ) dictionary.ShowSection( "ITERATOR_POLAR" );
    else dictionary.ShowSection( "ITERATOR_CARTESIAN" );

    shader_.load_from_template( "src/backend/generators/julia.glsl.tpl", dictionary );
}

void JuliaShader::set_uniform_variables( const float pixel_width )
{
    shader_.set_uniform_vec2d( "seed", seed_ );
    shader_.set_uniform_int( "max_iterations", max_iterations_ );
    shader_.set_uniform_float( "palette_offset", palette_offset_ );
    shader_.set_uniform_float( "palette_stretch", palette_stretch_ );
    shader_.set_uniform_float( "julia_exponent", julia_exponent_ );
    shader_.set_uniform_float( "height_scale", height_scale_ );
    shader_.set_uniform_float( "pixel_width", pixel_width );

    shader_.set_uniform_float( "red_phase", red_phase_ );
    shader_.set_uniform_float( "blue_phase", blue_phase_ );
    shader_.set_uniform_float( "green_phase", green_phase_ );
    shader_.set_uniform_float( "red_amplitude", red_amplitude_ );
    shader_.set_uniform_float( "blue_amplitude", blue_amplitude_ );
    shader_.set_uniform_float( "green_amplitude", green_amplitude_ );
    shader_.set_uniform_float( "red_frequency", red_frequency_ );
    shader_.set_uniform_float( "blue_frequency", blue_frequency_ );
    shader_.set_uniform_float( "green_frequency", green_frequency_ );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, palette_texture_ );
    shader_.set_uniform_int( "palette", 0 );

    // FIXME When a uniform variable that does not exist is accessed it causes the PyOpenGl stuff to freak out...
    glGetError();
}

#include <sys/time.h>
#include <time.h>

void JuliaShader::draw( const Vector2Di& screen_size, const Vector2Df& viewport_position, const Vector2Df& viewport_size )
{
    shader_.enable();

    float pixel_width = viewport_size.x_ / static_cast<float>( screen_size.x_ ); // Assumes square pixels.
    set_uniform_variables( pixel_width );

    // FIXME Just here for testing.
    // struct timeval begin;
    // gettimeofday( &begin, NULL );

    shader_.draw( screen_size, viewport_position, viewport_size );

    shader_.disable();

    // FIXME Just here for testing.
    glFinish();
    // struct timeval end;
    // gettimeofday( &end, NULL );
    // VNOTIFY( FAULT, "shader draw took %d ms.", ( end.tv_sec - begin.tv_sec ) * 1000 + ( end.tv_usec - begin.tv_usec ) / 1000 );
}
