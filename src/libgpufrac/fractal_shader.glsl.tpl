uniform vec2 seed;
uniform sampler2D palette;
uniform sampler2D orbit_trap;
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

// TODO: Desperately must come up with a better name than 'info'...
struct escape_info
{
    float magnitude;
    vec3 color;
};

vec3 palette_lookup( float escape_magnitude )
{
    escape_magnitude *= palette_stretch;
    escape_magnitude += palette_offset; // TODO: Stretch offset?

    // TODO: The palette cycling speed varies pretty drastically between the various palette modes.
    //       Figure out some way to make the apparent speed more uniform.

    {{#PALETTE_MODE_TEXTURE}}
        return texture2D( palette, vec2( escape_magnitude, 0.0 ) ).rgb;
    {{/PALETTE_MODE_TEXTURE}}

    {{#PALETTE_MODE_TRIG}}
        return vec3( red_amplitude * sin( red_frequency * escape_magnitude + red_phase ),
                     green_amplitude * sin( green_frequency * escape_magnitude + green_phase ),
                     blue_amplitude * sin( blue_frequency * escape_magnitude + blue_phase ) );
    {{/PALETTE_MODE_TRIG}}

    {{#PALETTE_MODE_ORBIT_TRAP}}
        return vec3( escape_magnitude, escape_magnitude, escape_magnitude );
    {{/PALETTE_MODE_ORBIT_TRAP}}
}

escape_info calculate_escape_info( vec2 z )
{
    escape_info info;

    info.magnitude = 0.0;
    info.color = vec3( 0.0, 0.0, 0.0 );

    float 
        escape_magnitude = 0.0,
        half_julia_exponent = julia_exponent / 2.0;

    int iteration = 0;

    float radius_squared = 0.0;

    {{#MANDELBROT_MODE_ENABLED}}
        vec2 use_seed = z; // TODO: Better name?
    {{/MANDELBROT_MODE_ENABLED}}

    {{#MANDELBROT_MODE_DISABLED}}
        vec2 use_seed = seed;
    {{/MANDELBROT_MODE_DISABLED}}

    for ( ; iteration < max_iterations; ++iteration )
    {
        float
            z_x_squared = z.x * z.x,
            z_y_squared = z.y * z.y;

        radius_squared = z_x_squared + z_y_squared;

        {{#COLORING_MODE_CONTINUOUS}}
            info.magnitude += exp( -sqrt( radius_squared ) ); // e ^ ( -radius )
        {{/COLORING_MODE_CONTINUOUS}}

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

            z = vec2( new_radius * cos( new_theta ), new_radius * sin( new_theta ) ) + use_seed;
        {{/ITERATOR_POLAR}}

        {{#ITERATOR_CARTESIAN}}
            z = vec2( z_x_squared - z_y_squared, 2.0 * z.x * z.y ) + use_seed;
        {{/ITERATOR_CARTESIAN}}

        {{#PALETTE_MODE_ORBIT_TRAP}}
            // FIXME: Ugly hack, just for now.
            vec4 trap_color = texture2D( orbit_trap, z );
            if ( trap_color.a > 0.01 )
            {
                // Currently the alpha channel of the orbit trap is used as the magnitude.  This is for no good reason, except for a
                // general lack of anything else to assign to it.  TODO: Come up with a reasonable meaning for 'magnitude' in orbit trap mode.
                info.magnitude = trap_color.a;
                info.color = trap_color.rgb * trap_color.a;
                return info;
            }
        {{/PALETTE_MODE_ORBIT_TRAP}}
    }

    {{#PALETTE_MODE_ORBIT_TRAP}}
        // FIXME: Ugly hack, just for now.
        info.magnitude = 0.0;
        return info;
    {{/PALETTE_MODE_ORBIT_TRAP}}

    {{#COLORING_MODE_CONTINUOUS}}
        // TODO remove duplicated info.color = ... etc.
        info.color = palette_lookup( info.magnitude );
    {{/COLORING_MODE_CONTINUOUS}}

    {{#COLORING_MODE_ITERATIVE}}
        info.magnitude = float( iteration );
        info.color = palette_lookup( info.magnitude );
    {{/COLORING_MODE_ITERATIVE}}

    {{#COLORING_MODE_RADIUS_SQUARED}}
        info.magnitude = radius_squared;
        info.color = palette_lookup( info.magnitude );
    {{/COLORING_MODE_RADIUS_SQUARED}}

    {{#COLORING_MODE_ANGLE}}
        info.magnitude = atan( z.y, z.x );
        info.color = palette_lookup( info.magnitude );
    {{/COLORING_MODE_ANGLE}}

    return info;
}

{{#NORMAL_MAPPING_DISABLED}}
vec3 get_color( vec2 z )
{
    return calculate_escape_info( z ).color;
}
{{/NORMAL_MAPPING_DISABLED}}

{{#NORMAL_MAPPING_ENABLED}}
vec3 get_color( vec2 z_a )
{
    float offset = pixel_width / 2.0;

    vec2
        z_b = vec2( z_a.x + offset, z_a.y ),
        z_c = vec2( z_a.x, z_a.y + offset );

    escape_info
        escape_info_a = calculate_escape_info( z_a ),
        escape_info_b = calculate_escape_info( z_b ),
        escape_info_c = calculate_escape_info( z_c );
    
    vec3 base_color = escape_info_a.color + escape_info_a.color + escape_info_a.color;
    base_color /= 3.0;
    
    vec3
        point_a = vec3( z_a.x, z_a.y, escape_info_a.magnitude * height_scale ),
        point_b = vec3( z_b.x, z_b.y, escape_info_b.magnitude * height_scale ),
        point_c = vec3( z_c.x, z_c.y, escape_info_c.magnitude * height_scale );
    
    vec3 normal = normalize( cross( point_b - point_a, point_c - point_a ) );

    // Multiply the color by the cosine of the angle between the face's normal and the light vector.
    // TODO Make the light vector configurable.
    return base_color * dot( normal, vec3( 0.0, 0.0, 1.00 ) );
    // return base_color * dot( normal, normalize( vec3( 0.0, 0.20, 0.80 ) ) );
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
