uniform sampler2D texture_0;
uniform int light_source_count;
uniform float light_source_intensities[64];
uniform float light_source_ranges[64];
uniform vec3 light_source_colors[64];
uniform vec3 light_source_positions[64];
uniform int light_source_enabled_flags[64];
in vec2 vertex_uv;
in vec3 fragment_position;
out vec4 color;
const vec3 ambient_light = vec3(0.6, 0.6, 0.6);
const int PIXELS_PER_UNIT = 64;


vec3 get_light_color()
{
    vec3 light_color = vec3(0);

    if (light_source_count == 0)
    {
        return light_color;
    }

    for (int i = 0; i < light_source_count; i++)
    {
        if (light_source_enabled_flags[i] == 0)
        {
            continue;
        }

        float light_source_range = light_source_ranges[i] * PIXELS_PER_UNIT;

        light_color +=
            light_source_intensities[i] *
            light_source_colors[i] *
            clamp(
                light_source_range - distance(light_source_positions[i].xy * PIXELS_PER_UNIT, fragment_position.xy),
                0,
                light_source_range);
    }

    return light_color;
}


void main()
{
    color = texture(texture_0, vertex_uv) * vec4(ambient_light + get_light_color(), 1);
}
