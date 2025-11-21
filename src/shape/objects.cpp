#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <optional>
#include <iostream>

#include "shape/shapes.hpp"
#include "shape/objects.hpp"
#include "shape/bezier.hpp"
#include <GL/glew.h>
#include <iostream>

// ============================================================================
// 对象基类实现
// ============================================================================

// 下一个编号。
uint64_t Object::s_nextID = 0;

Object::Object()
    : m_id(s_nextID++), m_pos(0.0f), m_rotation(0.0f), m_scale(1.0f),
      m_boundingSphereRadius(1.0f), m_active(true), m_visible(true) {
    m_name = "Object_" + std::to_string(m_id);
}

Object::Object(const glm::vec3& pos)
    : m_id(s_nextID++), m_pos(pos), m_rotation(0.0f), m_scale(1.0f),
      m_boundingSphereRadius(1.0f), m_active(true), m_visible(true) {
    m_name = "Object_" + std::to_string(m_id);
}

Object::Object(const glm::vec3& pos, std::shared_ptr<ColoredShape> shape)
    : m_id(s_nextID++), m_pos(pos), m_rotation(0.0f), m_scale(1.0f), m_shape(shape),
      m_boundingSphereRadius(1.0f), m_active(true), m_visible(true) {
    m_name = "Object_" + std::to_string(m_id);
}

void Object::SetPosition(const glm::vec3& pos) {
    m_pos = pos;
    SyncShapeTransform();
}

void Object::SetRotation(const glm::vec3& rotation) {
    m_rotation = rotation;
    SyncShapeTransform();
}

void Object::SetScale(const glm::vec3& scale) {
    m_scale = scale;
    SyncShapeTransform();
}

glm::mat4 Object::GetModelMatrix() const {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, m_pos);
    model = glm::rotate(model, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, m_scale);
    return model;
}

void Object::SyncShapeTransform() {
    if (m_shape) {
        m_shape->setPosition(m_pos);
        m_shape->setRotation(m_rotation);
        m_shape->setScale(m_scale);
    }
}

void Object::Draw(Shader& shader) {
    if (!m_active || !m_visible || !m_shape) return;
    m_shape->draw(shader);
}

// ============================================================================
// GeometryObject 类实现
// ============================================================================

GeometryObject::GeometryObject(const glm::vec3& pos, std::shared_ptr<ColoredShape> shape)
    : Object(pos, shape) {
    m_name = "GeometryObject_" + std::to_string(m_id);
}

// ============================================================================
// Bezier3DObject 类实现
// ============================================================================

Bezier3DObject::Bezier3DObject(const glm::vec3& pos, std::shared_ptr<ColoredShape> shape)
    : Object(pos, shape), m_animationTime(0.0f) {
    m_name = "Bezier3DObject_" + std::to_string(m_id);
}

void Bezier3DObject::SetControlPoints(const std::vector<glm::vec3>& controlPoints) {
    m_controlPoints = controlPoints;
}

void Bezier3DObject::Update(float deltaTime) {
    m_animationTime += deltaTime;
    
    // 可以在这里添加动画逻辑
    // 例如，沿着贝塞尔曲线移动对象
}

// ============================================================================
// LightObject 类实现
// ============================================================================

LightObject::LightObject(const glm::vec3& pos, std::shared_ptr<Light> light)
    : Object(pos), m_light(light)
{
    m_name = "LightObject_" + std::to_string(m_id);
    if (m_light) {
        m_light->SetPosition(pos);
    }
}

void LightObject::Draw(Shader& shader) {
    if (!m_active || !m_visible) return;
    
    // 更新灯光位置
    if (m_light) {
        m_light->SetPosition(m_pos);
    }

    // 如果有关联的形状（灯光球体），绘制它
    Object::Draw(shader);
}

ControlLines::ControlLines(const glm::vec3& color)
    : ColoredShape(color), VAO(0), VBO(0) {
    // 初始化 GL 对象
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
}

ControlLines::~ControlLines() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
}

void ControlLines::SetControlPoints(const std::vector<glm::vec3>& controlPoints) {
    m_controlPoints = controlPoints;
    Rebuild();
}

void ControlLines::Rebuild() {
    m_vertices.clear();
    if (m_controlPoints.size() < 2) {
        // nothing to draw
        return;
    }

    // 为每对相邻的控制点创建一条线
    for (size_t i = 0; i < m_controlPoints.size() - 1; ++i) {
        const glm::vec3& p1 = m_controlPoints[i];
        const glm::vec3& p2 = m_controlPoints[i + 1];
        
        // 第一个点
        m_vertices.push_back(p1.x);
        m_vertices.push_back(p1.y);
        m_vertices.push_back(p1.z);
        // normal (unused for lines)
        m_vertices.push_back(0.0f);
        m_vertices.push_back(0.0f);
        m_vertices.push_back(1.0f);
        // color
        m_vertices.push_back(m_color.r);
        m_vertices.push_back(m_color.g);
        m_vertices.push_back(m_color.b);
        
        // 第二个点
        m_vertices.push_back(p2.x);
        m_vertices.push_back(p2.y);
        m_vertices.push_back(p2.z);
        // normal (unused for lines)
        m_vertices.push_back(0.0f);
        m_vertices.push_back(0.0f);
        m_vertices.push_back(1.0f);
        // color
        m_vertices.push_back(m_color.r);
        m_vertices.push_back(m_color.g);
        m_vertices.push_back(m_color.b);
    }

    // upload to GL
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(float), m_vertices.data(), GL_STATIC_DRAW);

    // attribute layout: location0 pos, location1 normal, location2 color
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);
}

void ControlLines::draw(Shader& shader) {
    if (m_vertices.empty()) return;
    glm::mat4 model = getModelMatrix();
    shader.setMat4("model", model);

    glBindVertexArray(VAO);
    GLsizei count = (GLsizei)(m_vertices.size() / 9);
    glDrawArrays(GL_LINES, 0, count);
    glBindVertexArray(0);
}
