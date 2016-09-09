uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
out vec2 vertex_uv;


vec4 vertex_position()
{
    return projection * view * model * vec4(position, 1);
}


void main()
{
    gl_Position = vertex_position();
    vertex_uv = uv;
}
