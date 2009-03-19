#include <fstream>
#include <vector>
#include <stdexcept>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include "backend/generators/shader.h"

void Shader::init()
{
    GLenum glew_result = glewInit();

    if ( glew_result != GLEW_OK ) throw std::runtime_error( "Error initializing GLEW: " + cstring( reinterpret_cast<const char*>( glewGetErrorString( glew_result ) ) ) );

    if ( !glewIsSupported( "GL_VERSION_2_0" ) ) throw std::runtime_error( "OpenGL 2.0 not supported" );
}

Shader::Shader() :
    gl_shader_program_( 0 )
{
}

Shader::~Shader()
{
    disable();
    delete_program();
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
    if ( gl_shader_program_ ) glUseProgram( gl_shader_program_ );
}

void Shader::disable()
{
    glUseProgram( 0 );
}

void Shader::delete_program()
{
    if ( gl_shader_program_ != 0 ) glDeleteProgram( gl_shader_program_ );
}

void Shader::load_program( cstring& shader_program )
{
    const GLchar* program = shader_program.c_str();
    print_with_line_numbers( shader_program );

    delete_program();

    gl_shader_ = glCreateShader( GL_FRAGMENT_SHADER );

    if ( gl_shader_ != 0 )
    {
        glShaderSource( gl_shader_, 1, &program, NULL );
        glCompileShader( gl_shader_ );

        ensure_shader_compiled();

        gl_shader_program_ = glCreateProgram();

        if ( gl_shader_program_ != 0 )
        {
            glAttachShader( gl_shader_program_, gl_shader_ );
            glLinkProgram( gl_shader_program_ );
            ensure_shader_linked();
        }
        else throw std::runtime_error( "glCreateProgram() failed" );
    }
    else throw std::runtime_error( "glCreateShader() failed" );
}

void Shader::load_from_file( cstring& filename )
{
    std::ifstream stream( filename.c_str() );

    if ( stream.is_open() )
    {
        load_program( cstring( std::istreambuf_iterator<char>( stream ), std::istreambuf_iterator<char>() ) );
    }
    else throw std::runtime_error( "Unable to open shader program file " + filename );
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

    NOTIFY( VERBOSE, "Loading shader program:" );
    
    for ( stringv::size_type i = 0; i < lines.size(); ++i )
    {
        VNOTIFY( VERBOSE, "%3d: %s", i + 1, lines[i].c_str() );
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

        throw std::runtime_error( "Shader compilation failed" );
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

        throw std::runtime_error( "Shader linking failed" );
    }
}

void Shader::set_uniform_vec2d( cstring& name, const Vector2Df& value )
{
    int location = glGetUniformLocation( gl_shader_program_, name.c_str() );
    
    if ( location != -1 )
    {
        glUniform2f( location, value.x_, value.y_ );
    }
    // else VNOTIFY( FAULT, "WARNING: Could not find uniform vec2d '%s'", name.c_str() ); // TODO What to do here?
}

void Shader::set_uniform_float( cstring& name, const float v )
{
    int location = glGetUniformLocation( gl_shader_program_, name.c_str() );
    
    if ( location != -1 )
    {
        glUniform1f( location, v );
    }
    // else VNOTIFY( FAULT, "WARNING: Could not find uniform float '%s'", name.c_str() ); // TODO What to do here?
}

void Shader::set_uniform_int( cstring& name, const int v )
{
    int location = glGetUniformLocation( gl_shader_program_, name.c_str() );
    
    if ( location != -1 )
    {
        glUniform1i( location, v );
    }
    // else VNOTIFY( FAULT, "WARNING: Could not find uniform int '%s'", name.c_str() ); // TODO What to do here?
}

