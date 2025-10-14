#include <iostream>
#include <memory>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glwindow.hpp"
#include "shapes.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "light.hpp"

// 全局用于回调的指针（简单处理）
static Camera* g_camera = nullptr;

/**
 * @brief 鼠标移动回调
 * @param window 触发事件的 GLFWwindow*（可用于额外查询输入状态）
 * @param xpos 鼠标当前 X 坐标（像素，相对于窗口左上角）
 * @param ypos 鼠标当前 Y 坐标（像素，相对于窗口左上角）
 *
 * 回调会把鼠标偏移量传入全局摄像机对象的 processMouseMovement。
 */
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!g_camera) return;
    g_camera->processMouseCallback(xpos, ypos);
}

int main() {
    try {
        // 创建窗口
        Window window(800, 600, "OpenGL 3D Engine");

        // 创建着色器程序
        Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");
        shader.use();
        window.BindShader(&shader);

        // 创建摄像机并设置为全局供回调使用
        Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
        camera.setPerspective(45.0f, 0.1f, 100.0f);
        camera.toggleInvertX();
        g_camera = &camera;
        window.BindCamera(&camera); // 绑定摄像机到窗口

        // 设置鼠标回调（需要 GLFWwindow*）
        window.SetInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        window.SetCursorPosCallback(mouse_callback);

        // 可根据个人习惯初始化鼠标反向（如需默认反向可取消注释）
        float lastFrame = 0.0f;

        // 创建一些基本图形对象
        Point3D* point = new Point3D(0.0f, 0.0f, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f)); // 红色点
        window.AddShape(point);
        Line* line = new Line(-0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f)); // 绿色线
        window.AddShape(line);

        Triangle* triangle = new Triangle(
            0.0f, 0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            glm::vec3(0.0f, 0.0f, 1.0f)
        ); // 蓝色三角形
        window.AddShape(triangle);

        // 创建增强的3D图形对象
        Cube* cube = new Cube(0.5f, glm::vec3(1.0f, 1.0f, 0.0f)); // 黄色立方体
        cube->setPosition(glm::vec3(1.0f, 0.0f, 0.0f));
        window.AddShape(cube);
        
        Sphere* sphere = new Sphere(0.3f, 36, 18, glm::vec3(1.0f, 0.2f, 1.0f)); // 紫色球体
        sphere->setPosition(glm::vec3(-1.0f, 0.0f, 0.0f));
        window.AddShape(sphere);
        
        Quad* back_white = new Quad(
            0.0f, 0.0f, 0.0f,
            4.0f, 4.0f, 0.0f,
            0.0f, 4.0f, 0.0f,
            4.0f, 0.0f, 0.0f,
            glm::vec3(1.0f, 1.0f, 1.0f)
        );
        window.AddShape(back_white);

        // 创建光源
        Sphere* sun = new Sphere(0.5f);
        sun->setPosition(glm::vec3(1.2f, 1.0f, 2.0f));
        window.AddShape(sun);

        Light* light = new Light(POINT_LIGHT, glm::vec3(1.2f, 1.0f, 2.0f));
        light->setColor(glm::vec3(0.2f), glm::vec3(0.5f), glm::vec3(1.0f));
        window.AddLightSource(light);

        window.Run();
        
    } catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}