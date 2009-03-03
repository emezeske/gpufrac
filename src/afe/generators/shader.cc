#include <SDL/SDL.h>
#include <GL/glew.h>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include "common/exceptions.h"
#include "afe/generators/shader.h"

Shader::Shader()
{
    check_support();
}

Shader::~Shader()
{
    disable();
    glDeleteProgram( gl_shader_program_ );
}

void Shader::check_support()
{
    glewInit();
    if ( !glewIsSupported( "GL_VERSION_2_0" ) ) throw LoadingError( "OpenGL 2.0 not supported" );
}

void Shader::draw( const Vector2Di& screen_size, const Vector2Df& viewport_position, const Vector2Df& viewport_size )
{
    glBegin( GL_QUADS );
        glTexCoord2f( viewport_position.x_, viewport_position.y_ );
        glVertex2i( 0, 0 );

        glTexCoord2f( viewport_position.x_ + viewport_size.x_, viewport_position.y_ );
        glVertex2i( screen_size.x_, 0 );

        glTexCoord2f( viewport_position.x_ + viewport_size.x_, viewport_position.y_ + viewport_size.y_ );
        glVertex2i( screen_size.x_, screen_size.y_ );

        glTexCoord2f( viewport_position.x_, viewport_position.y_ + viewport_size.y_ );
        glVertex2i( 0, screen_size.y_ );
    glEnd();
}

void Shader::enable()
{
    glUseProgram( gl_shader_program_ );
}

void Shader::disable()
{
    glUseProgram( 0 );
}

void Shader::load_program( cstring& shader_program )
{
    const GLchar* program = shader_program.c_str();

    print_with_line_numbers( shader_program );

    gl_shader_ = glCreateShader( GL_FRAGMENT_SHADER );
    glShaderSource( gl_shader_, 1, &program, NULL );
    glCompileShader( gl_shader_ );

    ensure_shader_compiled();

    gl_shader_program_ = glCreateProgram();
    glAttachShader( gl_shader_program_, gl_shader_ );
    glLinkProgram( gl_shader_program_ );

    ensure_shader_linked();
}

void Shader::load_from_file( cstring& filename )
{
    std::ifstream stream( filename.c_str() );

    if ( stream.is_open() )
    {
        load_program( cstring( std::istreambuf_iterator<char>( stream ), std::istreambuf_iterator<char>() ) );
    }
    else throw LoadingError( "Unable to open shader program file " + filename );
}

void Shader::load_from_template( cstring& filename, const google::TemplateDictionary& dictionary )
{
    google::Template* glsl_template = google::Template::GetTemplate( filename.c_str(), google::DO_NOT_STRIP );
    mstring expanded_glsl;
    glsl_template->Expand( &expanded_glsl, &dictionary );
    load_program( expanded_glsl );
}

void Shader::print_with_line_numbers( cstring& shader_program )
{
    typedef std::vector<mstring> stringv;

    stringv lines;
    boost::algorithm::split( lines, shader_program, boost::algorithm::is_any_of( "\n" ) );

    NOTIFY( FAULT, "Loading shader program:" );

    for ( stringv::size_type i = 0; i < lines.size(); ++i )
    {
        VNOTIFY( FAULT, "%3d: %s", i + 1, lines[i].c_str() );
    }
}

void Shader::ensure_shader_compiled()
{
    int status;

    glGetShaderiv( gl_shader_, GL_COMPILE_STATUS, &status );

    if ( status == GL_FALSE )
    {
        int length;

        glGetShaderiv( gl_shader_, GL_INFO_LOG_LENGTH, &length );

        if ( length > 0 )
        {
            int written;
            std::vector<char> log( length );
            glGetShaderInfoLog( gl_shader_, length, &written, &log[0] );
            VNOTIFY( FAULT, "Shader compilation failed:\n%s", &log[0] );
        }

        throw LoadingError( "Shader compilation failed" );
    }
}

void Shader::ensure_shader_linked()
{
    int status;

    glGetProgramiv( gl_shader_, GL_LINK_STATUS, &status );

    if ( status == GL_FALSE )
    {
        int length;

        glGetProgramiv( gl_shader_, GL_INFO_LOG_LENGTH, &length );

        if ( length > 0 )
        {
            int written;
            std::vector<char> log( length );
            glGetProgramInfoLog( gl_shader_, length, &written, &log[0] );
            VNOTIFY( FAULT, "Shader linking failed:\n%s", &log[0] );
        }

        throw LoadingError( "Shader linking failed" );
    }
}

int Shader::variable_location( cstring& name )
{
    int location = glGetUniformLocation( gl_shader_program_, name.c_str() );
    // TODO
    //if ( location == -1 ) throw std::runtime_error( "Could not find uniform variable '" + name + "'" );
    if ( location == -1 ) VNOTIFY( FAULT, "WARNING: Could not find uniform variable '%s'", name );
    return location;
}
