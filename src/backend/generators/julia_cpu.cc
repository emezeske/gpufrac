#include <GL/glew.h>
#include <GL/gl.h>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

#include "backend/generators/julia_cpu.h"

JuliaCpu::JuliaCpu() :
    seed_( 0.0f, 0.0f ),
    max_iterations_( 128 )
{
    glEnable( GL_TEXTURE_2D );
    glGenTextures( 1, &texture_ );
}

void JuliaCpu::generate( const Vector2Di& screen_size,
               const Vector2Df& viewport_position,
               const Vector2Df& viewport_size,
               const int row_offset,
               const int num_rows,
               unsigned char *pixels )
{
    for ( int y = row_offset; y < row_offset + num_rows; ++y )
    {
        for ( int x = 0; x < screen_size.x_; ++x )
        {
            double
                z_real = viewport_position.x_ + static_cast<double>( x ) / screen_size.x_ * viewport_size.x_,
                z_imag = viewport_position.y_ + static_cast<double>( y ) / screen_size.y_ * viewport_size.y_;

            int i = 0;

            // TODO At the very least, assemble this loop by hand:
            for ( ; i < max_iterations_; ++i )
            {
                double
                    z_real_squared = z_real * z_real,
                    z_imag_squared = z_imag * z_imag,
                    radius_squared = z_real_squared + z_imag_squared;

                if ( radius_squared < 4.0 )
                {
                    z_imag = 2.0 * z_real * z_imag + seed_.y_;
                    z_real = z_real_squared - z_imag_squared + seed_.x_;
                }
                else break;
            }

            if ( i < max_iterations_ )
            {
                pixels[y * screen_size.x_ * 3 + x * 3 + 0] = static_cast<char>( static_cast<double>( i ) / max_iterations_ * 255 );
                pixels[y * screen_size.x_ * 3 + x * 3 + 1] = static_cast<char>( static_cast<double>( i ) / max_iterations_ * 255 );
                pixels[y * screen_size.x_ * 3 + x * 3 + 2] = static_cast<char>( static_cast<double>( i ) / max_iterations_ * 255 );
            }
            else
            {
                pixels[y * screen_size.x_ * 3 + x * 3 + 0] = 0;
                pixels[y * screen_size.x_ * 3 + x * 3 + 1] = 0;
                pixels[y * screen_size.x_ * 3 + x * 3 + 2] = 0;
            }
        }
    }
}

void JuliaCpu::draw( const Vector2Di& screen_size, const Vector2Df& viewport_position, const Vector2Df& viewport_size )
{
    unsigned char pixels[screen_size.y_ * screen_size.x_ * 3]; // FIXME This is from the C99 standard, and is not valid C++ (but g++ supports it as an extension).
    //std::auto_ptr<unsigned char> pixels( new unsigned char[screen_size.y_ * screen_size.x_ * 3] );

    const int 
        num_threads = boost::thread::hardware_concurrency(),
        normal_row_size = screen_size.y_ / num_threads,
        last_row_size = normal_row_size + screen_size.y_ % normal_row_size;

    // TODO This should be a thread pool, to avoid the overhead of creating new threads (not a big problem on Linux).
    boost::thread_group threads;

    for ( int i = 0; i < num_threads; ++i )
    {
        const int this_row_size = ( i + 1 < num_threads ) ? normal_row_size : last_row_size;

        threads.create_thread( boost::bind( &JuliaCpu::generate, 
                                            this,
                                            screen_size,
                                            viewport_position,
                                            viewport_size,
                                            normal_row_size * i,
                                            this_row_size,
                                            pixels ) );
    }

    threads.join_all();

    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, texture_ );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND );

    // FIXME Some graphics cards don't support non-power-of-2-textures...
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, screen_size.x_, screen_size.y_, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels );

    glBegin( GL_QUADS );
        glTexCoord2f( 0.0, 0.0 );
        glVertex2i( 0, 0 );

        glTexCoord2f( 1.0, 0.0 );
        glVertex2i( screen_size.x_, 0 );

        glTexCoord2f( 1.0, 1.0 );
        glVertex2i( screen_size.x_, screen_size.y_ );

        glTexCoord2f( 0.0, 1.0 );
        glVertex2i( 0, screen_size.y_ );
    glEnd();
}
