uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
out vec2 vertex_uv;


void main()
{
    gl_Position = projection * view * model * vec4(position, 1);
    vertex_uv = uv;
}
