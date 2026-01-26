#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec3 aInstancePos;
layout(location = 4) in float aFaceType;
out vec2 TexCoord;
out float FaceType;
out vec3 Normal;
out vec3 WorldPos;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    WorldPos=vec3(aPos+aInstancePos);
    gl_Position = projection*view*vec4(aPos+aInstancePos, 1.0);
    TexCoord = aTexCoord;
    FaceType=aFaceType;
    Normal=aNormal;
}
