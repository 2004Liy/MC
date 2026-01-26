#version 330 core
in vec2 TexCoord;
in float FaceType;
in float BlockType;
in vec3 WorldPos;
out vec4 FragColor;
uniform vec3 viewPos;
uniform sampler2D texFront;
uniform sampler2D texBack;
uniform sampler2D texLeft;
uniform sampler2D texRight;
uniform sampler2D texTop;
uniform sampler2D texBottom;
uniform sampler2D water;
uniform sampler2D rock;
uniform sampler2D sand;
uniform sampler2D dirt;

uniform float fogHeightThreshold;  // 雾气高度阈值（28）
void main()
{
    vec4 texturecolor;
    int index = int(FaceType);
    if (index==0) {
        if(BlockType==1.0){
            texturecolor = texture(texFront, TexCoord);
        }else if(BlockType==0.0){
            texturecolor=texture(sand,TexCoord);
        }else if(BlockType==2.0){
            texturecolor=texture(rock,TexCoord);
        }else if(BlockType==3.0){
            texturecolor = texture(dirt, TexCoord);
        }
    } else if (index==1) {
        if(BlockType==1.0){
            texturecolor = texture(texBack, TexCoord);
        }else if(BlockType==0.0){
            texturecolor=texture(sand,TexCoord);
        }else if(BlockType==2.0){
            texturecolor=texture(rock,TexCoord);
        }else if(BlockType==3.0){
            texturecolor = texture(dirt, TexCoord);
        }
    } else if (index==2) {
        if(BlockType==1.0){
            texturecolor = texture(texLeft, TexCoord);
        }else if(BlockType==0.0){
            texturecolor=texture(sand,TexCoord);
        }else if(BlockType==2.0){
            texturecolor=texture(rock,TexCoord);
        }else if(BlockType==3.0){
            texturecolor = texture(dirt, TexCoord);
        }
    } else if (index==3) {
        if(BlockType==1.0){
            texturecolor = texture(texRight, TexCoord);
        }else if(BlockType==0.0){
            texturecolor=texture(sand,TexCoord);
        }else if(BlockType==2.0){
            texturecolor=texture(rock,TexCoord);
        }else if(BlockType==3.0){
            texturecolor = texture(dirt, TexCoord);
        }
    } else if (index==4) {
        if(BlockType==1.0){
            texturecolor = texture(texTop, TexCoord);
        }else if(BlockType==0.0){
            texturecolor=texture(sand,TexCoord);
        }else if(BlockType==2.0){
            texturecolor=texture(rock,TexCoord);
        }else if(BlockType==3.0){
            texturecolor = texture(dirt, TexCoord);
        }
    } else if (index==5) {
        if(BlockType==1.0){
            texturecolor = texture(texBottom, TexCoord);
        }else if(BlockType==0.0){
            texturecolor=texture(sand,TexCoord);
        }else if(BlockType==2.0){
            texturecolor=texture(rock,TexCoord);
        }else if(BlockType==3.0){
            texturecolor = texture(dirt, TexCoord);
        }
    } else if(index==6){
        texturecolor = texture(water, TexCoord);
    }

    float distance = length(viewPos - WorldPos);
    if (WorldPos.y<fogHeightThreshold) {
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
    //FragColor=texturecolor;
}
