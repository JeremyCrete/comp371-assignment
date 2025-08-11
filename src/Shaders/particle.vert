#version 330 core
    layout (location = 0) in vec3 aPos;
    
    uniform mat4 projection;
    uniform mat4 view;
    uniform vec3 particlePos;
    uniform vec2 size;
    
    void main() {
        vec3 position = particlePos + vec3(aPos.x * size.x, aPos.y * size.y, 0.0);
        gl_Position = projection * view * vec4(position, 1.0);
    }