#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 lightSpaceMatrix;   

out VS_OUT {
    vec2 uv;
    vec3 normal;
    vec3 worldPos;
    vec4 fragPosLightSpace;
} vs_out;

void main()
{
    vec4 wp = worldMatrix * vec4(aPos, 1.0);
    vs_out.worldPos = wp.xyz;

    // transform normal to world space
    mat3 nmat = mat3(transpose(inverse(worldMatrix)));
    vs_out.normal = normalize(nmat * aNormal);

    vs_out.uv = aTexCoords;
    vs_out.fragPosLightSpace = lightSpaceMatrix * wp;

    gl_Position = projectionMatrix * viewMatrix * wp;
}
