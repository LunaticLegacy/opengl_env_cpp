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

// Global camera pointer used by the GLFW callbacks
static Camera* g_camera = nullptr;

// 使用一个全局的物体管理器，创建物体。
ObjectManager& objectManager = ObjectManager::GetInstance();

/**
 * 设置鼠标回调
 */
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!g_camera) return;
    g_camera->processMouseCallback(xpos, ypos);
}

/**
 * 设置Shader
 */
void configureShader(Window& window, Shader& shader) {
    shader.use();
    window.BindShader(&shader);
}

/**
 * 设置摄像机
 */
void setupCamera(Window& window, Camera& camera) {
    camera.setPerspective(45.0f, 0.1f, 100.0f);
    camera.toggleInvertX(); // keep mouse left/right natural
    g_camera = &camera;
    window.BindCamera(&camera);
    window.SetInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    window.SetCursorPosCallback(mouse_callback);
}

/**
 * 对窗口创建图形
 */
void addBasicShapes(Window& window) {
    auto point = std::make_shared<Point3D>(0.0f, 0.0f, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    auto pointObj = std::make_shared<GeometryObject>(glm::vec3(0.0, 0.0, 0.0), point);
    window.AddShape(std::move(point));
    objectManager.AddObject(std::move(pointObj));

    auto line = std::make_shared<Line>(-0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    auto lineObj = std::make_shared<GeometryObject>(glm::vec3(0.0, 0.0, 0.0), line);
    window.AddShape(std::move(line));
    objectManager.AddObject(std::move(lineObj));

    auto triangle = std::make_shared<Triangle>(
        0.0f, 0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        glm::vec3(0.0f, 0.0f, 1.0f)
    );
    auto triangleObj = std::make_shared<GeometryObject>(glm::vec3(0.0, 0.0, 0.0), triangle);
    window.AddShape(std::move(triangle));
    objectManager.AddObject(std::move(triangleObj));

    auto cube = std::make_shared<Cube>(0.5f, glm::vec3(1.0f, 1.0f, 0.0f));
    cube->setPosition(glm::vec3(1.0f, 0.0f, 0.0f));
    auto cubeObj = std::make_shared<GeometryObject>(glm::vec3(1.0, 0.0, 0.0), cube);
    window.AddShape(std::move(cube));
    objectManager.AddObject(std::move(cubeObj));
    
    auto sphere = std::make_shared<Sphere>(0.3f, 36, 18, glm::vec3(1.0f, 0.2f, 1.0f));
    sphere->setPosition(glm::vec3(-1.0f, 0.0f, 0.0f));
    auto sphereObj = std::make_shared<GeometryObject>(glm::vec3(-1.0, 0.0, 0.0), sphere);
    window.AddShape(std::move(sphere));
    objectManager.AddObject(std::move(sphereObj));
    
}

/**
 * 设置光源
 */
void addLightSources(Window& window) {

    auto whitelight = std::make_shared<Light>(POINT_LIGHT, glm::vec3(1.2f, -2.0f, 2.0f));
    whitelight->setColor(glm::vec3(1.0f), glm::vec3(0.9f), glm::vec3(0.9f));
    window.AddLightSource(std::move(whitelight));

    auto redlight = std::make_shared<Light>(POINT_LIGHT, glm::vec3(-1.2f, 2.0f, -1.0f));
    redlight->setColor(
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 0.9f, 0.9f),
        glm::vec3(1.0f, 0.9f, 0.9f)
    );
    auto redlightObj = std::make_shared<LightObject>(glm::vec3(1.2f, -2.0f, 2.0f), redlight);
    window.AddLightSource(std::move(redlight));
    objectManager.AddObject(redlightObj);
}

/**
 * 场景启动
 */
void setupScene(Window& window) {
    addBasicShapes(window);
    // addLightSources(window);
}

int main() {
    try {
        // 创建窗口、渲染器和摄像机
        Window window(1024, 768, "OpenGL 3D Engine");

        Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");
        configureShader(window, shader);

        Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
        setupCamera(window, camera);

        setupScene(window);

        window.Run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}