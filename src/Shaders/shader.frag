#version 330 core
out vec4 FragColor;

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define NR_POINT_LIGHTS 4

in vec3 vertexColor;
in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;

uniform sampler2D texture1;
uniform vec3 lightPos;
uniform PointLight pointLights[NR_POINT_LIGHTS];

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec4 textureColor)
{
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    float ambientStrength = 0.01;
    vec3 diffuse = diff * textureColor.rgb;
    // combine results
    vec3 result = ambientStrength * textureColor.rgb + diffuse;
    return result;
}

void main()
{
    int tileSize = 5;

    vec4 textureColor = texture(texture1, texCoord * tileSize);

    vec3 result = vec3(0.0);

    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], normal, fragPos, textureColor); 

    FragColor = vec4(result, 1.0);
}


