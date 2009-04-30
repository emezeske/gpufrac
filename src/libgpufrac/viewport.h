#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "geometry.h"

struct Viewport
{
    Viewport( const Vector2Df& _position, const Vector2Df& _size );

    void do_one_step( float step_time );

    void scale_extents( const Vector2Df& scale );

    void zoom( const float factor, const Vector2Df& locus );
    void pan( const Vector2Df& distance );

    void set_desired_zoom_velocity( const float desired_zoom_velocity );
    void set_desired_pan_velocity( const Vector2Df& desired_pan_velocity );

    const Vector2Df& position() const { return position_; }
    const Vector2Df& size() const { return size_; }

private:
    static const float
        ZOOM_ACCELERATION = 5.0f,
        PAN_ACCELERATION = 5.0f,
        ZERO_ENOUGH = 0.01f;

    float 
        desired_zoom_velocity_,
        zoom_velocity_;

    Vector2Df 
        position_,
        size_,
        original_size_,
        desired_pan_velocity_,
        pan_velocity_;

    float get_zoom_factor() const;
    void adjust_zoom_velocity( float step_time );
    void adjust_pan_velocity( float step_time );
};

#endif // VIEWPORT_H
