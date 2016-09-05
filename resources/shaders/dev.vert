out vec2 vertexUV;


void main() {
    gl_Position = vec4(position, 1.0);
    vertexUV = uv;
}
