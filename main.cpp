#include <iostream>
#include <memory>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glwindow.hpp"
#include "shapes.hpp"
#include "shader.hpp"

int main() {
    try {
        // 创建窗口
        Window window(800, 600, "OpenGL Window");

        // 创建着色器程序
        Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");
        shader.use();

        // 创建一些基本图形对象
        Point3D point(0.0f, 0.0f, 0.0f);
        Line line(-0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f);
        Triangle triangle(0.0f, 0.5f, 0.0f, 
                         -0.5f, -0.5f, 0.0f,
                          0.5f, -0.5f, 0.0f);

        // 主渲染循环
        while (!window.ShouldClose()) {
            window.Clear();

            // 使用着色器并绘制所有形状
            shader.use();
            point.draw();
            line.draw();
            triangle.draw();

            window.SwapBuffers();
            window.PollEvents();
        }
    } catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}