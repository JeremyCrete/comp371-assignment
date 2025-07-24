#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;
out vec3 vertexColor;
out vec2 texCoord;
uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
void main()
{
    vertexColor = aColor;
    texCoord = aTexCoord;
    gl_Position = projectionMatrix * viewMatrix * worldMatrix * vec4(aPos, 1.0);
}
