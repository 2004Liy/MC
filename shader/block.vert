#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aInstancePos;
layout(location = 3) in float aFaceType;
layout(location = 4) in float aBlockType;
out vec2 TexCoord;
out float FaceType;
out float BlockType;
out vec3 WorldPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

mat4 getRotationMatrix(int faceType) {
    if (faceType==0) return mat4(1.0);
    else if (faceType==1){
        return mat4(-1.0, 0.0, 0.0, 0.0,
                    0.0, 1.0, 0.0, 0.0,
                    0.0, 0.0, -1.0, 0.0,
                    0.0, 0.0, 0.0, 1.0);
    }
    else if (faceType==2){
        return mat4(0.0, 0.0, 1.0, 0.0,
                    0.0, 1.0, 0.0, 0.0,
                    -1.0, 0.0, 0.0, 0.0,
                    0.0, 0.0, 0.0, 1.0);
    }
    else if (faceType==3){
        return mat4(0.0, 0.0, -1.0, 0.0,
                    0.0, 1.0, 0.0, 0.0,
                    1.0, 0.0, 0.0, 0.0,
                    0.0, 0.0, 0.0, 1.0);
    }
    else if (faceType==4||faceType==6) {// 上面：绕X轴旋转-90度
        return mat4(1.0, 0.0, 0.0, 0.0,
                    0.0, 0.0, -1.0, 0.0,
                    0.0, 1.0, 0.0, 0.0,
                    0.0, 0.0, 0.0, 1.0);

    }
    else if(faceType==5){  // 下面：绕X轴旋转90度
        return mat4(1.0, 0.0, 0.0, 0.0,
                    0.0, 0.0, 1.0, 0.0,
                    0.0, -1.0, 0.0, 0.0,
                    0.0, 0.0, 0.0, 1.0);
    }
}

void main()
{
    BlockType=aBlockType;
    mat4 rotation=getRotationMatrix(int(aFaceType));
    vec4 rotatedPos=rotation*vec4(aPos, 1.0);
    vec3 worldPos=rotatedPos.xyz+aInstancePos;
    WorldPos=worldPos;
    gl_Position=projection*view*vec4(worldPos, 1.0);
    TexCoord=aTexCoord;
    FaceType=aFaceType;
}
