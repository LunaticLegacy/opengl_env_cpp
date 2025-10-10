#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <vector>

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


class Window {
public:
    Window(int width = 800, int height = 600, const char* title = "OpenGL Window")
        : m_width(width), m_height(height), m_title(title)
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

    ~Window() {
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

    // 获取窗口对象
    GLFWwindow* GetGLFWwindow() const {
        return m_window;
    }

    // 检查窗口是否应该关闭
    bool ShouldClose() const {
        return glfwWindowShouldClose(m_window);
    }

    // 交换缓冲区
    void SwapBuffers() {
        glfwSwapBuffers(m_window);
    }

    // 处理事件
    void PollEvents() {
        glfwPollEvents();
    }

    // 清除屏幕
    void Clear() {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    // 主循环
    void Run() {
        while (!ShouldClose()) {
            Clear();

            // TODO: 绘制内容。

            SwapBuffers();
            PollEvents();
        }
    }

private:
    int m_width;
    int m_height;
    const char* m_title;
    GLFWwindow* m_window;

    static inline bool s_glewInitialized = false;
    static inline int s_windowCount = 0;
};