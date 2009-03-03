#ifndef SHADER_H
#define SHADER_H

#include <SDL/SDL_opengl.h>
#include <string>
#include <google/template.h>  

#include "common/utilities.h"
#include "common/drawing_utilities.h"

struct Shader
{
    Shader();
    ~Shader();

    void draw( const Vector2Di& screen_size, const Vector2Df& viewport_position, const Vector2Df& viewport_size );

    void load_program( cstring& shader_program);
    void load_from_file( cstring& filename );
    void load_from_template( cstring& filename, const google::TemplateDictionary& dictionary );

    void enable();
    void disable();

    void set_uniform_vec2d( cstring& name, const Vector2Df& value );
    void set_uniform_float( cstring& name, const float v );
    void set_uniform_int( cstring& name, const int v );

protected:
    GLuint
        gl_shader_,
        gl_shader_program_;

    void check_support();
    void print_with_line_numbers( cstring& shader_program);
    void ensure_shader_compiled();
    void ensure_shader_linked();
};

#endif // SHADER_H

