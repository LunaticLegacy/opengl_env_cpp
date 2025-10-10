#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "shader.hpp"

// 粒子结构体
struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec4 color;
    float life;
    float rotation;
    float rotationSpeed;
    float size;
};

class ParticleSystem {
public:
    ParticleSystem(unsigned int maxParticles = 1000);
    ~ParticleSystem();

    void update(float deltaTime);
    void draw(Shader& shader);
    void emitParticle();
    void setEmissionRate(float rate) { emissionRate = rate; }
    void setParticleLifetime(float lifetime) { particleLifetime = lifetime; }
    void setEmitterPosition(const glm::vec3& pos) { emitterPosition = pos; }
    void setEmitterVelocity(const glm::vec3& vel) { emitterVelocity = vel; }

private:
    unsigned int maxParticles;
    unsigned int activeParticles;
    std::vector<Particle> particles;
    unsigned int VAO, VBO;
    
    float emissionRate;
    float emissionCounter;
    float particleLifetime;
    glm::vec3 emitterPosition;
    glm::vec3 emitterVelocity;
    
    void initRenderData();
    void respawnParticle(Particle& particle);
};