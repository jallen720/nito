uniform sampler2D texture_0;
uniform vec3 text_color;
in vec2 vertex_uv;
out vec4 color;


void main()
{
    color = vec4(text_color, texture(texture_0, vertex_uv).r);
}
