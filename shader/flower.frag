#version 330 core
in vec2 TexCoord;
in float FaceType;
in vec3 WorldPos;
out vec4 FragColor;
uniform vec3 viewPos;
uniform sampler2D flower;
uniform sampler2D grass;

void main()
{
    vec4 texturecolor;
    if(FaceType==0.0){
        vec4 color=texture(grass,TexCoord);
        if(color.a<0.2f){
            discard;
        }
        texturecolor=color;
    }else{
        vec4 color=texture(flower,TexCoord);
        if(color.a<0.2f){
            discard;
        }
        texturecolor=color;
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
