#version 330 core
in VS_OUT {
    vec2 uv;
    vec4 fragPosLightSpace;
} fs_in;

out vec4 FragColor;

uniform sampler2D diffuseTexture; 
uniform sampler2D shadowMap;

float ShadowFactor(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;


    if (projCoords.z > 1.0) return 0.0;

    float bias = 0.003; 
    float closest = texture(shadowMap, projCoords.xy).r;
    float current  = projCoords.z - bias;

    return current > closest ? 1.0 : 0.0;
}

void main(){
    vec3 base = texture(diffuseTexture, fs_in.uv).rgb;
    float shadow = ShadowFactor(fs_in.fragPosLightSpace);
    vec3 lit = mix(base, base * 0.45, shadow);
    FragColor = vec4(lit, 1.0);
}
