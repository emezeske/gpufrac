uniform vec2 seed;
uniform sampler2D palette;
uniform int max_iterations;
uniform float palette_offset;
uniform float palette_stretch;
uniform float pixel_width;
uniform float color_exponent;
uniform float julia_exponent;
uniform float red_phase;
uniform float blue_phase;
uniform float green_phase;
uniform float red_amplitude;
uniform float blue_amplitude;
uniform float green_amplitude;
uniform float red_frequency;
uniform float blue_frequency;
uniform float green_frequency;

float calculate_escape_magnitude( vec2 z )
{
    float 
        escape_magnitude = 0.0,
        half_julia_exponent = julia_exponent / 2.0;

    for ( int i = 0; i < max_iterations; ++i )
    {
        float
            z_x_squared = z.x * z.x,
            z_y_squared = z.y * z.y,
            radius_squared = z_x_squared + z_y_squared;

        {{#COLORING_METHOD_CONTINUOUS}}
            escape_magnitude += exp( -sqrt( radius_squared ) ); // e ^( -radius )
            // escape_magnitude = i + 1 - log( log( sqrt( radius_squared ) ) / log( 2.0 ) ) / log( 2.0 );
        {{/COLORING_METHOD_CONTINUOUS}}

        {{#COLORING_METHOD_ITERATIVE}}
            escape_magnitude = float( i ) / float( max_iterations );
            // escape_magnitude = 2 * pow( 1 - ( float( i ) / float( max_iterations ) ), color_exponent );
        {{/COLORING_METHOD_ITERATIVE}}

        {{#ESCAPE_CONDITION_CIRCLE}}
            if ( radius_squared > 4.0 ) break;      
        {{/ESCAPE_CONDITION_CIRCLE}}

        {{#ESCAPE_CONDITION_BOX}}
            if ( z.x > 2.0 && z.y > 2.0 ) break;
        {{/ESCAPE_CONDITION_BOX}}

        {{#ITERATOR_POLAR}}
            float 
                new_theta = julia_exponent * atan( z.y, z.x ),
                new_radius = pow( radius_squared, half_julia_exponent );

            z = vec2( new_radius * cos( new_theta ), new_radius * sin( new_theta ) ) + seed;
        {{/ITERATOR_POLAR}}

        {{#ITERATOR_CARTESIAN}}
            z = vec2( z_x_squared - z_y_squared, 2.0 * z.x * z.y ) + seed;
        {{/ITERATOR_CARTESIAN}}
    }

    return escape_magnitude;
}

vec3 get_color( float escape_magnitude )
{
    escape_magnitude *= palette_stretch;
    escape_magnitude += palette_offset; // TODO Stretch offset?

    {{#PALETTE_MODE_TEXTURE}}
        // TODO
        // escape_magnitude /= max_iterations;
        return texture2D( palette, vec2( escape_magnitude, 0.0 ) ).rgb;
    {{/PALETTE_MODE_TEXTURE}}

    {{#PALETTE_MODE_TRIG}}
        return vec3( red_amplitude * sin( red_frequency * escape_magnitude + red_phase ),
                     green_amplitude * sin( green_frequency * escape_magnitude + green_phase ),
                     blue_amplitude * sin( blue_frequency * escape_magnitude + blue_phase ) );
    {{/PALETTE_MODE_TRIG}}

    {{#PALETTE_MODE_MAGNITUDE}}
        // TODO
        return vec3( escape_magnitude, escape_magnitude, escape_magnitude );
    {{/PALETTE_MODE_MAGNITUDE}}
}

void main ()
{
    float half_pixel = pixel_width / 2.0;

    vec2 z_a = gl_TexCoord[0].st;
    float escape_magnitude_a = calculate_escape_magnitude( z_a );
    vec3 base_color = get_color( escape_magnitude_a );

    {{#MULTISAMPLING_ENABLED}}
        vec2
            z_b = vec2( z_a.x + half_pixel, z_a.y ),
            z_c = vec2( z_a.x, z_a.y + half_pixel );

        float
            escape_magnitude_b = calculate_escape_magnitude( z_b ),
            escape_magnitude_c = calculate_escape_magnitude( z_c );

        base_color += ( get_color( escape_magnitude_b ) + get_color( escape_magnitude_c ) );
        base_color /= 3.0;

        {{#NORMAL_MAPPING_ENABLED}}
            vec3
                point_a = vec3( z_a.x, z_a.y, float( escape_magnitude_a ) ),
                point_b = vec3( z_b.x, z_b.y, float( escape_magnitude_b ) ),
                point_c = vec3( z_c.x, z_c.y, float( escape_magnitude_c ) );

            vec3 normal = normalize( cross( point_b - point_a, point_c - point_a ) );
            // Multiply the color by the cosine of the angle between the face's normal and the light vector.
            base_color *= dot( normal, normalize( vec3( 0.0, 0.0, 1.00 ) ) );
        {{/NORMAL_MAPPING_ENABLED}}

    {{/MULTISAMPLING_ENABLED}}

    gl_FragColor = vec4( base_color, 1.0 );
}
