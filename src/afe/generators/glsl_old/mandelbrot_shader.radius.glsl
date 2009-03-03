uniform sampler2D palette;
uniform int max_iterations;
uniform float palette_offset;

void main ()
{
    vec2 c = gl_TexCoord[0].st;
    vec2 z = c;

    gl_FragColor = texture2D( palette, palette_offset, 0.0 );

    for ( int i = 0; i < max_iterations; ++i )
    {
        float 
            z_x_squared = z.x * z.x,
            z_y_squared = z.y * z.y,
            radius_squared = z_x_squared + z_y_squared;

        z = vec2( z_x_squared - z_y_squared, 2.0 * z.x * z.y ) + c;

        if ( radius_squared > 4.0 )
        {
            gl_FragColor = texture2D( palette, radius_squared / 100.0 + palette_offset, 0.0 );
            break;
        }
    }
}
