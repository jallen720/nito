// Specify a color output to the fragment shader
out vec4 fragColor;


void main() {
    gl_Position = vec4(vertexPosition, 1.0);
    fragColor = vec4(vertexPosition, 1.0);
}
