in vec4 fragColor;
out vec4 color;
uniform vec4 uniformColor;


void main() {
    color = mix(fragColor, uniformColor, 0.5);
}
