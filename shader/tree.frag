#version 330 core
in vec2 TexCoord;
in float FaceType;
in vec3 Normal;
in vec3 WorldPos;
out vec4 FragColor;
uniform vec3 viewPos;
uniform sampler2D treeinterior;;
uniform sampler2D treesurface;
uniform sampler2D leaf;

void main()
{
    vec4 texturecolor;
    if(FaceType==1.0){
        vec4 acolor=texture(leaf,TexCoord);
        if(acolor.a<0.1f){
            discard;
        }
        texturecolor=acolor;
    }else{
        if(Normal==vec3(0.0,1.0,0.0)||Normal==vec3(0.0,-1.0,0.0)){
            texturecolor=texture(treeinterior,TexCoord);
        }else{
            texturecolor=texture(treesurface,TexCoord);
        }
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
