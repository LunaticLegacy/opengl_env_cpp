#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <functional>
#include <string>
#include <sstream>
#include <iomanip>

#include "shapes.hpp"
#include "shader.hpp"
#include "light.hpp"
#include "camera.hpp"

/**
 * @brief GLFW 初始化/终止管理器
 *
 * 负责调用 glfwInit() / glfwTerminate()，保证在多窗口场景下只初始化/终止一次。
 */
class GLCore {
public:
    // 初始化 GLFW（只调用一次）
    static void Initialize() {
        if (s_initialized) return;

        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW\n";
            throw std::runtime_error("GLFW init failed");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        s_initialized = true;
    }

    // 终止 GLFW（在程序结束时调用一次）
    static void Shutdown() {
        if (s_initialized) {
            glfwTerminate();
            s_initialized = false;
        }
    }

    static bool is_initialized() {return GLCore::s_initialized;}

private:
    static inline bool s_initialized = false;
};

#define WINDOW_BASIC                // 窗口基本功能
#define WINDOW_CALLBACK_MANAGER     // 窗口回调函数管理

class Window {
public:
    /**
     * @brief 创建并初始化一个窗口
     * @param width 窗口宽度（像素）
     * @param height 窗口高度（像素）
     * @param title 窗口标题
     *
     * 该构造器会：确保 GLFW 被初始化、创建 GLFW 窗口、设置当前上下文、初始化 GLEW（仅首次）、设置视口并启用深度测试。
     */
    WINDOW_BASIC explicit Window(int width = 800, int height = 600, const char* title = "OpenGL Window")
        : m_width(width), m_height(height), m_title(title), m_lastCameraOutput(0.0f)
    {
        if (!GLCore::is_initialized()) GLCore::Initialize(); // 确保 GLFW 已初始化
        
        // 创建窗口
        m_window = glfwCreateWindow(m_width, m_height, m_title, nullptr, nullptr);
        if (!m_window) {
            std::cerr << "Failed to create GLFW window\n";
            throw std::runtime_error("GLFW window creation failed");
        }

        glfwMakeContextCurrent(m_window);

        // 初始化 GLEW（只在第一个窗口调用）
        if (!s_glewInitialized) {
            glewExperimental = GL_TRUE;
            if (glewInit() != GLEW_OK) {
                glfwDestroyWindow(m_window);
                std::cerr << "Failed to initialize GLEW\n";
                throw std::runtime_error("GLEW init failed");
            }
            s_glewInitialized = true;
        }

        glViewport(0, 0, m_width, m_height);
        glEnable(GL_DEPTH_TEST);
        s_windowCount++;
    }

    WINDOW_BASIC ~Window() {
        if (m_window) {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }
        // GLFW 终止只在最后一个窗口析构时调用
        s_windowCount--;
        if (s_windowCount == 0) {
            s_glewInitialized = false;
            GLCore::Shutdown();
        }
    }

    // ---------------------- 基本窗口和渲染 ----------------------

    /**
     * @brief 获取底层 GLFWwindow* 指针
     * @return 指向内部 GLFW 窗口对象的指针，可用于注册 GLFW 回调或直接调用 GLFW API
     */
    WINDOW_BASIC GLFWwindow* GetGLFWwindow() const {
        return m_window;
    }

    /**
     * @brief 检查窗口是否请求关闭
     * @return 如果窗口已收到关闭事件（例如点击关闭按钮）则返回 true
     */
    WINDOW_BASIC bool ShouldClose() const {
        return glfwWindowShouldClose(m_window);
    }

    /**
     * @brief 交换前后缓冲区（将后备缓冲内容呈现到屏幕）
     */
    WINDOW_BASIC void SwapBuffers() {
        glfwSwapBuffers(m_window);
    }

    /**
     * @brief 增加形状。
     * @param shape 新形状。
     */
    WINDOW_BASIC void AddShape(ColoredShape* shape) {
        this->m_shape_list.push_back(shape);
    }

    /**
     * @brief 绑定着色器。
     * @param shader 指向着色器的指针。
     */
    WINDOW_BASIC void BindShader(Shader* shader) {
        this->m_shader = shader;
    }

    /**
     * 
     */
    WINDOW_BASIC void BindCamera(Camera* camera) {
        this->m_camera = camera;
    }

    /**
     * @brief 增加光源。
     * @param light 新光源。
     */
    WINDOW_BASIC void AddLightSource(Light* light) {
        this->m_light_list.push_back(light);
    }

    /**
     * @brief 清理当前帧的颜色/深度缓冲区
     * 本实现会使用固定的背景颜色并清除颜色与深度缓冲。若需自定义背景色请修改此方法或在外部调用 glClearColor
     */
    WINDOW_BASIC void Clear() {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    // 主循环
    WINDOW_BASIC void Run() {
        float lastFrame = 0.0f, currentFrame = 0.0f, deltaTime = 0.0f;

        while (!this->ShouldClose()) {
            this->Clear();
            m_shader->use();

            // 上传视图与投影矩阵到着色器（uniform 名称需与着色器代码一致）
            glm::mat4 view = m_camera->getViewMatrix();
            glm::mat4 projection = m_camera->getProjectionMatrix(800.0f / 600.0f);
            m_shader->setMat4("view", view);
            m_shader->setMat4("projection", projection);

            // 更新时间
            currentFrame = (float)glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
            this->key_callback(&deltaTime);


            // 设置视点位置
            m_shader->setVec3("viewPos", m_camera->Position);
            
            // 设置材质属性
            m_shader->setVec3("material.ambient",  glm::vec3(1.0f, 1.0f, 1.0f));
            m_shader->setVec3("material.diffuse",  glm::vec3(1.0f, 0.5f, 1.0f));
            m_shader->setVec3("material.specular", glm::vec3(0.5f, 0.5f, 0.5f));
            m_shader->setFloat("material.shininess", 32.0f);
            
            // 设置光源属性
            for (auto& light : m_light_list) {
                light->setUniform(m_shader->ID, "light");
            }

            for (auto& shape : m_shape_list) {
                // 为每个形状设置model矩阵
                glm::mat4 model = shape->getModelMatrix();
                m_shader->setMat4("model", model);
                shape->draw(*(this->m_shader));
            }

            // 显示摄像机坐标
            // this->displayCameraCoordinates(currentFrame);

            // 刷新缓冲区，并轮询。
            this->SwapBuffers();
            this->PollEvents();
        }
    }

    // --------------------------- 事件处理 ---------------------------

    /**
     * @brief 处理由操作系统/GLFW 产生的事件（键盘/鼠标/窗口事件等）
     * 通常在每帧末尾调用以处理回调和更新内部事件队列。
     */
    WINDOW_CALLBACK_MANAGER void PollEvents() {
        glfwPollEvents();
    }

    WINDOW_CALLBACK_MANAGER void SetInputMode(int mode, int value) {
        glfwSetInputMode(this->m_window, mode, value);
    }

    WINDOW_CALLBACK_MANAGER void SetCursorPosCallback(GLFWcursorposfun&& mouse_callback) {
        glfwSetCursorPosCallback(this->m_window, mouse_callback);
    }

private:
    int m_width;            // 当前窗口宽
    int m_height;           // 当前窗口高
    const char* m_title;    // 当前窗口标题
    GLFWwindow* m_window;   // 当前实例窗口
    Shader* m_shader;         // 使用的着色器
    Camera* m_camera;         // 当前主镜头

    std::vector<ColoredShape*> m_shape_list;  // 形状列表
    std::vector<Light*> m_light_list;         // 光源列表

    static inline bool s_glewInitialized = false;
    static inline int s_windowCount = 0;
    
    // 控制台输出时间间隔
    float m_lastCameraOutput;


    /**
     * @brief 私有函数：键盘行为。
     * @param lastFrame (float*): 用于表达最后一帧的时点。
     */
    void key_callback(float* deltaTime) {

        // 处理键盘输入（WASD）
        if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
            m_camera->processKeyboard(FORWARD, *deltaTime);
        if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
            m_camera->processKeyboard(BACKWARD, *deltaTime);
        if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
            m_camera->processKeyboard(LEFT, *deltaTime);
        if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
            m_camera->processKeyboard(RIGHT, *deltaTime);
        if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS)
            m_camera->processKeyboard(UP, *deltaTime);
        if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            m_camera->processKeyboard(DOWN, *deltaTime);

        // 切换鼠标反向：按 V 切换 Y 轴反向；按 B 切换 X 轴反向
        static int lastVState = GLFW_RELEASE;
        static int lastBState = GLFW_RELEASE;
        int vState = glfwGetKey(m_window, GLFW_KEY_V);
        int bState = glfwGetKey(m_window, GLFW_KEY_B);
        if (vState == GLFW_PRESS && lastVState == GLFW_RELEASE) {
            m_camera->toggleInvertY();
            std::cout << "InvertY set to " << (m_camera->isInvertY() ? "ON" : "OFF") << std::endl;
        }
        if (bState == GLFW_PRESS && lastBState == GLFW_RELEASE) {
            m_camera->toggleInvertX();
            std::cout << "InvertX set to " << (m_camera->isInvertX() ? "ON" : "OFF") << std::endl;
        }
        lastVState = vState;
        lastBState = bState;
    }

    /**
     * @brief 显示摄像机坐标
     */
    void displayCameraCoordinates(float currentFrame) {
        // 每隔0.5秒在控制台输出一次摄像机坐标
        if (currentFrame - m_lastCameraOutput > 0.5f) {
            glm::vec3 pos = m_camera->Position;
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2);
            ss << "Camera: (" << pos.x << ", " << pos.y << ", " << pos.z << ")";
            std::cout << ss.str() << std::endl;
            m_lastCameraOutput = currentFrame;
        }
        
        // 在屏幕右上角绘制一个简单的指示器
        glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // 禁用深度测试，确保始终在最上层显示
        glDisable(GL_DEPTH_TEST);
        
        // 设置正交投影
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0.0f, m_width, m_height, 0.0f, -1.0f, 1.0f);
        
        // 设置模型视图矩阵
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        
        // 绘制一个简单的坐标指示器（红色方块）
        glBegin(GL_QUADS);
        glColor3f(1.0f, 0.0f, 0.0f);  // 红色
        glVertex2f(m_width - 30, 10);
        glVertex2f(m_width - 10, 10);
        glVertex2f(m_width - 10, 30);
        glVertex2f(m_width - 30, 30);
        glEnd();
        
        // 绘制文字提示（简单绘制几个点代表"C"）
        glColor3f(1.0f, 1.0f, 1.0f);  // 白色
        glBegin(GL_POINTS);
        glVertex2f(m_width - 50, 15);
        glVertex2f(m_width - 55, 20);
        glVertex2f(m_width - 50, 25);
        glEnd();
        
        // 恢复OpenGL状态
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        
        glPopAttrib();
    }
};