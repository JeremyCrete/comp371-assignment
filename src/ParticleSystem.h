#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <random>
#include <iostream>
 
struct Particle {
    glm:: vec3 position;
    glm::vec3 velocity;
    glm::vec4 color;
    float life;
};

// Particle system parameters
const int MAX_PARTICLES = 1000;
const float PARTICLE_LIFETIME = 3.0f;
const glm::vec3 EMITTER_POSITION(0.0f, 0.0f, 0.0f);
const glm::vec3 PARTICLE_GRAVITY(0.0f, -0.5f, 0.0f);
const glm::vec2 PARTICLE_SIZE(1.0f, 1.0f);


class ParticleSystem {
private:
    std::vector<Particle> particles;
    std::default_random_engine generator;
    std::uniform_real_distribution<float> distribution;
    float emissionRate = 100.0f;
    float timeSinceLastEmission = 0.0f;
    GLuint VBO;

public:
    GLuint VAO;
    int shaderProgram;

    ParticleSystem() : distribution(-1.0f, 1.0f) {
        particles.resize(MAX_PARTICLES);
    }

    void emitParticles(float dt, glm::vec3 emitterPos);

    void update(float dt);

    void render(glm::mat4 view, glm::mat4 projection);

    void createQuad();

    void compileAndLinkShaders(const char* vertexSource, const char* fragmentSource);

private:
    int findUnusedParticle();
};

