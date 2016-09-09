// The input variable from the vertex shader (same name and same type).
in vec4 frag_color;


// The output color for this fragment.
out vec4 color;


void main()
{
    color = frag_color;
}
