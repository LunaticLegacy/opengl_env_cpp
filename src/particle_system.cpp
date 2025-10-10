#include "particle_system.hpp"
#include <glm/gtc/random.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <iostream>

ParticleSystem::ParticleSystem(unsigned int maxParticles) 
    : maxParticles(maxParticles), 
      activeParticles(0), 
      particles(maxParticles),
      emissionRate(100.0f),
      emissionCounter(0.0f),
      particleLifetime(5.0f),
      emitterPosition(0.0f, 0.0f, 0.0f),
      emitterVelocity(0.0f, 2.0f, 0.0f) {
    initRenderData();
}

ParticleSystem::~ParticleSystem() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void ParticleSystem::initRenderData() {
    // 设置粒子顶点数据（一个正方形）
    float particleQuad[] = {
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particleQuad), particleQuad, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}

void ParticleSystem::update(float deltaTime) {
    // 添加新粒子
    emissionCounter += emissionRate * deltaTime;
    while (emissionCounter >= 1.0f) {
        emitParticle();
        emissionCounter -= 1.0f;
    }

    // 更新所有活动粒子
    for (unsigned int i = 0; i < activeParticles; ++i) {
        Particle& p = particles[i];
        if (p.life > 0.0f) {
            p.life -= deltaTime;
            if (p.life > 0.0f) {
                p.position += p.velocity * deltaTime;
                p.rotation += p.rotationSpeed * deltaTime;
                // 简单的颜色渐变
                p.color.a = p.life / particleLifetime;
            } else {
                // 粒子生命周期结束，重置为 inactive
                p.life = 0.0f;
                // 将最后一个活动粒子移动到当前位置以保持紧密排列
                if (i != activeParticles - 1) {
                    particles[i] = particles[activeParticles - 1];
                    --activeParticles;
                    --i; // 重新检查移动过来的粒子
                } else {
                    --activeParticles;
                }
            }
        }
    }
}

void ParticleSystem::draw(Shader& shader) {
    glDepthFunc(GL_LESS); // 恢复默认深度测试函数
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader.use();
    glBindVertexArray(VAO);
    
    for (unsigned int i = 0; i < activeParticles; ++i) {
        Particle& p = particles[i];
        if (p.life > 0.0f) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, p.position);
            model = glm::translate(model, glm::vec3(0.5f, 0.5f, 0.0f));
            model = glm::rotate(model, glm::radians(p.rotation), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::translate(model, glm::vec3(-0.5f, -0.5f, 0.0f));
            model = glm::scale(model, glm::vec3(p.size));
            
            shader.setMat4("model", model);
            shader.setVec4("particleColor", p.color);
            
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }
    
    glBindVertexArray(0);
    glDisable(GL_BLEND);
}

void ParticleSystem::emitParticle() {
    if (activeParticles < maxParticles) {
        Particle& p = particles[activeParticles];
        respawnParticle(p);
        activeParticles++;
    } else {
        // 如果达到最大粒子数，循环使用最早的粒子
        // 查找第一个死亡的粒子
        for (unsigned int i = 0; i < activeParticles; ++i) {
            if (particles[i].life <= 0.0f) {
                respawnParticle(particles[i]);
                return;
            }
        }
        // 如果没有找到死亡粒子，则替换第一个粒子
        respawnParticle(particles[0]);
    }
}

void ParticleSystem::respawnParticle(Particle& particle) {
    // 随机初始位置（围绕发射器），单位为像素（发射器通常在窗口坐标内）
    float randomX = glm::linearRand(-10.0f, 10.0f);
    float randomY = glm::linearRand(-10.0f, 10.0f);
    float randomZ = glm::linearRand(-1.0f, 1.0f);

    particle.position = emitterPosition + glm::vec3(randomX, randomY, randomZ);

    // 随机初始速度（像素/秒）
    float randomVelX = glm::linearRand(-50.0f, 50.0f);
    float randomVelY = glm::linearRand(20.0f, 150.0f);
    float randomVelZ = glm::linearRand(-10.0f, 10.0f);

    particle.velocity = emitterVelocity + glm::vec3(randomVelX, randomVelY, randomVelZ);

    // 随机颜色（暖色调）
    float red = glm::linearRand(0.5f, 1.0f);
    float green = glm::linearRand(0.2f, red);
    float blue = glm::linearRand(0.0f, green);

    particle.color = glm::vec4(red, green, blue, 1.0f);
    particle.life = particleLifetime;
    particle.rotation = glm::linearRand(0.0f, 360.0f);
    particle.rotationSpeed = glm::linearRand(-90.0f, 90.0f);
    // 粒子大小以像素为单位（quad 模板为 0..1，所以使用 scale 把它扩大为像素）
    particle.size = glm::linearRand(4.0f, 20.0f);
}