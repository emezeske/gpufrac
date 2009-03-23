uniform vec2 seed;
uniform sampler2D palette;
uniform int max_iterations;
uniform float palette_offset;
uniform float palette_stretch;
uniform float julia_exponent;
uniform float height_scale;
uniform float pixel_width;
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

    int iteration = 0;

    float radius_squared = 0.0;

    for ( ; iteration < max_iterations; ++iteration )
    {
        float
            z_x_squared = z.x * z.x,
            z_y_squared = z.y * z.y;

        radius_squared = z_x_squared + z_y_squared;

        {{#COLORING_METHOD_CONTINUOUS}}
            escape_magnitude += exp( -sqrt( radius_squared ) ); // e ^( -radius )
            // escape_magnitude = iteration + 1 - log( log( sqrt( radius_squared ) ) / log( 2.0 ) ) / log( 2.0 );
        {{/COLORING_METHOD_CONTINUOUS}}

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

    {{#COLORING_METHOD_ITERATIVE}}
        escape_magnitude = float( iteration );
    {{/COLORING_METHOD_ITERATIVE}}

    {{#COLORING_METHOD_RADIUS_SQUARED}}
        escape_magnitude = radius_squared;
    {{/COLORING_METHOD_RADIUS_SQUARED}}

    {{#COLORING_METHOD_ANGLE}}
        escape_magnitude = atan( z.y, z.x );
    {{/COLORING_METHOD_ANGLE}}

    return escape_magnitude;
}

vec3 palette_lookup( float escape_magnitude )
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

{{#NORMAL_MAPPING_DISABLED}}
vec3 get_color( vec2 z )
{
    return palette_lookup( calculate_escape_magnitude( z ) );
}
{{/NORMAL_MAPPING_DISABLED}}

{{#NORMAL_MAPPING_ENABLED}}
vec3 get_color( vec2 z_a )
{
    //
    // TODO: These calculations could be simplified if we assume the light vector is always going to be (0,0,1).
    //

    float offset = pixel_width / 2.0;

    vec2
        z_b = vec2( z_a.x + offset, z_a.y ),
        z_c = vec2( z_a.x, z_a.y + offset );
    
    float
        escape_magnitude_a = calculate_escape_magnitude( z_a ),
        escape_magnitude_b = calculate_escape_magnitude( z_b ),
        escape_magnitude_c = calculate_escape_magnitude( z_c );
    
    vec3 base_color = palette_lookup( escape_magnitude_a ) + 
                      palette_lookup( escape_magnitude_b ) + 
                      palette_lookup( escape_magnitude_c );

    base_color /= 3.0;
    
    vec3
        point_a = vec3( z_a.x, z_a.y, escape_magnitude_a * height_scale ),
        point_b = vec3( z_b.x, z_b.y, escape_magnitude_b * height_scale ),
        point_c = vec3( z_c.x, z_c.y, escape_magnitude_c * height_scale );
    
    vec3 normal = normalize( cross( point_b - point_a, point_c - point_a ) );

    // Multiply the color by the cosine of the angle between the face's normal and the light vector.
    return base_color * dot( normal, vec3( 0.0, 0.0, 1.00 ) );
}
{{/NORMAL_MAPPING_ENABLED}}

vec3 get_color_multisample_4x( vec2 z_a )
{
    // 4X rotated grid samples:
    //  -  -  B  -
    //  A  -  -  -
    //  -  -  -  C
    //  -  D  -  -
    
    float quarter_pixel = pixel_width / 4.0;
    
    vec2
        z_b = vec2( z_a.x + 2.0 * quarter_pixel, z_a.y - quarter_pixel ),
        z_c = vec2( z_a.x + 3.0 * quarter_pixel, z_a.y + quarter_pixel ),
        z_d = vec2( z_a.x + quarter_pixel, z_a.y + 2.0 * quarter_pixel );
    
    vec3 color = get_color( z_a ) + get_color( z_b ) + get_color( z_c ) + get_color( z_d );
    return color / 4.0;
}

void main()
{
    {{#MULTISAMPLING_NONE}}
        vec3 pixel_color = get_color( gl_TexCoord[0].st );
    {{/MULTISAMPLING_NONE}}

    {{#MULTISAMPLING_4X}}
        vec3 pixel_color = get_color_multisample_4x( gl_TexCoord[0].st );
    {{/MULTISAMPLING_4X}}

    {{#MULTISAMPLING_8X}}
        float eighth_pixel = pixel_width / 8.0;
        vec2
            z_a = gl_TexCoord[0].st,
            z_b = vec2( z_a.x + eighth_pixel, z_a.y + eighth_pixel ); 
        vec3 pixel_color = get_color_multisample_4x( z_a ) + get_color_multisample_4x( z_b );
        pixel_color /= 2.0;
    {{/MULTISAMPLING_8X}}

    gl_FragColor = vec4( pixel_color, 1.0 );
}
