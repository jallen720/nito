uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
out vec2 vertexUV;


vec4 vertexPosition() {
    return projection * view * model * vec4(position, 1);
}


void main() {
    gl_Position = vertexPosition();
    vertexUV = uv;
}
