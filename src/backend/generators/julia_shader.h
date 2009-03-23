#ifndef JULIA_SHADER_H
#define JULIA_SHADER_H

#include "backend/geometry.h"
#include "backend/utilities.h"
#include "backend/generators/shader.h"

typedef std::vector<char> ByteVector;

enum ColoringMethod
{
    CM_ITERATIVE,
    CM_CONTINUOUS,
    CM_RADIUS_SQUARED,
    CM_ANGLE
};

enum EscapeCondition
{
    EC_BOX,
    EC_CIRCLE
};

enum PaletteMode
{
    PM_TEXTURE,
    PM_TRIG,
    PM_MAGNITUDE
};

enum MultisamplingMode
{
    MS_NONE,
    MS_4X,
    MS_8X
};

struct JuliaShader
{
    JuliaShader();

    void draw( const Vector2Di& screen_size, const Vector2Df& viewport_position, const Vector2Df& viewport_size );

    void set_seed( const Vector2Df& seed ) { seed_ = seed; }
    void set_palette_offset( const float palette_offset ) { palette_offset_ = palette_offset; }
    void set_palette_stretch( const float palette_stretch ) { palette_stretch_ = palette_stretch; }
    void set_julia_exponent( const float julia_exponent ) { julia_exponent_ = julia_exponent; }
    void set_height_scale( const float height_scale ) { height_scale_ = height_scale; }
    void set_max_iterations( const int max_iterations ) { max_iterations_ = max_iterations; }
    void set_mandelbrot_mode_enabled( const bool mandelbrot_mode_enabled );
    void set_normal_mapping_enabled( const bool normal_mapping_enabled );
    void set_arbitrary_exponent_enabled( const bool arbitrary_exponent_enabled );
    void set_coloring_method( const ColoringMethod coloring_method );
    void set_escape_condition( const EscapeCondition escape_condition );
    void set_palette_mode( const PaletteMode palette_mode );
    void set_multisampling_mode( const MultisamplingMode multisampling_mode );
    void set_palette_texture( const ByteVector& image_data, const unsigned width, const unsigned height );
    void set_red_phase( const float red_phase ) { red_phase_ = red_phase; }
    void set_green_phase( const float green_phase ) { green_phase_ = green_phase; }
    void set_blue_phase( const float blue_phase ) { blue_phase_ = blue_phase; }
    void set_red_amplitude( const float red_amplitude ) { red_amplitude_ = red_amplitude; }
    void set_green_amplitude( const float green_amplitude ) { green_amplitude_ = green_amplitude; }
    void set_blue_amplitude( const float blue_amplitude ) { blue_amplitude_ = blue_amplitude; }
    void set_red_frequency( const float red_frequency ) { red_frequency_ = red_frequency; }
    void set_green_frequency( const float green_frequency ) { green_frequency_ = green_frequency; }
    void set_blue_frequency( const float blue_frequency ) { blue_frequency_ = blue_frequency; }

    Vector2Df get_seed() const { return seed_; }
    float get_palette_offset() const { return palette_offset_; }

private:
    Shader shader_;

    GLuint palette_texture_;

    Vector2Df seed_;

    float 
        palette_offset_,
        palette_stretch_,
        julia_exponent_,
        height_scale_,
        red_phase_,
        blue_phase_,
        green_phase_,
        red_amplitude_,
        blue_amplitude_,
        green_amplitude_,
        red_frequency_,
        blue_frequency_,
        green_frequency_;

    int max_iterations_;

    bool
        mandelbrot_mode_enabled_,
        normal_mapping_enabled_,
        arbitrary_exponent_enabled_;
    
    ColoringMethod coloring_method_;
    EscapeCondition escape_condition_;
    PaletteMode palette_mode_;
    MultisamplingMode multisampling_mode_;

    void load_shader_program();
    void set_uniform_variables( const float pixel_width );
};

#endif // JULIA_SHADER_H
