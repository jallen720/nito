#version 330 core


// The position attribute is the first attribute in a vertex (index 0).
layout (location = 0) in vec3 position;


// Specify a color output to the fragment shader
out vec4 vertexColor;


void main() {
    gl_Position = vec4(position, 1.0);
    vertexColor = vec4(position, 1.0);
}
