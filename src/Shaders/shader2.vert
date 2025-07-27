#version 330 core
layout (location = 0) in vec3 aPos;
out vec4 vertexColor;

uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
    vertexColor = vec4(1.0, 1.0, 1.0, 1.0); 
    gl_Position = projectionMatrix * viewMatrix * worldMatrix * vec4(aPos, 1.0);
}
