#ifndef FRACTAL_GENERATOR_H
#define FRACTAL_GENERATOR_H

#include <boost/shared_ptr.hpp>

#include "common/geometry.h"

struct FractalGenerator
{
    virtual ~FractalGenerator();
    virtual void doOneStep( double step_time ) = 0;
    virtual void draw( const Vector2Di& screen_size, const Vector2Df& viewport_position, const Vector2Df& viewport_size ) = 0;
    virtual bool handleMouseMotionEvent( const Vector2Df& position ) { return false; }
    virtual bool handleKeyDownEvent( const int key ) { return false; }
    virtual bool handleKeyUpEvent( const int key ) { return false; }
};

typedef boost::shared_ptr<FractalGenerator> pFractalGenerator;

#endif // FRACTAL_GENERATOR_H
