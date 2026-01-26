#version 330 core
in vec2 TexCoord;
in float FaceType;
in vec3 WorldPos;
out vec4 FragColor;
uniform vec3 viewPos;
uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;
uniform sampler2D tex4;
uniform sampler2D tex5;
uniform sampler2D tex6;
uniform sampler2D tex7;
uniform sampler2D tex8;

void main()
{
    vec4 texturecolor;
    if(FaceType==0){
        texturecolor=texture(tex0,TexCoord);
    }else if(FaceType==1){
        texturecolor=texture(tex1,TexCoord);
    }else if(FaceType==2){
        texturecolor=texture(tex2,TexCoord);
    }else if(FaceType==3){
        texturecolor=texture(tex3,TexCoord);
    }else if(FaceType==4){
        texturecolor=texture(tex4,TexCoord);
    }else if(FaceType==5){
        texturecolor=texture(tex5,TexCoord);
    }else if(FaceType==6){
        texturecolor=texture(tex6,TexCoord);
    }else if(FaceType==7){
        texturecolor=texture(tex7,TexCoord);
    }else if(FaceType==8){
        texturecolor=texture(tex8,TexCoord);
    }
    if(texturecolor.a<0.1){
        discard;
    }
    float distance = length(viewPos - WorldPos);
    if (WorldPos.y<28) {
        float fogDensity = 0.1;
        float fogFactor = exp(-fogDensity * distance);
        fogFactor = clamp(fogFactor, 0.0, 1.0);
        vec3 fogColor = vec3(0.0, 0.1, 0.2);
        vec3 finalColor = mix(fogColor, texturecolor.rgb, fogFactor);
        FragColor = vec4(finalColor, texturecolor.a);
    }else{
        float fogDensity = 0.025;
        float fogFactor = exp(-fogDensity * distance);
        fogFactor = clamp(fogFactor, 0.0, 1.0);
        vec3 fogColor = vec3(0.7, 0.7, 0.8);
        vec3 finalColor = mix(fogColor, texturecolor.rgb, fogFactor);
        FragColor = vec4(finalColor, texturecolor.a);
    }
}
