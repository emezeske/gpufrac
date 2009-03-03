#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "common/geometry.h"

struct Viewport
{
    Viewport( const Vector2Df& _position, const Vector2Df& _size );

    void doOneStep( float step_time );

    void setDesiredPanVelocity( const Vector2Df& desired_pan_velocity );
    void setDesiredZoomVelocity( const float desired_zoom_velocity );

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

    float getZoomFactor() const;
    void adjustZoomVelocity( float step_time );
    void adjustPanVelocity( float step_time );
};

#endif // VIEWPORT_H
