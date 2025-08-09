#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aUV;
layout (location = 3) in vec3 aNormal;

out VS_OUT {
    vec2 uv;
    vec4 fragPosLightSpace;
} vs_out;

uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 lightSpaceMatrix;

void main(){
    vec4 worldPos = worldMatrix * vec4(aPos, 1.0);
    vs_out.uv = aUV;
    vs_out.fragPosLightSpace = lightSpaceMatrix * worldPos;
    gl_Position = projectionMatrix * viewMatrix * worldPos;
}
