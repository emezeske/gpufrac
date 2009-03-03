uniform vec2 seed;
uniform sampler2D palette;
uniform int num_iterations;

void main ()
{
    vec2 z = gl_TexCoord[0].st;

    gl_FragColor = texture2D( palette, 0.0, 0.0 );

    for ( int i = 0; i < num_iterations; ++i )
    {
        z = vec2( z.x * z.x - z.y * z.y, 2.0 * z.x * z.y ) + seed;

        if ( dot( z, z ) > 4.0 )
        //if ( z.x > 2.0 && z.y > 2.0 ) // Wow, different escape criterion produce magical results...
        //if ( z.x > 1.0 && z.y > 2.0 )
        {
            gl_FragColor = texture2D( palette, i / (float)num_iterations, 0.0 );
            break;
        }
    }
}
