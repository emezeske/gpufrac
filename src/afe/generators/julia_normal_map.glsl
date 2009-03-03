uniform vec2 seed;
uniform sampler2D palette;
uniform int num_iterations;
uniform float palette_offset;
uniform float pixel_width;

float calculate_escape_magnitude( vec2 z )
{
    float escape_magnitude = 1.0;

    for ( int i = 0; i < num_iterations; ++i )
    {
        float
            z_x_squared = z.x * z.x,
            z_y_squared = z.y * z.y,
            radius_squared = z_x_squared + z_y_squared;

        escape_magnitude += pow( 2.71828182845904523536, -sqrt( radius_squared ) );

        if ( radius_squared > 4.0 ) break;

        z = vec2( z_x_squared - z_y_squared, 2.0 * z.x * z.y ) + seed;
    }

    return escape_magnitude;
}

vec3 get_color( float escape_magnitude )
{
    return texture2D( palette, vec2( escape_magnitude / 10.0 + palette_offset, 0.0 ) ).rgb;
    //return vec3( escape_magnitude, escape_magnitude, escape_magnitude );
}

void main ()
{
    float half_pixel = pixel_width / 2.0;

    vec2
        z_a = gl_TexCoord[0].st,
        z_b = vec2( z_a.x + half_pixel, z_a.y ),
        z_c = vec2( z_a.x, z_a.y + half_pixel );

    float 
        escape_magnitude_a = calculate_escape_magnitude( z_a ),
        escape_magnitude_b = calculate_escape_magnitude( z_b ),
        escape_magnitude_c = calculate_escape_magnitude( z_c );

    vec3
        point_a = vec3( z_a.x, z_a.y, float( escape_magnitude_a ) ),
        point_b = vec3( z_b.x, z_b.y, float( escape_magnitude_b ) ),
        point_c = vec3( z_c.x, z_c.y, float( escape_magnitude_c ) );

    vec3 normal = normalize( cross( point_b - point_a, point_c - point_a ) );
    float cosine = dot( normal, normalize( vec3( 0.0, 0.0, 1.00 ) ) );

    vec3 base_color = ( get_color( escape_magnitude_a ) + 
                        get_color( escape_magnitude_b ) + 
                        get_color( escape_magnitude_c ) ) / 3.0;

    //gl_FragColor = vec4( base_color, 1.0 );
    //gl_FragColor = vec4( base_color * pow( cosine, 0.1 ), 1.0 );
    //gl_FragColor = vec4( base_color * cosine, 1.0 );
    gl_FragColor = vec4( vec3( 1.0, 1.0, 1.0 ) * cosine, 1.0 );
    //gl_FragColor = vec4( base_color * 0.1 + base_color * cosine, 1.0 );

    //if ( cosine < 0.15 ) gl_FragColor = vec4( 0.0, 1.0, 1.0, 1.0 );
    //else gl_FragColor = vec4( base_color * 0.1 + base_color * cosine, 1.0 );
}
