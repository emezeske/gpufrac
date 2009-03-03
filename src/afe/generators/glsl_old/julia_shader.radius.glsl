uniform vec2 seed;
uniform sampler2D palette;
uniform int num_iterations;

void main ()
{
    vec2 z = gl_TexCoord[0].st;

    gl_FragColor = texture2D( palette, 0.0, 0.0 );

    for ( int i = 0; i < num_iterations; ++i )
    {
        float 
            z_x_squared = z.x * z.x,
            z_y_squared = z.y * z.y,
            radius_squared = z_x_squared + z_y_squared;

        z = vec2( z_x_squared - z_y_squared, 2.0 * z.x * z.y ) + seed;

        if ( radius_squared > 4.0 )
        {
            gl_FragColor = texture2D( palette, radius_squared / 10.0, 0.0 );
            break;
        }
    }
}

