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

// 全局用于回调的指针（简单处理）
static Camera* g_camera = nullptr;
static bool firstMouse = true;
static double lastX = 400.0, lastY = 300.0;

// 鼠标移动回调，用来控制摄像机朝向
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!g_camera) return;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = (float)(xpos - lastX);
    float yoffset = (float)(lastY - ypos); // 反转 y
    lastX = xpos;
    lastY = ypos;

    g_camera->processMouseMovement(xoffset, yoffset);
}

int main() {
    try {
        // 创建窗口
        Window window(800, 600, "OpenGL 3D Engine");

        // 创建着色器程序
        Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");
        shader.use();

        // 创建摄像机并设置为全局供回调使用
        Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
        camera.setPerspective(45.0f, 0.1f, 100.0f);
        g_camera = &camera;

        // 设置鼠标回调（需要 GLFWwindow*）
        glfwSetInputMode(window.GetGLFWwindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window.GetGLFWwindow(), mouse_callback);

        // 先反转一下鼠标。
        camera.toggleInvertX();

        float lastFrame = 0.0f;

        // 创建一些基本图形对象
        Point3D point(0.0f, 0.0f, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f)); // 红色点
        Line line(-0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f)); // 绿色线
        Triangle triangle(0.0f, 0.5f, 0.0f,
                         -0.5f, -0.5f, 0.0f,
                          0.5f, -0.5f, 0.0f,
                          glm::vec3(0.0f, 0.0f, 1.0f)); // 蓝色三角形

        // 创建增强的3D图形对象
        Cube cube(0.5f, glm::vec3(1.0f, 1.0f, 0.0f)); // 黄色立方体
        cube.setPosition(glm::vec3(1.0f, 0.0f, 0.0f));
        
        Sphere sphere(0.3f, 36, 18, glm::vec3(1.0f, 0.0f, 1.0f)); // 紫色球体
        sphere.setPosition(glm::vec3(-1.0f, 0.0f, 0.0f));

        // 主渲染循环
        while (!window.ShouldClose()) {
            float currentFrame = (float)glfwGetTime();
            float deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            // 处理键盘输入（WASD）
            if (glfwGetKey(window.GetGLFWwindow(), GLFW_KEY_W) == GLFW_PRESS)
                camera.processKeyboard(FORWARD, deltaTime);
            if (glfwGetKey(window.GetGLFWwindow(), GLFW_KEY_S) == GLFW_PRESS)
                camera.processKeyboard(BACKWARD, deltaTime);
            if (glfwGetKey(window.GetGLFWwindow(), GLFW_KEY_A) == GLFW_PRESS)
                camera.processKeyboard(LEFT, deltaTime);
            if (glfwGetKey(window.GetGLFWwindow(), GLFW_KEY_D) == GLFW_PRESS)
                camera.processKeyboard(RIGHT, deltaTime);

            // 切换鼠标反向：按 V 切换 Y 轴反向；按 B 切换 X 轴反向
            static int lastVState = GLFW_RELEASE;
            static int lastBState = GLFW_RELEASE;
            int vState = glfwGetKey(window.GetGLFWwindow(), GLFW_KEY_V);
            int bState = glfwGetKey(window.GetGLFWwindow(), GLFW_KEY_B);
            if (vState == GLFW_PRESS && lastVState == GLFW_RELEASE) {
                camera.toggleInvertY();
                std::cout << "InvertY set to " << (camera.isInvertY() ? "ON" : "OFF") << std::endl;
            }
            if (bState == GLFW_PRESS && lastBState == GLFW_RELEASE) {
                camera.toggleInvertX();
                std::cout << "InvertX set to " << (camera.isInvertX() ? "ON" : "OFF") << std::endl;
            }
            lastVState = vState;
            lastBState = bState;

            window.Clear();

            // 使用着色器并绘制所有形状
            shader.use();
            // 上传 view 和 projection
            glm::mat4 view = camera.getViewMatrix();
            glm::mat4 projection = camera.getProjectionMatrix(800.0f / 600.0f);
            shader.setMat4("view", view);
            shader.setMat4("projection", projection);

            point.draw(shader);
            line.draw(shader);
            triangle.draw(shader);
            cube.draw(shader);
            sphere.draw(shader);

            window.SwapBuffers();
            window.PollEvents();
        }
    } catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}