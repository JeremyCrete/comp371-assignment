#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aUV;
layout (location = 3) in vec3 aNormal;

out VS_OUT {
    vec2 uv;
    vec4 fragPosLightSpace0;
    vec4 fragPosLightSpace1;
} fs_out;

out vec3 normal;
out vec3 fragPos;

uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 lightSpaceMatrix0;
uniform mat4 lightSpaceMatrix1;

void main(){
    vec4 worldPos = worldMatrix * vec4(aPos, 1.0);
    fragPos = worldPos.xyz;
    normal = mat3(transpose(inverse(worldMatrix))) * aNormal;

    fs_out.uv = aUV;
    fs_out.fragPosLightSpace0 = lightSpaceMatrix0 * worldPos;
    fs_out.fragPosLightSpace1 = lightSpaceMatrix1 * worldPos;

    gl_Position = projectionMatrix * viewMatrix * worldPos;
}
