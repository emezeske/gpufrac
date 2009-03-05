#ifndef JULIA_SHADER_H
#define JULIA_SHADER_H

#include <CEGUI.h>

#include "common/geometry.h"
#include "common/utilities.h"
#include "afe/generators/fractal_generator.h"
#include "afe/generators/shader.h"

enum coloring_method
{
    CM_ITERATIVE,
    CM_CONTINUOUS
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

    static const float
      MAX_PALETTE_CYCLE_SPEED = 1.0f,
      PALETTE_CYCLE_SPEED_DEADZONE = 0.1f,
      MIN_COLOR_EXPONENT = 0.1f,
      MAX_COLOR_EXPONENT = 10.0f,
      MIN_JULIA_EXPONENT = -10.0f,
      MAX_JULIA_EXPONENT = 10.0f;

    Shader shader_;

    TextureMap palette_;

    float 
      palette_offset_,
      palette_cycle_speed_,
      color_exponent_,
      julia_exponent_;

    int num_iterations_;
    
    Vector2Df seed_;

    bool
        mouse_moving_seed_,
        enable_multisampling_,
        enable_normal_mapping_;

    coloring_method coloring_method_;
    escape_condition escape_condition_;

    void initialize_gui();
    void destroy_gui();

    bool handleIterationsSlider( const CEGUI::EventArgs& e );
    bool handleColorSlider( const CEGUI::EventArgs& e);
    bool handleExponentSlider( const CEGUI::EventArgs& e);
    bool handlePaletteCycleSpeedSlider( const CEGUI::EventArgs& e );
    bool handleColoringMethod( const CEGUI::EventArgs& e );
    bool handleEscapeCondition( const CEGUI::EventArgs& e );
    bool handleSeedReal( const CEGUI::EventArgs& e );
    bool handleEnableMultisampling( const CEGUI::EventArgs& e );
    bool handleEnableNormalMapping( const CEGUI::EventArgs& e );
    
    void load_shader_program();
    void set_uniform_variables( const float pixel_width );
};

#endif // JULIA_SHADER_H
