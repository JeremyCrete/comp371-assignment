#version 330 core
in VS_OUT {
    vec2 uv;
    vec4 fragPosLightSpace0;
    vec4 fragPosLightSpace1;
} fs_in;

struct Light {
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// Now we’ll use 2 lights
#define NR_POINT_LIGHTS 2

in vec3 normal;
in vec3 fragPos;

out vec4 FragColor;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap0;
uniform sampler2D shadowMap1;
uniform bool uUseShadow1;
uniform Light pointLights[NR_POINT_LIGHTS];

float ShadowFactor(sampler2D sm, vec4 lightSpacePos) {
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0) return 0.0;

    float bias = 0.003;
    float closest = texture(sm, projCoords.xy).r;
    float current = projCoords.z - bias;
    return current > closest ? 1.0 : 0.0;
}

void main(){
    int tileSize = 5;
    vec3 base = texture(diffuseTexture, fs_in.uv * tileSize).rgb;
    vec3 N = normalize(normal);

    // Light 0
    vec3 L0 = normalize(pointLights[0].position - fragPos);
    float diff0 = max(dot(N, L0), 0.0);
    float sh0 = ShadowFactor(shadowMap0, fs_in.fragPosLightSpace0);

    // Light 1 
    vec3 L1 = normalize(pointLights[1].position - fragPos);
    float diff1 = max(dot(N, L1), 0.0);
    float sh1 = uUseShadow1 ? ShadowFactor(shadowMap1, fs_in.fragPosLightSpace1) : 0.0;

    
    vec3 ambient = base * 0.15;
    vec3 color = ambient
               + (1.0 - sh0) * (diff0 * base)
               + (1.0 - sh1) * (diff1 * base);

    FragColor = vec4(color, 1.0);
}
