#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <vector>

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

    /**
     * @brief 获取底层 GLFWwindow* 指针
     * @return 指向内部 GLFW 窗口对象的指针，可用于注册 GLFW 回调或直接调用 GLFW API
     */
    GLFWwindow* GetGLFWwindow() const {
        return m_window;
    }

    /**
     * @brief 检查窗口是否请求关闭
     * @return 如果窗口已收到关闭事件（例如点击关闭按钮）则返回 true
     */
    bool ShouldClose() const {
        return glfwWindowShouldClose(m_window);
    }

    /**
     * @brief 交换前后缓冲区（将后备缓冲内容呈现到屏幕）
     */
    void SwapBuffers() {
        glfwSwapBuffers(m_window);
    }

    /**
     * @brief 处理由操作系统/GLFW 产生的事件（键盘/鼠标/窗口事件等）
     *
     * 通常在每帧末尾调用以处理回调和更新内部事件队列。
     */
    void PollEvents() {
        glfwPollEvents();
    }

    /**
     * @brief 清理当前帧的颜色/深度缓冲区
     *
     * 本实现会使用固定的背景颜色并清除颜色与深度缓冲。若需自定义背景色请修改此方法或在外部调用 glClearColor
     */
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