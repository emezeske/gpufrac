#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <cmath>
#include <list>
#include <vector>

const float FUZZ_FACTOR = 0.001f;

template <class T>
struct Vector2D
{
    T
        x_,
        y_;

    Vector2D() : x_( static_cast<T>( 0 ) ), y_( static_cast<T>( 0 ) ) {};

    Vector2D( T x, T y ) : x_( x ), y_( y ) {};

    Vector2D( const Vector2D &rhs ) 
    {
        x_ = rhs.x_;
        y_ = rhs.y_;
    }

    bool operator == ( const Vector2D &rhs )
    {
        return x_ == rhs.x_ && y_ == rhs.y_;
    }

    T length_squared() const
    {
        return x_ * x_ + y_ * y_;
    }

    T length() const
    {
        return sqrt( length_squared() ); // TODO Provide specialization for sqrtf();
    }

    T slope() const
    {
        return y_ / x_;
    }

    Vector2D &normalize()
    {
        T len = length();
        if ( len > 0.0f )
        {
            x_ /= len;
            y_ /= len;
        }
        return *this;
    }

    Vector2D normal() const
    {
        return Vector2D( -y_, x_ );
    }
    
    Vector2D operator - ( const Vector2D &rhs ) const
    {
        return Vector2D( x_ - rhs.x_, y_ - rhs.y_ );
    }
    
    Vector2D &operator -= ( const Vector2D &rhs )
    {
        return ( *this = *this - rhs );
    }

    Vector2D operator + ( const Vector2D &rhs ) const
    {
        return Vector2D( x_ + rhs.x_, y_ + rhs.y_ );
    }

    Vector2D &operator += ( const Vector2D &rhs )
    {
        return ( *this = *this + rhs );
    }

    Vector2D operator * ( const T s ) const
    {
        return Vector2D( x_ * s, y_ * s );
    }

    Vector2D &operator *= ( const T s )
    {
        return ( *this = *this * s );
    }

    // Dot Product
    T operator * ( const Vector2D &rhs ) const
    {
        return  x_ * rhs.x_ + y_ * rhs.y_;
    }

    Vector2D operator / ( const T s ) const
    {
        return Vector2D( x_ / s, y_ / s );
    }

    Vector2D &operator /= ( const T s )
    {
        return ( *this = *this / s );
    }

    float distance( Vector2D v )
    {
        float 
            dx = x_ - v.x_,
            dy = y_ - v.y_;
    
        return sqrt( dx * dx + dy * dy );
    }
};

typedef Vector2D<int> Vector2Di;
typedef Vector2D<float> Vector2Df;

typedef std::list<Vector2Di> Vector2DiList;
typedef std::list<Vector2Df> Vector2DfList;

template <typename T>
class Matrix2D
{
    public:
        Matrix2D( const Vector2Di &s ) :
            size_( s ),
            data_( size_.x_ * size_.y_, T() )
        {}

        bool includes( const int x, const int y ) const { return x >= 0 && x < size_.x_ && y >= 0 && y < size_.y_; }
        bool includes( const Vector2Di &i ) const { return includes( i.x_, i.y_ ); }

        T &at( const int x, const int y ) { return data_[x * size_.x_ + y]; }
        T &at( const Vector2Di &i ) { return at( i.x_, i.y_ ); }

        Vector2Di size() const { return size_; }

    private:
        Vector2Di size_;
        std::vector<T> data_;
};

Vector2Di roundVector( const Vector2Df &v );

inline float sign_of( const float f )
{
    return std::signbit( f ) ? -1.0f : 1.0f;
}

#endif // GEOMETRY_H
