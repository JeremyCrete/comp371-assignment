#include "ParticleSystem.h"

void ParticleSystem::createQuad() {


    float vertices[] = {
                -0.5f, -0.5f, 0.0f,
                 0.5f, -0.5f, 0.0f,
                 0.5f,  0.5f, 0.0f,
                -0.5f,  0.5f, 0.0f
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void ParticleSystem::render(glm::mat4 view, glm::mat4 projection) {
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniform2fv(glGetUniformLocation(shaderProgram, "size"), 1, &PARTICLE_SIZE[0]);

    glBindVertexArray(VAO);
    for (const auto& p : particles) {
        if (p.life > 0.0f) {
            glUniform3fv(glGetUniformLocation(shaderProgram, "particlePos"), 1, &p.position[0]);
            glUniform4fv(glGetUniformLocation(shaderProgram, "particleColor"), 1, &p.color[0]);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }
    }
    glBindVertexArray(0);
}

void ParticleSystem::emitParticles(float dt, glm::vec3 cameraPos)
{
    timeSinceLastEmission += dt; 
    int particlesToEmit = timeSinceLastEmission * emissionRate;
    timeSinceLastEmission -= particlesToEmit / emissionRate;

    for (int i = 0; i < particlesToEmit; i++)
    {
        int idx = findUnusedParticle();
        if (idx == -1) return;

        particles[idx].life = PARTICLE_LIFETIME;
        particles[idx].position = EMITTER_POSITION;
        particles[idx].velocity = glm::vec3(
            distribution(generator) * 0.5f,
            abs(distribution(generator)) * 2.0f,
            distribution(generator) * 0.5f
        );

        particles[idx].color = glm::vec4(
            0.5f, 0.5f, 0.5f, 1.0f
        );
        
    }
}

void ParticleSystem::update(float dt)
{
    for(auto& p : particles)
    {
        if (p.life > 0.0f) {
            p.life -= dt;
            if (p.life > 0.0f)
            {
                p.velocity += PARTICLE_GRAVITY * dt;
                p.position += p.velocity * dt;
                p.color.a = p.life / PARTICLE_LIFETIME;
            }
        }
    }
}

int ParticleSystem::findUnusedParticle() {
    for (size_t i = 0; i < particles.size(); i++) 
    {
        if (particles[i].life <= 0.0f) return i;
    }
    return -1;
}

void ParticleSystem::compileAndLinkShaders(const char* vertexSource, const char* fragmentSource)
{
    // compile and link shader program
    // return shader program id
    // ------------------------------------

    // vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
            << infoLog << std::endl;
    }

    // fragment shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
            << infoLog << std::endl;
    }

    // link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
            << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


}