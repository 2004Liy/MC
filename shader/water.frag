#version 330 core
out vec4 FragColor;
in vec3 TexCoords;
uniform sampler2D cubeMap;
void main() {
    FragColor = texture(cubeMap, TexCoords);
}
