#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aInstancePos;
out vec3 TexCoords;
uniform mat4 projection;
uniform mat4 view;
void main() {
    TexCoords = aTexCoord;
    vec4 pos = projection * view * vec4(aPos+aInstancePos, 1.0);
    gl_Position = pos.xyzw;
}
