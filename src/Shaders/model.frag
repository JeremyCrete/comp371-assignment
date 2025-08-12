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

uniform sampler2D shadowMap;      
uniform vec3 lightPosWorld;
uniform Material material;
uniform int   textureMode;   // 1=onyx, 2=gold, 3=white wrap (RGB)
uniform float uTime;  
uniform vec3  viewPos;       //used in phong 

float ShadowFactor(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 proj = fragPosLightSpace.xyz / fragPosLightSpace.w;
    proj = proj * 0.5 + 0.5;       // NDC -> [0,1]
    if (proj.z > 1.0) return 0.0;  // outside light frustum

    float bias = max(0.003 * (1.0 - dot(normalize(normal), normalize(lightDir))), 0.001);

    float closest = texture(shadowMap, proj.xy).r;
    float current = proj.z - bias;
    return current > closest ? 1.0 : 0.0; // 1=in shadow
}

// HSV -> RGB
vec3 hsv2rgb(vec3 c) {
    vec3 p = abs(fract(c.xxx + vec3(0., 2./3., 1./3.)) * 6. - 3.);
    vec3 rgb = clamp(p - 1., 0., 1.);
    return c.z * mix(vec3(1.0), rgb, c.y);
}


vec3 rainbowColor(float t) {
    float h = fract(t * 0.08);   // adjust 0.08 for speed
    return hsv2rgb(vec3(h, 1.0, 1.0));
}

void main()
{
    // Albedo
    vec3 base = texture(material.diffuse, fs_in.uv).rgb;


    if (textureMode == 3) {
        float lum  = dot(base, vec3(0.299, 0.587, 0.114));
        float mask = smoothstep(0.80, 0.95, lum);
        vec3  rgb  = rainbowColor(uTime);
        base = mix(base, rgb, mask);
    }

    // Phong lighting
    vec3 N = normalize(fs_in.normal);
    vec3 L = normalize(lightPosWorld - fs_in.worldPos);
    vec3 V = normalize(viewPos       - fs_in.worldPos);
    vec3 R = reflect(-L, N);

    float diff = max(dot(N, L), 0.0);

    float spec = 0.0;
    if (diff > 0.0) {
        spec = pow(max(dot(R, V), 0.0), material.shininess);
    }

    float shadow = ShadowFactor(fs_in.fragPosLightSpace, N, L);

    vec3 ambient  = material.ambient * base;
    vec3 diffuse  = diff * base;
    vec3 specular = material.specular * spec;

    // Shadows dim diffuse+specular (ambient stays)
    vec3 color = ambient + (1.0 - shadow) * (diffuse + specular);

    FragColor = vec4(color, 1.0);
}
