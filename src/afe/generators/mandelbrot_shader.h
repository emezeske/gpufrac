#ifndef MANDLEBROT_SHADER_H
#define MANDLEBROT_SHADER_H

#include "common/utilities.h"
#include "afe/generators/fractal_generator.h"
#include "afe/generators/shader.h"

struct MandelbrotShader : public FractalGenerator
{
    static cstring GENERATOR_NAME;

    MandelbrotShader();
    virtual ~MandelbrotShader();

    virtual void doOneStep( double step_time );
    virtual void draw( const Vector2Di& screen_size, const Vector2Df& viewport_position, const Vector2Df& viewport_size );

private:
    TextureMap palette_;

    float palette_offset_;

    int num_iterations_;

    Shader shader_;

    void initialize_gui();
    void destroy_gui();

    void set_uniform_variables();
};

#endif // MANDLEBROT_SHADER_H
