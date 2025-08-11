#version 330 core
in VS_OUT {
    vec2 uv;
    vec3 normal;
    vec3 worldPos;
    vec4 fragPosLightSpace;
} fs_in;

struct Material {
    sampler2D diffuse;
    vec3 ambient;
    vec3 specular;
    float shininess;
};


out vec4 FragColor;

uniform sampler2D modelTexture;
uniform sampler2D shadowMap;      
uniform vec3 lightPosWorld;
uniform Material material;


float ShadowFactor(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 proj = fragPosLightSpace.xyz / fragPosLightSpace.w;
    proj = proj * 0.5 + 0.5;       // NDC -> [0,1]

    if (proj.z > 1.0) return 0.0;  // outside light frustum

    // bias scaled by angle to reduce acne
    float bias = max(0.003 * (1.0 - dot(normalize(normal), normalize(lightDir))), 0.001);

    float closest = texture(shadowMap, proj.xy).r;
    float current = proj.z - bias;

    return current > closest ? 1.0 : 0.0; // 1=in shadow
}

void main()
{
    vec3 base = texture(material.diffuse, fs_in.uv).rgb;

    vec3 L = lightPosWorld - fs_in.worldPos;
    float shadow = ShadowFactor(fs_in.fragPosLightSpace, fs_in.normal, L);


    vec3 color = mix(base, base * 0.45, shadow);
    FragColor = vec4(color, 1.0);
}
