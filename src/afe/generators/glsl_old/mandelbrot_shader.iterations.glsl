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
        z = vec2( z.x * z.x - z.y * z.y, 2.0 * z.x * z.y ) + c;

        if ( dot( z, z ) > 4.0 )
        {
            gl_FragColor = texture2D( palette, (float)i / max_iterations + palette_offset, 0.0 );
            break;
        }
    }
}
