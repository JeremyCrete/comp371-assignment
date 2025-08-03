#version 150 core

in vec3 position;
in vec3 normal;
in vec2 texCoord;

uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
    gl_Position = projectionMatrix * viewMatrix * worldMatrix * vec4(position, 1.0);
}
