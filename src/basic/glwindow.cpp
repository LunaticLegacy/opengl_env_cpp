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
#include <algorithm>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "basic/glwindow.hpp"

#define GLM_ENABLE_EXPERIMENTAL

// GLCore 实现

void GLCore::Initialize() {
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

void GLCore::Shutdown() {
    if (s_initialized) {
        glfwTerminate();
        s_initialized = false;
    }
}

bool GLCore::is_initialized() {
    return GLCore::s_initialized;
}

// Window 类实现

Window::Window(int width, int height, const char* title)
    : m_width(width), m_height(height), m_title(title), m_lastCameraOutput(0.0f)
{
    if (!GLCore::is_initialized()) GLCore::Initialize(); // 确保 GLFW 已初始化
    
    // 创建窗口
    this->m_window = glfwCreateWindow(m_width, m_height, m_title, nullptr, nullptr);
    if (!this->m_window) {
        std::cerr << "Failed to create GLFW window\n";
        throw std::runtime_error("GLFW window creation failed");
    }

    glfwMakeContextCurrent(this->m_window);

    // 初始化 GLEW（只在第一个窗口调用）
    if (!s_glewInitialized) {
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            glfwDestroyWindow(this->m_window);
            std::cerr << "Failed to initialize GLEW\n";
            throw std::runtime_error("GLEW init failed");
        }
        s_glewInitialized = true;
    }

    glViewport(0, 0, m_width, m_height);
    glEnable(GL_DEPTH_TEST);
    // 启用面剔除以提高渲染性能并解决深度问题
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    // 初始化用于绘制准星的 VAO/VBO（动态更新）
    glGenVertexArrays(1, &m_crossVAO);
    glGenBuffers(1, &m_crossVBO);
    glBindVertexArray(m_crossVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_crossVBO);
    // 每个顶点使用 pos(3) + normal(3) + color(3) = 9 floats；准备4个顶点的缓冲
    glBufferData(GL_ARRAY_BUFFER, 4 * 9 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    // ImGui initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    s_windowCount++;

    m_lastFpsTime = glfwGetTime();

    // 加入鼠标指针回调。
    glfwSetWindowUserPointer(this->m_window, this);
    glfwSetScrollCallback(m_window, scroll_callback);

}

Window::~Window() {
    if (this->m_window) {
        // 在上下文仍然有效时清理准星的 GL 资源
        if (m_crossVBO) { glDeleteBuffers(1, &m_crossVBO); m_crossVBO = 0; }
        if (m_crossVAO) { glDeleteVertexArrays(1, &m_crossVAO); m_crossVAO = 0; }

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(this->m_window);
        this->m_window = nullptr;
    }
    // GLFW 终止只在最后一个窗口析构时调用
    s_windowCount--;
    if (s_windowCount == 0) {
        s_glewInitialized = false;
        GLCore::Shutdown();
    }
}

GLFWwindow* Window::GetGLFWwindow() const {
    return this->m_window;
}

bool Window::ShouldClose() const {
    return glfwWindowShouldClose(this->m_window);
}

void Window::Close() {
    glfwSetWindowShouldClose(this->m_window, GLFW_TRUE);
}

void Window::SwapBuffers() {
    glfwSwapBuffers(this->m_window);
}

void Window::AddShape(std::shared_ptr<ColoredShape> shape) {
    this->m_shape_list.push_back(shape);
}

bool Window::RemoveShape(std::shared_ptr<ColoredShape> shape) {
    auto it = std::find(m_shape_list.begin(), m_shape_list.end(), shape);
    if (it != m_shape_list.end()) {
        m_shape_list.erase(it);
        return true;
    }
    return false;
}

void Window::RemoveAllShapes() {
    m_shape_list.clear();
}

void Window::AddLightSource(std::shared_ptr<Light> light) {
    this->m_light_list.push_back(light);
}

bool Window::RemoveLightSource(std::shared_ptr<Light> light) {
    auto it = std::find(m_light_list.begin(), m_light_list.end(), light);
    if (it != m_light_list.end()) {
        m_light_list.erase(it);
        return true;
    }
    return false;
}

void Window::RemoveAllLightSources() {
    m_light_list.clear();
}

void Window::BindShader(Shader* shader) {
    this->m_shader = shader;
}

void Window::BindCamera(Camera* camera) {
    this->m_camera = camera;
}

void Window::Clear() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::Run() {
    float lastFrame = 0.0f, currentFrame = 0.0f, deltaTime = 0.0f;
    std::cout << " --------------- " << std::endl;
    std::cout << "Window started." << std::endl;
    std::cout << "Press WSAD to move. " << std::endl;
    std::cout << "Press LShift to dive, press SPACEBAR to float. " << std::endl;
    std::cout << "Press V to change vertical mouse behaviour, press B to change horizontal mouse behaviour." << std::endl;
    std::cout << "Press U and I to change rendering mode." << std::endl;
    std::cout << "Press P to print object manager stats." << std::endl;
    std::cout << "Press J / K to switch curve type (Bezier / B-Spline / NURBS)." << std::endl;
    std::cout << "Press [ / ] to change NURBS degree (limited by control count)." << std::endl;
    std::cout << "Curves: Bezier(Y), NURBS(M), BSpline(C)." << std::endl;
    std::cout << "Press RMB to place a Bezier control point." << std::endl;
    std::cout << "Press LMB while aiming on a control point to remove it." << std::endl;
    std::cout << "Press ESC to quit the demo." << std::endl;
    std::cout << " --------------- " << std::endl;

    std::shared_ptr<Light> lux = std::make_shared<Light>(POINT_LIGHT, glm::vec3({-4.0, 0.0, 0.0}));
    lux->setColor(glm::vec3({0.4, 0.4, 0.4}), glm::vec3({0.3, 0.6, 0.5}), glm::vec3({0.9, 0.6, 0.9}));
    std::shared_ptr<LightObject> lObj = std::make_shared<LightObject>(glm::vec3({-4.0, 0.0, 0.0}), lux);

    this->AddLightSource(lux);
    this->objectManager.AddObject(lObj);

    while (!this->ShouldClose()) {
        this->Clear();
        m_shader->use();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 上传视图与投影矩阵到着色器（uniform 名称需与着色器代码一致）
        glm::mat4 view = m_camera->getViewMatrix();
        glm::mat4 projection = m_camera->getProjectionMatrix((float)m_width / (float)m_height);
        m_shader->setMat4("view", view);
        m_shader->setMat4("projection", projection);

        // 设置渲染模式
        m_shader->setInt("renderMode", static_cast<int>(m_renderMode));

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

        // 光点转转转
        double time = glfwGetTime();
        double radius = 4.0;
        double lux_x = radius * cos(time);
        double lux_z = radius * sin(time);
        lux->SetPosition(glm::vec3(static_cast<float>(lux_x), 0.0f, static_cast<float>(lux_z)));

        // 设置光源属性
        int lightCount = static_cast<int>(m_light_list.size());
        for (int i = 0; i < lightCount && i < 8; i++) {
            std::string lightName = "lights[" + std::to_string(i) + "]";
            m_light_list[i]->setUniform(m_shader->ID, lightName);
        }
        m_shader->setInt("lightCount", lightCount);

        // 更新所有对象
        objectManager.UpdateAll(deltaTime);

        // 绘制所有形状
        for (auto& shape : m_shape_list) {
            // 为每个形状设置model矩阵
            glm::mat4 model = shape->getModelMatrix();
            m_shader->setMat4("model", model);
            shape->draw(*(this->m_shader));
        }

        // 通过ObjectManager绘制对象
        objectManager.DrawAll(*(this->m_shader));

        // 绘制准星
        drawCrosshair();

        // 计算 FPS
        m_frameCounter++;
        double now = glfwGetTime();
        if (now - m_lastFpsTime >= 1.0) {
            m_fps = static_cast<float>(m_frameCounter / (now - m_lastFpsTime));
            m_frameCounter = 0;
            m_lastFpsTime = now;
        }

        // HUD
        ImGui::SetNextWindowBgAlpha(0.35f);
        ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(260.0f, 90.0f), ImGuiCond_Always);
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav;
        ImGui::Begin("HUD", nullptr, flags);
        ImGui::Text("FPS: %.1f", m_fps);
        ImGui::Text("Curve degree: %d", m_currentDegree);
        const char* curveModeStr = "Bezier";
        switch (m_curveDisplay) {
            case CurveDisplayMode::BEZIER: curveModeStr = "Bezier"; break;
            case CurveDisplayMode::BSPLINE: curveModeStr = "B-Spline"; break;
            case CurveDisplayMode::NURBS: curveModeStr = "NURBS"; break;
        }
        ImGui::Text("Curve type: %s", curveModeStr);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // 刷新缓冲区，并轮询。
        this->SwapBuffers();
        this->PollEvents();
    }

    std::cout << "Quit." << std::endl;
}

void Window::PollEvents() {
    glfwPollEvents();
}

void Window::SetInputMode(int mode, int value) {
    glfwSetInputMode(this->m_window, mode, value);
}

void Window::SetCursorPosCallback(GLFWcursorposfun&& mouse_callback) {
    glfwSetCursorPosCallback(this->m_window, mouse_callback);
}

void Window::key_callback(float* deltaTime) {
    key_callback_moving(deltaTime);
    key_callback_mouse_inverse();
    key_callback_render_change();
    key_callback_quit();
    key_callback_debug_print();
    key_callback_bezier();
}

void Window::key_callback_moving(float* deltaTime) {
    // 处理键盘输入（WASD）
    if (glfwGetKey(this->m_window, GLFW_KEY_W) == GLFW_PRESS)
        m_camera->processKeyboard(FORWARD, *deltaTime);
    if (glfwGetKey(this->m_window, GLFW_KEY_S) == GLFW_PRESS)
        m_camera->processKeyboard(BACKWARD, *deltaTime);
    if (glfwGetKey(this->m_window, GLFW_KEY_A) == GLFW_PRESS)
        m_camera->processKeyboard(LEFT, *deltaTime);
    if (glfwGetKey(this->m_window, GLFW_KEY_D) == GLFW_PRESS)
        m_camera->processKeyboard(RIGHT, *deltaTime);
    if (glfwGetKey(this->m_window, GLFW_KEY_SPACE) == GLFW_PRESS)
        m_camera->processKeyboard(UP, *deltaTime);
    if (glfwGetKey(this->m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        m_camera->processKeyboard(DOWN, *deltaTime);
}

void Window::key_callback_mouse_inverse() {
    // 切换鼠标反向：按 V 切换 Y 轴反向；按 B 切换 X 轴反向
    static int lastVState = GLFW_RELEASE;
    static int lastBState = GLFW_RELEASE;
    int vState = glfwGetKey(this->m_window, GLFW_KEY_V);
    int bState = glfwGetKey(this->m_window, GLFW_KEY_B);
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

void Window::key_callback_render_change() {
    // 渲染模式切换
    static int lastUState = GLFW_RELEASE;
    static int lastIState = GLFW_RELEASE;
    int uState = glfwGetKey(this->m_window, GLFW_KEY_U);
    int iState = glfwGetKey(this->m_window, GLFW_KEY_I);
    
    if (uState == GLFW_PRESS && lastUState == GLFW_RELEASE) {
        cycleRenderModeForward();
    }
    if (iState == GLFW_PRESS && lastIState == GLFW_RELEASE) {
        cycleRenderModeBackward();
    }
    lastUState = uState;
    lastIState = iState;
}

void Window::key_callback_quit() {
    // 按ESC以退出。
    if (glfwGetKey(this->m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(this->m_window, GLFW_TRUE);
    }
}

void Window::key_callback_debug_print() {
    // 按P键打印调试信息
    static int lastPState = GLFW_RELEASE;
    int pState = glfwGetKey(this->m_window, GLFW_KEY_P);
    
    if (pState == GLFW_PRESS && lastPState == GLFW_RELEASE) {
        auto& objectManager = ObjectManager::GetInstance();
        objectManager.PrintDebugInfo();
        
        auto stats = objectManager.GetSceneStats();
        std::cout << "\n=== Scene Statistics ===" << std::endl;
        std::cout << "Total Objects: " << stats.totalObjects << std::endl;
        std::cout << "Active Objects: " << stats.activeObjects << std::endl;
        std::cout << "Visible Objects: " << stats.visibleObjects << std::endl;
        std::cout << "Geometry Objects: " << stats.geometryObjects << std::endl;
        std::cout << "Bezier Objects: " << stats.bezierObjects << std::endl;
        std::cout << "Light Objects: " << stats.lightObjects << std::endl;
        std::cout << "========================\n" << std::endl;
    }
    lastPState = pState;
}


void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    // 从 GLFWwindow 里拿回当初塞进去的 Window*
    void* ptr = glfwGetWindowUserPointer(window);
    auto self = static_cast<Window*>(ptr);
    if (!self) return;

    // 真正逻辑交给成员函数处理
    self->onScroll(xoffset, yoffset);
    
}

void Window::onScroll(double x_offset, double y_offset) {
    // 这是贝塞尔曲线的子函数。注意：现在是static状态。
    // yoffset: 纵向滚轮，上滚为正，下滚为负
    if (m_now_controlling_target) {
        // 获取当前位置和目标距离的欧氏距离。
        const glm::vec3& pos_target = this->m_now_controlling_target->GetPosition();
        const glm::vec3& cam_target = this->m_camera->Position;
        glm::vec3 delta({
            pos_target[0] - cam_target[0],
            pos_target[1] - cam_target[1],
            pos_target[2] - cam_target[2]
        });
        // 单位化差值向量。
        glm::vec delta_square = glm::pow(delta, {2, 2, 2});
        double len = glm::sqrt(delta_square[0] + delta_square[1] + delta_square[2]);
        delta /= len;

        if (y_offset < 0) {
            this->m_now_controlling_target->Move(-delta);
            this->m_controlling_light->Move(-delta);
            this->m_controlling_light->GetLight()->Move(-delta);
        }
        else if (y_offset > 0) {
            this->m_now_controlling_target->Move(delta);
            this->m_controlling_light->Move(delta);
            this->m_controlling_light->GetLight()->Move(delta);
        }
    }
}


void Window::key_callback_bezier() {
    // Bezier / NURBS controls.
    // RMB: add control point. LMB: delete control point. MMB: lock a control point then scroll to push/pull it.
    // [ / ] : change NURBS degree (clamped by control-point count).
    static int lastRightState = GLFW_RELEASE;
    static int lastLeftState = GLFW_RELEASE;
    static int lastMidState = GLFW_RELEASE;
    static int lastDegDownState = GLFW_RELEASE;
    static int lastDegUpState = GLFW_RELEASE;
    static int lastJState = GLFW_RELEASE;
    static int lastKState = GLFW_RELEASE;
    
    int rightState = glfwGetMouseButton(this->m_window, GLFW_MOUSE_BUTTON_RIGHT);
    int leftState = glfwGetMouseButton(this->m_window, GLFW_MOUSE_BUTTON_LEFT);
    int midState = glfwGetMouseButton(this->m_window, GLFW_MOUSE_BUTTON_MIDDLE);
    int degDownState = glfwGetKey(this->m_window, GLFW_KEY_LEFT_BRACKET);
    int degUpState = glfwGetKey(this->m_window, GLFW_KEY_RIGHT_BRACKET);
    int jState = glfwGetKey(this->m_window, GLFW_KEY_J);
    int kState = glfwGetKey(this->m_window, GLFW_KEY_K);

    if (degDownState == GLFW_PRESS && lastDegDownState == GLFW_RELEASE) {
        if (m_nurbsDegree > 1) m_nurbsDegree--;
        std::cout << "Target NURBS degree set to " << m_nurbsDegree << std::endl;
    }
    if (degUpState == GLFW_PRESS && lastDegUpState == GLFW_RELEASE) {
        m_nurbsDegree++;
        std::cout << "Target NURBS degree set to " << m_nurbsDegree << std::endl;
    }
    lastDegDownState = degDownState;
    lastDegUpState = degUpState;

    if (jState == GLFW_PRESS && lastJState == GLFW_RELEASE) {
        switch (m_curveDisplay) {
            case CurveDisplayMode::BEZIER: m_curveDisplay = CurveDisplayMode::NURBS; break;
            case CurveDisplayMode::BSPLINE: m_curveDisplay = CurveDisplayMode::BEZIER; break;
            case CurveDisplayMode::NURBS: m_curveDisplay = CurveDisplayMode::BSPLINE; break;
        }
        std::cout << "Curve type: " << (m_curveDisplay == CurveDisplayMode::BEZIER ? "Bezier" :
                                         m_curveDisplay == CurveDisplayMode::BSPLINE ? "B-Spline" : "NURBS") << std::endl;
    }
    if (kState == GLFW_PRESS && lastKState == GLFW_RELEASE) {
        switch (m_curveDisplay) {
            case CurveDisplayMode::BEZIER: m_curveDisplay = CurveDisplayMode::BSPLINE; break;
            case CurveDisplayMode::BSPLINE: m_curveDisplay = CurveDisplayMode::NURBS; break;
            case CurveDisplayMode::NURBS: m_curveDisplay = CurveDisplayMode::BEZIER; break;
        }
        std::cout << "Curve type: " << (m_curveDisplay == CurveDisplayMode::BEZIER ? "Bezier" :
                                         m_curveDisplay == CurveDisplayMode::BSPLINE ? "B-Spline" : "NURBS") << std::endl;
    }
    lastJState = jState;
    lastKState = kState;

    // RMB: create control point at camera position
    if (rightState == GLFW_PRESS && lastRightState == GLFW_RELEASE) {
        glm::vec3 placePos = m_camera->Position;

        auto ctrlShape = std::make_shared<Sphere>(0.06f, 12, 8, glm::vec3(1.0f, 1.0f, 0.0f));
        ctrlShape->setPosition(placePos);
        auto ctrlObj = std::make_shared<GeometryObject>(placePos, ctrlShape);

        ctrlObj->SetName(std::string("bezier_control_") + std::to_string(ctrlObj->GetID()));
        objectManager.AddObject(ctrlObj);
        objectManager.TagObject(ctrlObj->GetID(), "bezier_control");
        
        std::cout << "Created control point " << ctrlObj->GetName() << " at "
                  << placePos.x << "," << placePos.y << "," << placePos.z << std::endl;

        updateCurve();
    }

    // LMB: ray test and remove control point
    if (leftState == GLFW_PRESS && lastLeftState == GLFW_RELEASE) {
        auto hit = castRayAndDetectObjects();
        if (hit) {
            auto ctrlList = objectManager.GetObjectsByTag("bezier_control");
            bool isCtrl = false;
            for (auto &c : ctrlList) {
                if (c->GetID() == hit->GetID()) { isCtrl = true; break; }
            }
            if (isCtrl) {
                std::cout << "Removing Bezier control point " << hit->GetName() << std::endl;
                objectManager.RemoveObjectByID(hit->GetID());
            } else {
                std::cout << "Hit object is not a bezier control point: " << hit->GetName() << std::endl;
            }
        }
    }

    // MMB: toggle controlling a point
    if (midState == GLFW_PRESS && lastMidState == GLFW_RELEASE) {
        if (!m_is_controlling_bezier) {
            std::shared_ptr<Object> hit = castRayAndDetectObjects();
            if (hit) {
                auto ctrlList = objectManager.GetObjectsByTag("bezier_control");
                bool isCtrl = false;
                for (auto &c : ctrlList) {
                    if (c->GetID() == hit->GetID()) { isCtrl = true; break; }
                }
                if (isCtrl) {
                    m_is_controlling_bezier = true;
                    m_now_controlling_target = 
                        std::dynamic_pointer_cast<GeometryObject>(hit);

                    const glm::vec3& pos = hit->GetPosition();
                    std::shared_ptr<Light> greenLight = std::make_shared<Light>(
                        POINT_LIGHT, pos
                    );
                    greenLight->setColor(
                        glm::vec3(0.0f, 0.1f, 0.0f),
                        glm::vec3(0.0f, 0.1f, 0.0f),
                        glm::vec3(0.0f, 0.1f, 0.0f)
                    );
                    m_controlling_light = std::make_shared<LightObject>(
                        pos, greenLight
                    );
                    this->AddLightSource(greenLight);
                    objectManager.AddObject(m_controlling_light);
                    m_light_oid = m_controlling_light->GetID();
                }
            }
        } else {
            m_is_controlling_bezier = false;
            m_now_controlling_target = nullptr;
            this->RemoveLightSource(m_controlling_light->GetLight());
            objectManager.RemoveObjectByID(m_light_oid);
            m_light_oid = 0;
        }

    }

    lastRightState = rightState;
    lastLeftState = leftState;
    lastMidState = midState;

    updateCurve();
}

void Window::cycleRenderModeForward() {
    switch (m_renderMode) {
        case RenderMode::VERTEX_SHADER_RESULT:
            m_renderMode = RenderMode::RASTERIZED_RESULT;
            break;
        case RenderMode::RASTERIZED_RESULT:
            m_renderMode = RenderMode::FRAGMENT_SHADER_RESULT;
            break;
        case RenderMode::FRAGMENT_SHADER_RESULT:
            m_renderMode = RenderMode::FINAL_RESULT;
            break;
        case RenderMode::FINAL_RESULT:
            m_renderMode = RenderMode::VERTEX_SHADER_RESULT;
            break;
    }
    std::cout << "Render mode: " << static_cast<int>(m_renderMode) << std::endl;
}

void Window::cycleRenderModeBackward() {
    switch (m_renderMode) {
        case RenderMode::VERTEX_SHADER_RESULT:
            m_renderMode = RenderMode::FINAL_RESULT;
            break;
        case RenderMode::RASTERIZED_RESULT:
            m_renderMode = RenderMode::VERTEX_SHADER_RESULT;
            break;
        case RenderMode::FRAGMENT_SHADER_RESULT:
            m_renderMode = RenderMode::RASTERIZED_RESULT;
            break;
        case RenderMode::FINAL_RESULT:
            m_renderMode = RenderMode::FRAGMENT_SHADER_RESULT;
            break;
    }
    std::cout << "Render mode: " << static_cast<int>(m_renderMode) << std::endl;
}

void Window::drawCrosshair() {
    // 使用现代 OpenGL 绘制屏幕中心的十字准星（NDC 空间）
    if (!m_crossVAO || !m_crossVBO || !m_shader) return;

    // 计算像素偏移对应的 NDC 偏移
    float halfW = (float)m_width / 2.0f;
    float halfH = (float)m_height / 2.0f;
    float offsetPixels = 10.0f;
    float dx = (offsetPixels) / halfW;   // 转换到 NDC（[-1,1]）
    float dy = (offsetPixels) / halfH;

    // 中心在 NDC 上为 (0,0)
    float cx = 0.0f;
    float cy = 0.0f;

    // 四个顶点：水平两点、垂直两点
    float verts[4 * 9] = {
        // horiz left
        cx - dx, cy, 0.0f,   0.0f,0.0f,1.0f,   1.0f,1.0f,1.0f,
        // horiz right
        cx + dx, cy, 0.0f,   0.0f,0.0f,1.0f,   1.0f,1.0f,1.0f,
        // vert bottom
        cx, cy - dy, 0.0f,   0.0f,0.0f,1.0f,   1.0f,1.0f,1.0f,
        // vert top
        cx, cy + dy, 0.0f,   0.0f,0.0f,1.0f,   1.0f,1.0f,1.0f
    };

    // 禁用深度测试以确保在屏幕上方绘制
    GLboolean depthEnabled = glIsEnabled(GL_DEPTH_TEST);
    if (depthEnabled) glDisable(GL_DEPTH_TEST);

    // 临时保存并设置投影/view/model 为单位矩阵（因为顶点已为 NDC）
    int prevRenderMode = static_cast<int>(m_renderMode);
    m_shader->use();
    glm::mat4 identity(1.0f);
    m_shader->setMat4("projection", identity);
    m_shader->setMat4("view", identity);
    m_shader->setMat4("model", identity);
    m_shader->setInt("renderMode", static_cast<int>(RenderMode::VERTEX_SHADER_RESULT));

    // 上传顶点数据并绘制
    glBindVertexArray(m_crossVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_crossVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    glLineWidth(2.0f);
    // 绘制水平线（顶点0-1）
    glDrawArrays(GL_LINES, 0, 2);
    // 绘制垂直线（顶点2-3）
    glDrawArrays(GL_LINES, 2, 2);
    glBindVertexArray(0);

    // 恢复 renderMode 和深度测试状态
    m_shader->setInt("renderMode", prevRenderMode);
    if (depthEnabled) glEnable(GL_DEPTH_TEST);
}

std::shared_ptr<Object> Window::castRayAndDetectObjects() {
    if (!m_camera) return nullptr;

    // 计算屏幕中心点的标准化设备坐标 (NDC)
    float x = (2.0f * (m_width / 2)) / m_width - 1.0f;
    float y = 1.0f - (2.0f * (m_height / 2)) / m_height;
    
    // 获取投影和视图矩阵
    glm::mat4 projection = m_camera->getProjectionMatrix((float)m_width / (float)m_height);
    glm::mat4 view = m_camera->getViewMatrix();
    
    // 计算视图投影矩阵的逆矩阵
    glm::mat4 invVP = glm::inverse(projection * view);
    
    // 创建近平面和远平面的射线点
    glm::vec4 nearPoint = glm::vec4(x, y, -1.0f, 1.0f);
    glm::vec4 farPoint = glm::vec4(x, y, 1.0f, 1.0f);
    
    // 转换到世界坐标
    glm::vec4 nearPointWorld = invVP * nearPoint;
    glm::vec4 farPointWorld = invVP * farPoint;
    
    // 透视除法
    nearPointWorld /= nearPointWorld.w;
    farPointWorld /= farPointWorld.w;
    
    // 计算射线方向
    glm::vec3 rayOrigin = m_camera->Position;
    glm::vec3 rayDirection = glm::normalize(glm::vec3(farPointWorld) - glm::vec3(nearPointWorld));
    
    // std::cout << "Casting ray from: (" << rayOrigin.x << ", " << rayOrigin.y << ", " << rayOrigin.z << ")" << std::endl;
    // std::cout << "Ray direction: (" << rayDirection.x << ", " << rayDirection.y << ", " << rayDirection.z << ")" << std::endl;
    
    auto hitObject = objectManager.RayCast(rayOrigin, rayDirection, 1000.0f);
    
    if (hitObject) {
        // auto pos = hitObject->GetPosition();
        // std::string pos_str = "(" + std::to_string(pos[0]) + ", " + std::to_string(pos[1]) + ", " + std::to_string(pos[2]) + ")";
        // std::cout << "Hit object: " << hitObject->GetName() << " at ID: " << hitObject->GetID()
        //     << " and pos: " << pos_str << std::endl;
        return hitObject;
    } else {
        // std::cout << "No object hit within 1000.0 units" << std::endl;
        return nullptr;
    }
}

void Window::updateCurveVisibility(bool showBezier, bool showBSpline, bool showNURBS) {
    auto setVis = [&](const std::string& name, bool visible) {
        auto obj = objectManager.GetObjectByName(name);
        if (obj) obj->SetVisible(visible);
    };

    setVis("bezier_curve", showBezier);
    setVis("bspline_curve", showBSpline);
    setVis("nurbs_curve", showNURBS);

    auto pieces = objectManager.GetObjectsByTag("nurbs_bezier_piece");
    for (auto& obj : pieces) {
        obj->SetVisible(showNURBS);
    }
}

void Window::updateCurve() {
    // collect control points (sorted by creation order)
    auto ctrlObjs = objectManager.GetObjectsByTag("bezier_control");
    std::sort(ctrlObjs.begin(), ctrlObjs.end(), [](const std::shared_ptr<Object>& a, const std::shared_ptr<Object>& b){ return a->GetID() < b->GetID(); });
    std::vector<glm::vec3> pts;
    for (auto &c : ctrlObjs) pts.push_back(c->GetPosition());

    // update plain Bezier curve
    auto bezierObj = objectManager.GetObjectByName("bezier_curve");
    if (!bezierObj) {
        auto bezierShape = std::make_shared<BezierCurve>(glm::vec3(1.0f, 1.0f, 0.0f));
        bezierShape->SetResolution(96);
        auto bezierSceneObj = std::make_shared<GeometryObject>(glm::vec3(0.0f), bezierShape);
        bezierSceneObj->SetName("bezier_curve");
        bezierSceneObj->SetVisible(true);
        bezierSceneObj->SetActive(true);
        objectManager.AddObject(bezierSceneObj);
        bezierObj = bezierSceneObj;
    }

    if (bezierObj && bezierObj->HaveShape()) {
        auto bezierShape = std::dynamic_pointer_cast<BezierCurve>(bezierObj->GetShape());
        if (bezierShape) {
            bezierShape->SetControlPoints(pts);
        }
    }

    // Update control lines between adjacent control points
    auto controlLinesObj = objectManager.GetObjectByName("control_lines");
    if (!controlLinesObj) {
        auto controlLinesShape = std::make_shared<ControlLines>(glm::vec3(0.5f, 0.5f, 0.5f));
        auto controlLinesSceneObj = std::make_shared<GeometryObject>(glm::vec3(0.0f), controlLinesShape);
        controlLinesSceneObj->SetName("control_lines");
        controlLinesSceneObj->SetVisible(true);
        controlLinesSceneObj->SetActive(true);
        objectManager.AddObject(controlLinesSceneObj);
        controlLinesObj = controlLinesSceneObj;
    }

    if (controlLinesObj && controlLinesObj->HaveShape()) {
        auto controlLinesShape = std::dynamic_pointer_cast<ControlLines>(controlLinesObj->GetShape());
        if (controlLinesShape) {
            controlLinesShape->SetControlPoints(pts);
        }
    }

    // rebuild NURBS / B-Spline and Bezier decomposition
    int maxDegree = std::max(1, static_cast<int>(pts.size()) - 1);
    int degreeUsed = std::clamp(m_nurbsDegree, 1, maxDegree);
    m_currentDegree = degreeUsed;
    std::vector<float> uniformWeights(pts.size(), 1.0f);

    auto nurbsShape = updateNurbsShape("nurbs_curve", glm::vec3(1.0f, 0.3f, 0.8f), uniformWeights);
    updateNurbsShape("bspline_curve", glm::vec3(0.2f, 0.9f, 1.0f), uniformWeights);

    auto pieces = nurbsShape ? nurbsShape->ToBezierControlPolygons() : std::vector<std::vector<glm::vec3>>{};
    updateBezierPieces(pieces);

    bool showBezier = (m_curveDisplay == CurveDisplayMode::BEZIER);
    bool showBSpline = (m_curveDisplay == CurveDisplayMode::BSPLINE);
    bool showNURBS = (m_curveDisplay == CurveDisplayMode::NURBS);
    updateCurveVisibility(showBezier, showBSpline, showNURBS);
}

std::shared_ptr<NURBSCurve> Window::updateNurbsShape(const std::string& name, const glm::vec3& color, const std::vector<float>& weights) {
    auto ctrlObjs = objectManager.GetObjectsByTag("bezier_control");
    std::sort(ctrlObjs.begin(), ctrlObjs.end(), [](const std::shared_ptr<Object>& a, const std::shared_ptr<Object>& b){ return a->GetID() < b->GetID(); });
    std::vector<glm::vec3> pts;
    for (auto &c : ctrlObjs) pts.push_back(c->GetPosition());
    
    int maxDegree = std::max(1, static_cast<int>(pts.size()) - 1);
    int degreeUsed = std::clamp(m_nurbsDegree, 1, maxDegree);
    
    auto obj = objectManager.GetObjectByName(name);
    std::shared_ptr<NURBSCurve> curveShape = nullptr;
    if (!obj) {
        auto curve = std::make_shared<NURBSCurve>(degreeUsed, color);
        curve->SetResolution(96);
        auto curveObj = std::make_shared<GeometryObject>(glm::vec3(0.0f), curve);
        curveObj->SetName(name);
        curveObj->SetVisible(true);
        curveObj->SetActive(true);
        objectManager.AddObject(curveObj);
        obj = curveObj;
    }

    if (obj && obj->HaveShape()) {
        curveShape = std::dynamic_pointer_cast<NURBSCurve>(obj->GetShape());
        if (curveShape) {
            curveShape->SetResolution(96);
            curveShape->SetDegree(degreeUsed);
            if (!weights.empty()) curveShape->SetWeights(weights);
            curveShape->SetControlPoints(pts);
        }
    }
    return curveShape;
}

void Window::updateBezierPieces(const std::vector<std::vector<glm::vec3>>& pieces) {
    auto tagged = objectManager.GetObjectsByTag("nurbs_bezier_piece");
    for (auto& obj : tagged) {
        const std::string& nm = obj->GetName();
        std::size_t pos = nm.rfind('_');
        bool remove = false;
        if (pos != std::string::npos) {
            try {
                int idx = std::stoi(nm.substr(pos + 1));
                if (idx >= static_cast<int>(pieces.size())) remove = true;
            } catch (...) {
                remove = true;
            }
        }
        if (remove) {
            objectManager.RemoveObjectByID(obj->GetID());
        }
    }

    for (size_t i = 0; i < pieces.size(); ++i) {
        std::string name = "nurbs_bezier_piece_" + std::to_string(i);
        auto obj = objectManager.GetObjectByName(name);
        if (!obj) {
            auto segShape = std::make_shared<BezierCurve>(glm::vec3(0.3f, 1.0f, 0.3f));
            segShape->SetResolution(48);
            auto segObj = std::make_shared<GeometryObject>(glm::vec3(0.0f), segShape);
            segObj->SetName(name);
            segObj->SetVisible(true);
            segObj->SetActive(true);
            objectManager.AddObject(segObj);
            objectManager.TagObject(segObj->GetID(), "nurbs_bezier_piece");
            obj = segObj;
        }
        if (obj->HaveShape()) {
            auto segShape = std::dynamic_pointer_cast<BezierCurve>(obj->GetShape());
            if (segShape) {
                segShape->SetControlPoints(pieces[i]);
            }
        }
    }
}
