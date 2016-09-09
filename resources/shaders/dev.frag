uniform sampler2D texture_0;
in vec2 vertex_uv;
out vec4 color;


void main()
{
    color = texture(texture_0, vertex_uv);
}
