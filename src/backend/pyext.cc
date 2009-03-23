#include <boost/python.hpp>
#include <boost/python/enum.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include "backend/geometry.h"
#include "backend/viewport.h"
#include "backend/generators/julia_shader.h"
#include "backend/generators/julia_cpu.h"

namespace py = boost::python;

BOOST_PYTHON_MODULE(afepy)
{
    py::class_<Shader, boost::noncopyable>( "Shader" )
        .def( "init", &Shader::init ).staticmethod( "init" );

    py::enum_<ColoringMethod>( "ColoringMethod" )
        .value( "CM_ITERATIVE",      CM_ITERATIVE )
        .value( "CM_CONTINUOUS",     CM_CONTINUOUS )
        .value( "CM_RADIUS_SQUARED", CM_RADIUS_SQUARED )
        .value( "CM_ANGLE",          CM_ANGLE );

    py::enum_<EscapeCondition>( "EscapeCondition" )
        .value( "EC_CIRCLE", EC_CIRCLE )
        .value( "EC_BOX",    EC_BOX );

    py::enum_<PaletteMode>( "PaletteMode" )
        .value( "PM_TEXTURE",   PM_TEXTURE )
        .value( "PM_TRIG",      PM_TRIG )
        .value( "PM_MAGNITUDE", PM_MAGNITUDE );

    py::enum_<MultisamplingMode>( "MultisamplingMode" )
        .value( "MS_NONE", MS_NONE )
        .value( "MS_4X",   MS_4X )
        .value( "MS_8X",   MS_8X );

    py::class_<JuliaShader, boost::noncopyable>( "JuliaShader" )
        .def( "draw",                           &JuliaShader::draw ) 
        .def( "set_seed",                       &JuliaShader::set_seed ) 
        .def( "set_palette_offset",             &JuliaShader::set_palette_offset )
        .def( "set_palette_stretch",            &JuliaShader::set_palette_stretch )
        .def( "set_julia_exponent",             &JuliaShader::set_julia_exponent )
        .def( "set_max_iterations",             &JuliaShader::set_max_iterations )
        .def( "set_height_scale",               &JuliaShader::set_height_scale )
        .def( "set_mandelbrot_mode_enabled",    &JuliaShader::set_mandelbrot_mode_enabled )
        .def( "set_normal_mapping_enabled",     &JuliaShader::set_normal_mapping_enabled )
        .def( "set_arbitrary_exponent_enabled", &JuliaShader::set_arbitrary_exponent_enabled )
        .def( "set_coloring_method",            &JuliaShader::set_coloring_method )
        .def( "set_escape_condition",           &JuliaShader::set_escape_condition )
        .def( "set_palette_mode",               &JuliaShader::set_palette_mode )
        .def( "set_palette_texture",            &JuliaShader::set_palette_texture )
        .def( "set_multisampling_mode",         &JuliaShader::set_multisampling_mode )
        .def( "set_red_phase",                  &JuliaShader::set_red_phase )
        .def( "set_green_phase",                &JuliaShader::set_green_phase )
        .def( "set_blue_phase",                 &JuliaShader::set_blue_phase )
        .def( "set_red_amplitude",              &JuliaShader::set_red_amplitude )
        .def( "set_green_amplitude",            &JuliaShader::set_green_amplitude )
        .def( "set_blue_amplitude",             &JuliaShader::set_blue_amplitude )
        .def( "set_red_frequency",              &JuliaShader::set_red_frequency )
        .def( "set_green_frequency",            &JuliaShader::set_green_frequency )
        .def( "set_blue_frequency",             &JuliaShader::set_blue_frequency )
        .def( "get_seed",                       &JuliaShader::get_seed )
        .def( "get_palette_offset",             &JuliaShader::get_palette_offset );

    py::class_<JuliaCpu, boost::noncopyable>( "JuliaCpu" )
        .def( "draw",                           &JuliaCpu::draw ) 
        .def( "set_seed",                       &JuliaCpu::set_seed ) 
        .def( "set_max_iterations",             &JuliaCpu::set_max_iterations );

    py::class_<Viewport, boost::noncopyable>( "Viewport", py::init<const Vector2Df&, const Vector2Df&>() )
        .def( "do_one_step",               &Viewport::do_one_step )
        .def( "scale_extents",             &Viewport::scale_extents )
        .def( "set_desired_pan_velocity",  &Viewport::set_desired_pan_velocity )
        .def( "set_desired_zoom_velocity", &Viewport::set_desired_zoom_velocity )
        .def( "position",                  &Viewport::position, py::return_value_policy<py::copy_const_reference>() )
        .def( "size",                      &Viewport::size, py::return_value_policy<py::copy_const_reference>() );

    py::class_<Vector2Di>( "Vector2Di", py::init<int, int>() )
        .def_readwrite( "x", &Vector2Di::x_ )
        .def_readwrite( "y", &Vector2Di::y_ );

    py::class_<Vector2Df>( "Vector2Df", py::init<float, float>() )
        .def_readwrite( "x", &Vector2Df::x_ )
        .def_readwrite( "y", &Vector2Df::y_ );
        // .def( py::self + py::self )
        // .def( py::self += py::self );

    py::class_<ByteVector, boost::noncopyable>( "ByteVector" )
        .def( py::vector_indexing_suite<ByteVector>() );
}
