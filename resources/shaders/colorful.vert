// Specify a color output to the fragment shader
out vec4 vertexColor;


void main() {
    gl_Position = vec4(position, 1.0);
    vertexColor = vec4(position, 1.0);
}
