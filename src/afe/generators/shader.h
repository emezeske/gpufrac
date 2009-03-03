#ifndef SHADER_H
#define SHADER_H

#include <SDL/SDL_opengl.h>
#include <string>

#include "common/utilities.h"
#include "common/drawing_utilities.h"

struct Shader
{
    Shader();
    ~Shader();

    void draw( const Vector2Di& screen_size, const Vector2Df& viewport_position, const Vector2Df& viewport_size );

    void load_program( cstring& shader_program);
    void load_from_file( cstring& filename );
    int variable_location( cstring& name );

    void enable();
    void disable();

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

