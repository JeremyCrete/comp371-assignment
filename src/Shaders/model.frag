#version 330 core
in VS_OUT {
    vec2 uv;
    vec3 normal;
    vec3 worldPos;
    
} fs_in;

struct Material {
    sampler2D diffuse;
    vec3 ambient;
    vec3 specular;
    float shininess;
};

out vec4 FragColor;

uniform Material material;
uniform int   textureMode;     // 1=onyx, 2=gold, 3=white wrap (RGB)
uniform float uTime;
uniform vec3  viewPos;

//2 light positions and 2 shadow maps
uniform vec3 lightPosWorld0;
uniform vec3 lightPosWorld1;

uniform sampler2D shadowMap0;
uniform sampler2D shadowMap1;

uniform mat4 lightSpaceMatrix0;
uniform mat4 lightSpaceMatrix1;

uniform int uUseShadow1; //light toggle 

// HSV -> RGB
vec3 hsv2rgb(vec3 c) {
    vec3 p = abs(fract(c.xxx + vec3(0., 2./3., 1./3.)) * 6. - 3.);
    vec3 rgb = clamp(p - 1., 0., 1.);
    return c.z * mix(vec3(1.0), rgb, c.y);
}

//creates the rainbow effect 
vec3 rainbowColorConstant(float t) {
    float h = fract(t * 0.08); // change speed if you want
    return hsv2rgb(vec3(h, 1.0, 1.0));
}

//shadow map0 
float ShadowFactor0(vec3 normal, vec3 lightDir) {
    vec4 fragLS = lightSpaceMatrix0 * vec4(fs_in.worldPos, 1.0);
    vec3 proj = fragLS.xyz / fragLS.w;
    proj = proj * 0.5 + 0.5;
    if (proj.z > 1.0) return 0.0;

    float bias = max(0.003 * (1.0 - dot(normalize(normal), normalize(lightDir))), 0.001);
    float closest = texture(shadowMap0, proj.xy).r;
    float current = proj.z - bias;
    return current > closest ? 1.0 : 0.0;
}

// Shadow map1
float ShadowFactor1(vec3 normal, vec3 lightDir) {
    vec4 fragLS = lightSpaceMatrix1 * vec4(fs_in.worldPos, 1.0);
    vec3 proj = fragLS.xyz / fragLS.w;
    proj = proj * 0.5 + 0.5;
    if (proj.z > 1.0) return 0.0;

    float bias = max(0.003 * (1.0 - dot(normalize(normal), normalize(lightDir))), 0.001);
    float closest = texture(shadowMap1, proj.xy).r;
    float current = proj.z - bias;
    return current > closest ? 1.0 : 0.0;
}

void main()
{
    // Base albedo
    vec3 base = texture(material.diffuse, fs_in.uv).rgb;

    
    if (textureMode == 3) {
        float lum  = dot(base, vec3(0.299, 0.587, 0.114));
        float mask = smoothstep(0.80, 0.95, lum);
        vec3  rgb  = rainbowColorConstant(uTime);
        base = mix(base, rgb, mask);
    }

    vec3 N = normalize(fs_in.normal);
    vec3 V = normalize(viewPos - fs_in.worldPos);

    //light0
    vec3 L0 = normalize(lightPosWorld0 - fs_in.worldPos);
    float diff0 = max(dot(N, L0), 0.0);
    float spec0 = 0.0;
    if (diff0 > 0.0) {
        vec3 R0 = reflect(-L0, N);
        spec0 = pow(max(dot(R0, V), 0.0), material.shininess);
    }
    float sh0 = ShadowFactor0(N, L0);

    //light1
    vec3 L1 = normalize(lightPosWorld1 - fs_in.worldPos);
    float diff1 = max(dot(N, L1), 0.0);
    float spec1 = 0.0;
    if (diff1 > 0.0) {
        vec3 R1 = reflect(-L1, N);
        spec1 = pow(max(dot(R1, V), 0.0), material.shininess);
    }
    float sh1 = (uUseShadow1 == 1) ? ShadowFactor1(N, L1) : 0.0;

    vec3 ambient  = material.ambient * base;
    vec3 lambert0 = diff0 * base;
    vec3 lambert1 = diff1 * base;
    vec3 specCol0 = material.specular * spec0;
    vec3 specCol1 = material.specular * spec1;

    vec3 color = ambient
               + (1.0 - sh0) * (lambert0 + specCol0)
               + (uUseShadow1 == 1 ? (1.0 - sh1) * (lambert1 + specCol1) : vec3(0.0));

    FragColor = vec4(color, 1.0);
}
