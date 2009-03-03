#ifndef JULIA_SHADER_H
#define JULIA_SHADER_H

#include <CEGUI.h>

#include "common/geometry.h"
#include "common/utilities.h"
#include "afe/generators/fractal_generator.h"
#include "afe/generators/shader.h"

enum coloring_method
{
    CM_ITERATIONS,
    CM_RADIUS_SQUARED
};

enum escape_condition
{
    EC_BOX,
    EC_CIRCLE
};

struct JuliaShader : public FractalGenerator
{
    static cstring GENERATOR_NAME;

    JuliaShader();
    virtual ~JuliaShader();

    virtual void doOneStep( double step_time );
    virtual void draw( const Vector2Di& screen_size, const Vector2Df& viewport_position, const Vector2Df& viewport_size );
    virtual bool handleMouseMotionEvent( const Vector2Df& position );
    virtual bool handleKeyDownEvent( const int key );
    virtual bool handleKeyUpEvent( const int key );

    void setSeed( const Vector2Df& seed );

private:
    static const int 
        MIN_ITERATIONS = 1,
        MAX_ITERATIONS = 512;

    Shader shader_;

    TextureMap palette_;

    float palette_offset_;

    int num_iterations_;

    Vector2Df seed_;

    bool
        mouse_moving_seed_,
        cycle_palette_;

    coloring_method coloring_method_;
    escape_condition escape_condition_;

    void initialize_gui();
    void destroy_gui();

    bool handleIterationsSlider( const CEGUI::EventArgs& e );
    bool handleColoringMethod( const CEGUI::EventArgs& e );
    bool handleEscapeCondition( const CEGUI::EventArgs& e );
    bool handleSeedReal( const CEGUI::EventArgs& e );
    bool handleCyclePalette( const CEGUI::EventArgs& e );
    
    void load_shader_program();
    void set_uniform_variables( const float pixel_width );
};

#endif // JULIA_SHADER_H
