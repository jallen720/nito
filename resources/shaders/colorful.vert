// Specify a color output to the fragment shader
out vec4 frag_color;


void main()
{
    gl_Position = vec4(position, 1.0);
    frag_color = vec4(position, 1.0);
}
