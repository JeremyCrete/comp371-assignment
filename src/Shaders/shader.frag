#version 330 core
in vec3 vertexColor;
in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;
out vec4 FragColor;

uniform sampler2D texture1;
uniform vec3 lightPos;


void main()
{
    int tileSize = 50;
    float ambientStrength = 0.1;
    vec4 textureColor = texture(texture1, texCoord * tileSize);

    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * textureColor.rgb;

    vec3 result = ambientStrength * textureColor.rgb + diffuse;
    FragColor = vec4(result, 1.0);
}
