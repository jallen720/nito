uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
out vec2 vertex_uv;
out vec3 fragment_position;


void main()
{
    gl_Position = projection * view * model * vec4(position, 1);
    vertex_uv = vec2(uv.x, 1 - uv.y);
    fragment_position = vec3(model * vec4(position, 1));
}
