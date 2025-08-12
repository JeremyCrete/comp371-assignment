#version 330 core
in VS_OUT {
    vec2 uv;
    vec4 fragPosLightSpace;
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

#define NR_POINT_LIGHTS 1

in vec3 normal;
in vec3 fragPos;

out vec4 FragColor;

uniform sampler2D diffuseTexture; 
uniform sampler2D shadowMap;
uniform Light pointLights[NR_POINT_LIGHTS];

// calculates the color when using a point light.
vec3 CalcPointLight(Light light, vec3 normal, vec3 fragPos, vec4 textureColor)
{
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * textureColor.rgb;
    // combine results
    vec3 result = textureColor.rgb + diffuse;
    return result;
};

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
    int tileSize = 5;
    vec4 textureColor = texture(diffuseTexture, fs_in.uv * tileSize);
    vec3 result = vec3(0.0);

    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], normal, fragPos, textureColor); 

    float shadow = ShadowFactor(fs_in.fragPosLightSpace);

    vec3 lit = mix(result * 0.75, result * 0.15, shadow);
    FragColor = vec4(lit, 1.0);
}
