uniform sampler2D texture0;
uniform sampler2D texture1;
uniform float textureMixValue;
in vec2 vertexUV;
out vec4 color;


vec4 mixedTextures() {
    return mix(
        texture(texture0, vertexUV),
        texture(texture1, vertexUV),
        textureMixValue);
}


void main() {
    color = mixedTextures();
}
