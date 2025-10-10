#include <iostream>
#include <memory>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "glwindow.hpp"
#include "shapes.hpp"
#include "shader.hpp"
#include "particle_system.hpp"

int main() {
    try {
        // 创建窗口
        Window window(800, 600, "OpenGL Particle System Demo");

        // 创建着色器程序
        Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");
        
        // 设置投影矩阵
        glm::mat4 projection = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f);
        shader.use();
        shader.setMat4("projection", projection);

        // 创建粒子系统
        ParticleSystem particleSystem(2000);
        particleSystem.setEmissionRate(300.0f); // 每秒发射300个粒子
        particleSystem.setParticleLifetime(2.0f); // 粒子生命周期2秒
    // 将发射器放在窗口中心（像素坐标系）
    particleSystem.setEmitterPosition(glm::vec3(400.0f, 300.0f, 0.0f));

        // 记录时间
        float lastTime = glfwGetTime();
        
        // 主渲染循环
        while (!window.ShouldClose()) {
            float currentTime = glfwGetTime();
            float deltaTime = currentTime - lastTime;
            lastTime = currentTime;
            
            window.Clear();

            // 更新和绘制粒子系统
            particleSystem.update(deltaTime);
            particleSystem.draw(shader);

            window.SwapBuffers();
            window.PollEvents();
        }
    } catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}