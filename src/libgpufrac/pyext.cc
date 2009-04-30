#include <boost/python.hpp>
#include <boost/python/enum.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include "geometry.h"
#include "viewport.h"
#include "fractal_shader.h"

namespace py = boost::python;

BOOST_PYTHON_MODULE(gpufrac)
{
    py::class_<Shader, boost::noncopyable>( "Shader" )
        .def( "init", &Shader::init ).staticmethod( "init" );

    py::enum_<ColoringMode>( "ColoringMode" )
        .value( "CM_ITERATIVE",      CM_ITERATIVE )
        .value( "CM_CONTINUOUS",     CM_CONTINUOUS )
        .value( "CM_RADIUS_SQUARED", CM_RADIUS_SQUARED )
        .value( "CM_ANGLE",          CM_ANGLE );

    py::enum_<EscapeCondition>( "EscapeCondition" )
        .value( "EC_CIRCLE", EC_CIRCLE )
        .value( "EC_BOX",    EC_BOX );

    py::enum_<PaletteMode>( "PaletteMode" )
        .value( "PM_TEXTURE",    PM_TEXTURE )
        .value( "PM_TRIG",       PM_TRIG )
        .value( "PM_ORBIT_TRAP", PM_ORBIT_TRAP );

    py::enum_<MultisamplingMode>( "MultisamplingMode" )
        .value( "MS_NONE", MS_NONE )
        .value( "MS_4X",   MS_4X )
        .value( "MS_8X",   MS_8X )
        .value( "MS_16X",  MS_16X );

    py::class_<FractalShader, boost::noncopyable>( "FractalShader" )
        .def( "draw",                           &FractalShader::draw ) 
        .def( "set_seed",                       &FractalShader::set_seed ) 
        .def( "set_palette_offset",             &FractalShader::set_palette_offset )
        .def( "set_palette_stretch",            &FractalShader::set_palette_stretch )
        .def( "set_julia_exponent",             &FractalShader::set_julia_exponent )
        .def( "set_max_iterations",             &FractalShader::set_max_iterations )
        .def( "set_height_scale",               &FractalShader::set_height_scale )
        .def( "set_mandelbrot_mode_enabled",    &FractalShader::set_mandelbrot_mode_enabled )
        .def( "set_normal_mapping_enabled",     &FractalShader::set_normal_mapping_enabled )
        .def( "set_arbitrary_exponent_enabled", &FractalShader::set_arbitrary_exponent_enabled )
        .def( "set_coloring_mode",              &FractalShader::set_coloring_mode )
        .def( "set_escape_condition",           &FractalShader::set_escape_condition )
        .def( "set_palette_mode",               &FractalShader::set_palette_mode )
        .def( "set_palette_texture",            &FractalShader::set_palette_texture )
        .def( "set_orbit_trap_texture",         &FractalShader::set_orbit_trap_texture )
        .def( "set_multisampling_mode",         &FractalShader::set_multisampling_mode )
        .def( "set_red_phase",                  &FractalShader::set_red_phase )
        .def( "set_green_phase",                &FractalShader::set_green_phase )
        .def( "set_blue_phase",                 &FractalShader::set_blue_phase )
        .def( "set_red_amplitude",              &FractalShader::set_red_amplitude )
        .def( "set_green_amplitude",            &FractalShader::set_green_amplitude )
        .def( "set_blue_amplitude",             &FractalShader::set_blue_amplitude )
        .def( "set_red_frequency",              &FractalShader::set_red_frequency )
        .def( "set_green_frequency",            &FractalShader::set_green_frequency )
        .def( "set_blue_frequency",             &FractalShader::set_blue_frequency )
        .def( "get_seed",                       &FractalShader::get_seed )
        .def( "get_palette_offset",             &FractalShader::get_palette_offset );

    py::class_<Viewport, boost::noncopyable>( "Viewport", py::init<const Vector2Df&, const Vector2Df&>() )
        .def( "do_one_step",               &Viewport::do_one_step )
        .def( "scale_extents",             &Viewport::scale_extents )
        .def( "zoom",                      &Viewport::zoom )
        .def( "pan",                       &Viewport::pan )
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
