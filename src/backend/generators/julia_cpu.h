#ifndef JULIA_CPU_H
#define JULIA_CPU_H

#include "backend/geometry.h"
#include "backend/utilities.h"

struct JuliaCpu
{
    JuliaCpu();

    void draw( const Vector2Di& screen_size, const Vector2Df& viewport_position, const Vector2Df& viewport_size );

    void set_seed( const Vector2Df& seed ) { seed_ = seed; }
    void set_max_iterations( const int max_iterations ) { max_iterations_ = max_iterations; }

private:
    GLuint texture_;

    Vector2Df seed_;

    int max_iterations_;

    void generate( const Vector2Di& screen_size,
                   const Vector2Df& viewport_position,
                   const Vector2Df& viewport_size,
                   const int row_offset,
                   const int num_rows,
                   unsigned char *pixels );
};

#endif // JULIA_CPU_H
