#version 330 core

in vec2 TexCoords;

uniform sampler2D modelTexture;
uniform vec3 modelColor;

out vec4 FragColor;

void main()
{
    vec4 texColor = texture(modelTexture, TexCoords);
    FragColor = texColor;
}
