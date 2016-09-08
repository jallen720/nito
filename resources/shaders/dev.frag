uniform sampler2D texture0;
uniform float textureMixValue;
in vec2 vertexUV;
out vec4 color;


void main() {
    color = texture(texture0, vertexUV);
}
