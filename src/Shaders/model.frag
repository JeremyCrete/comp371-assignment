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
uniform int   textureMode;   // 1=onyx, 2=gold, 3=white wrap (RGB)
uniform float uTime;  


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


vec3 hsv2rgb(vec3 c){
    vec3 p = abs(fract(c.xxx + vec3(0., 2./3., 1./3.))*6. - 3.);
    return c.z * clamp(p - 1., 0., 1.);
}


vec3 rainbowGlobal(float t){
    float h = fract(t * 0.20);          // speed
    vec3  rgb = hsv2rgb(vec3(h, 1.0, 1.0));
    return clamp(rgb * 3, 0.0, 1.0); //birghtness for more vibrant colors
}

void main()
{
   vec3 base = texture(material.diffuse, fs_in.uv).rgb;


if (textureMode == 3) {
    float lum  = dot(base, vec3(0.299, 0.587, 0.114)); // luminance
    float mask = smoothstep(0.80, 0.95, lum);          
    vec3  rgb  = rainbowGlobal(uTime);
    base = mix(base, rgb, mask);   
}

    vec3 L = lightPosWorld - fs_in.worldPos;
    float shadow = ShadowFactor(fs_in.fragPosLightSpace, fs_in.normal, L);


    vec3 color = mix(base, base * 0.45, shadow);
    FragColor = vec4(color, 1.0);
}
