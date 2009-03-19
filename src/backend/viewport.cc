#include "backend/utilities.h"
#include "backend/viewport.h"

Viewport::Viewport( const Vector2Df& _position, const Vector2Df& _size ) :
    desired_zoom_velocity_( 0.0f ),
    zoom_velocity_( 0.0f ),
    position_( _position ),
    size_( _size ),
    original_size_( _size ),
    desired_pan_velocity_( 0.0f, 0.0f ),
    pan_velocity_( 0.0f, 0.0f )
{
}

// TODO Acceleration / velocity should be scaled with respect to the current zoom level.

void Viewport::set_desired_zoom_velocity( const float desired_zoom_velocity )
{
    desired_zoom_velocity_ = desired_zoom_velocity;
}

void Viewport::set_desired_pan_velocity( const Vector2Df& desired_pan_velocity )
{
    desired_pan_velocity_ = desired_pan_velocity;
}

float Viewport::get_zoom_factor() const
{
    return static_cast<float>( original_size_.x_ ) / size_.x_;
}

void Viewport::adjust_zoom_velocity( float step_time )
{
    float zoom_velocity_difference = desired_zoom_velocity_ - zoom_velocity_;

    if ( zoom_velocity_difference > ZERO_ENOUGH )
    {
        float zoom_velocity_change = sign_of( zoom_velocity_difference ) * ZOOM_ACCELERATION * step_time;

        if ( fabs( zoom_velocity_change - zoom_velocity_difference ) > ZERO_ENOUGH )
        {
            zoom_velocity_ += zoom_velocity_change;
        }
        else zoom_velocity_ = desired_zoom_velocity_;
    }
    else zoom_velocity_ = desired_zoom_velocity_;
}

void Viewport::adjust_pan_velocity( float step_time )
{
    Vector2Df 
        scaled_desired_pan_velocity_ = desired_pan_velocity_ / get_zoom_factor(),
        pan_velocity_difference = scaled_desired_pan_velocity_ - pan_velocity_;

    if ( pan_velocity_difference.length() > ZERO_ENOUGH )
    {
        Vector2Df
            pan_acceleration_direction = pan_velocity_difference,
            pan_velocity_change = pan_acceleration_direction.normalize() * PAN_ACCELERATION * step_time;

        if ( pan_velocity_change.length() < pan_velocity_difference.length() )
        {
            pan_velocity_ += pan_velocity_change;
        }
        else pan_velocity_ = scaled_desired_pan_velocity_;
    }
    else pan_velocity_ = scaled_desired_pan_velocity_;
}

void Viewport::do_one_step( float step_time )
{
    adjust_zoom_velocity( step_time );
    Vector2Df size_change = size_ * zoom_velocity_ * step_time;
    position_ += size_change / 2.0f;
    size_ -= size_change;

    // The pan velocity depends on the current, zoom level, so the zoom velocity must be calculated first.

    adjust_pan_velocity( step_time );
    position_ += pan_velocity_ * step_time;
}
