#include <GL/glew.h>
#include <CEGUI.h>
#include <sstream>

#include "afe/generators/mandelbrot_shader.h"

namespace {

mstring escape_condition_code()
{
    //if ( z.x > 2.0 && z.y > 2.0 )
    //if ( z.x > 1.0 && z.y > 2.0 )
    return "if ( radius_squared >= 4.0 )";
}

mstring coloring_method_code()
{
    // gl_FragColor = texture2D( palette, radius_squared / 100.0 + palette_offset, 0.0 );
    return "gl_FragColor = texture2D( palette, vec2( i / float( num_iterations ) + palette_offset, 0.0 ) );";
}

void build_shader_program( std::ostream& stream )
{
    using std::endl;

    stream      << "uniform sampler2D palette;"
        << endl << "uniform int num_iterations;"
        << endl << "uniform float palette_offset;"
        << endl << ""
        << endl << "void main ()"
        << endl << "{"
        << endl << ""
        << endl << "    vec2 c = gl_TexCoord[0].st;"
        << endl << "    vec2 z = c;"
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
        << endl << "        z = vec2( z_x_squared - z_y_squared, 2.0 * z.x * z.y ) + c;"
        << endl << ""
        << endl << "        " << escape_condition_code()
        << endl << "        {"
        << endl << "            " << coloring_method_code()
        << endl << "            break;"
        << endl << "        }"
        << endl << "    }"
        << endl << "}"
        << endl;
}

} // anonymous namespace

cstring MandelbrotShader::GENERATOR_NAME = "Mandelbrot (OpenGL Shader)";

MandelbrotShader::MandelbrotShader() :
    palette_( "palette.png" ),
    palette_offset_( 0.0f ),
    num_iterations_( 8 )
{
    initialize_gui();

    std::ostringstream stream;
    build_shader_program( stream );
    shader_.load_program( stream.str() );
}

MandelbrotShader::~MandelbrotShader()
{
    destroy_gui();
}

void MandelbrotShader::initialize_gui()
{
}

void MandelbrotShader::destroy_gui()
{
}

void MandelbrotShader::set_uniform_variables()
{
    shader_.set_uniform_int( "num_iterations", num_iterations_ );
    shader_.set_uniform_float( "palette_offset", palette_offset_ );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, palette_.getTexture() );
    shader_.set_uniform_int( "palette", 0 );
}

void MandelbrotShader::doOneStep( double step_time )
{
    const float palette_cycle_speed = 0.1f;
    palette_offset_ += static_cast<float>( palette_cycle_speed * step_time );
    if ( palette_offset_ >= 1.0f ) palette_offset_ = 0.0f;
}

void MandelbrotShader::draw( const Vector2Di& screen_size, const Vector2Df& viewport_position, const Vector2Df& viewport_size )
{
    shader_.enable();
    set_uniform_variables();
    shader_.draw( screen_size, viewport_position, viewport_size );
    shader_.disable();
}

