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
#include <cmath>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "shapes.hpp"
#include "shader.hpp"
#include "light.hpp"
#include "camera.hpp"
#include "shape/object_manager.hpp"
#include "shape/objects.hpp"
#include "shape/bezier.hpp"
#include "shape/nurbs.hpp"

// 添加GLM的相交检测功能
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/intersect.hpp>

/**
 * @brief GLFW 初始化/终止管理器
 *
 * 负责调用 glfwInit() / glfwTerminate()，保证在多窗口场景下只初始化/终止一次。
 */
class GLCore {
public:
    // 初始化 GLFW（只调用一次）
    static void Initialize();

    // 终止 GLFW（在程序结束时调用一次）
    static void Shutdown();

    static bool is_initialized();

private:
    static inline bool s_initialized = false;
};

#define WINDOW_BASIC                // 窗口基本功能
#define WINDOW_CALLBACK_MANAGER     // 窗口回调函数管理
#define WINDOW_KEY_MANAGER          // 按键管理

// 渲染模式枚举
enum class RenderMode {
    VERTEX_SHADER_RESULT = 0,    // 顶点着色器结果
    RASTERIZED_RESULT = 1,       // 光栅化后结果
    FRAGMENT_SHADER_RESULT = 2,  // 片段着色后结果
    FINAL_RESULT = 3             // 最终处理结果
};

// 聊天框内容。
enum class MessageType {
    NONE,
    INFO,
    WARN,
    ERROR
};

struct HudMessage {
    std::string text;
    MessageType type;
    float life;      // 剩余寿命（秒）
    float totalLife; // 总寿命（用来算透明度）
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
    WINDOW_BASIC explicit Window(int width = 800, int height = 600, const char* title = "OpenGL Window");

    WINDOW_BASIC ~Window();

    // ---------------------- 基本窗口和渲染 ----------------------

    /**
     * @brief 获取底层 GLFWwindow* 指针
     * @return 指向内部 GLFW 窗口对象的指针，可用于注册 GLFW 回调或直接调用 GLFW API
     */
    WINDOW_BASIC GLFWwindow* GetGLFWwindow() const;

    /**
     * @brief 检查窗口是否请求关闭
     * @return 如果窗口已收到关闭事件（例如点击关闭按钮）则返回 true
     */
    WINDOW_BASIC bool ShouldClose() const;

    /**
     * @brief 关闭窗口。
     */
    WINDOW_BASIC void Close();

    /**
     * @brief 交换前后缓冲区（将后备缓冲内容呈现到屏幕）
     */
    WINDOW_BASIC void SwapBuffers();

    /**
     * @brief 增加形状。
     * @param shape 新形状。
     */
    WINDOW_BASIC void AddShape(std::shared_ptr<ColoredShape> shape);

    /**
     * @brief 移除形状。
     * @param shape 要移除的形状。
     * @return 是否成功移除
     */
    WINDOW_BASIC bool RemoveShape(std::shared_ptr<ColoredShape> shape);

    /**
     * @brief 移除所有形状。
     */
    WINDOW_BASIC void RemoveAllShapes();

    /**
     * @brief 获取形状列表（只读）。
     */
    WINDOW_BASIC const std::vector<std::shared_ptr<ColoredShape>>& GetShapes() const { return m_shape_list; }

    /**
     * @brief 增加光源。
     * @param light 新光源。
     */
    WINDOW_BASIC void AddLightSource(std::shared_ptr<Light> light);

    /**
     * @brief 移除光源。
     * @param light 要移除的光源。
     * @return 是否成功移除
     */
    WINDOW_BASIC bool RemoveLightSource(std::shared_ptr<Light> light);

    /**
     * @brief 移除所有光源。
     */
    WINDOW_BASIC void RemoveAllLightSources();

    /**
     * @brief 获取光源列表（只读）。
     */
    WINDOW_BASIC const std::vector<std::shared_ptr<Light>>& GetLightSources() const { return m_light_list; }

    /**
     * @brief 绑定着色器。
     * @param shader 指向着色器的指针。
     */
    WINDOW_BASIC void BindShader(Shader* shader);

    /**
     * @brief 绑定摄像机。
     * @
     */
    WINDOW_BASIC void BindCamera(Camera* camera);

    /**
     * @brief 清理当前帧的颜色/深度缓冲区
     * 本实现会使用固定的背景颜色并清除颜色与深度缓冲。若需自定义背景色请修改此方法或在外部调用 glClearColor
     */
    WINDOW_BASIC void Clear();

    // 主循环
    WINDOW_BASIC void Run();

    // --------------------------- 事件处理 ---------------------------

    /**
     * @brief 处理由操作系统/GLFW 产生的事件（键盘/鼠标/窗口事件等）
     * 通常在每帧末尾调用以处理回调和更新内部事件队列。
     */
    WINDOW_CALLBACK_MANAGER void PollEvents();

    WINDOW_CALLBACK_MANAGER void SetInputMode(int mode, int value);

    WINDOW_CALLBACK_MANAGER void SetCursorPosCallback(GLFWcursorposfun&& mouse_callback);


private:
    ObjectManager& objectManager = ObjectManager::GetInstance(); // 物体管理器。


    int m_width;            // 当前窗口宽
    int m_height;           // 当前窗口高
    const char* m_title;    // 当前窗口标题
    GLFWwindow* m_window;   // 当前实例窗口
    Shader* m_shader;         // 使用的着色器
    Camera* m_camera;         // 当前主镜头

    std::vector<std::shared_ptr<ColoredShape>> m_shape_list;  // 形状列表
    std::vector<std::shared_ptr<Light>> m_light_list;         // 光源列表

    // 渲染模式
    RenderMode m_renderMode = RenderMode::FINAL_RESULT;

    static inline bool s_glewInitialized = false;
    static inline int s_windowCount = 0;
    
    // Crosshair GL objects (modern OpenGL, core profile compatible)
    unsigned int m_crossVAO = 0;
    unsigned int m_crossVBO = 0;
    // 控制台输出时间间隔
    float m_lastCameraOutput;

    // 是否处于对贝塞尔曲线点位的控制状态。
    bool m_is_controlling_bezier = false;
    std::shared_ptr<GeometryObject> m_now_controlling_target = nullptr;   // 是否正在控制一个贝塞尔曲线样点。
    std::shared_ptr<LightObject> m_controlling_light = nullptr;     // 对控制的物体的描边
    uint64_t m_light_oid = 0;

    int m_nurbsDegree = 9999999; // NURBS曲线的最大度数

    enum class CurveDisplayMode { BEZIER = 0, BSPLINE = 1, NURBS = 2 };
    CurveDisplayMode m_curveDisplay = CurveDisplayMode::BEZIER;
    float m_fps = 0.0f;
    double m_lastFpsTime = 0.0;
    int m_frameCounter = 0;
    int m_currentDegree = 0;

    /**
     * @brief 私有函数：键盘行为。
     * @param lastFrame (float*): 用于表达最后一帧的时点。
     */
    WINDOW_KEY_MANAGER void key_callback(float* deltaTime);

    WINDOW_KEY_MANAGER void key_callback_moving(float* deltaTime);

    WINDOW_KEY_MANAGER void key_callback_mouse_inverse();

    WINDOW_KEY_MANAGER void key_callback_render_change();

    WINDOW_KEY_MANAGER void key_callback_quit();

    WINDOW_KEY_MANAGER void key_callback_ray_casting();

    WINDOW_KEY_MANAGER void key_callback_debug_print();

    WINDOW_KEY_MANAGER void key_callback_bezier();

    static WINDOW_KEY_MANAGER void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

    // 真正干活的成员函数
    WINDOW_KEY_MANAGER void onScroll(double xoffset, double yoffset);

    /**
     * @brief 向前循环切换渲染模式
     */
    void cycleRenderModeForward();

    /**
     * @brief 向后循环切换渲染模式
     */
    void cycleRenderModeBackward();

    /**
     * @brief 绘制屏幕中央的准星
     */
    void drawCrosshair();

    /**
     * @brief 射线投射并检测物体，返回被检测到的物体。
     * 这是一个统一的枪逻辑。
     */
    std::shared_ptr<Object> castRayAndDetectObjects();

    void updateCurveVisibility(bool showBezier, bool showBSpline, bool showNURBS);
    
    // Bezier曲线相关函数
    void updateCurve();
    std::shared_ptr<NURBSCurve> updateNurbsShape(const std::string& name, const glm::vec3& color, const std::vector<float>& weights);
    void updateBezierPieces(const std::vector<std::vector<glm::vec3>>& pieces);

};
