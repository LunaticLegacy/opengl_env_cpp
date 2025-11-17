#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <optional>
#include <iostream>

#include "shape/shapes.hpp"
#include "shape/objects.hpp"
#include "shader.hpp"
#include "light.hpp"

// ============================================================================
// Object 类实现
// ============================================================================

uint64_t Object::s_nextID = 0;

Object::Object() 
    : m_id(s_nextID++), 
      m_name("Object_" + std::to_string(m_id)),
      m_pos(0.0f, 0.0f, 0.0f),
      m_rotation(0.0f, 0.0f, 0.0f),
      m_scale(1.0f, 1.0f, 1.0f),
      m_shape(nullptr),
      m_boundingSphereRadius(0.5f),
      m_active(true),
      m_visible(true) 
{
}

Object::Object(const glm::vec3& pos) 
    : m_id(s_nextID++),
      m_name("Object_" + std::to_string(m_id)),
      m_pos(pos),
      m_rotation(0.0f, 0.0f, 0.0f),
      m_scale(1.0f, 1.0f, 1.0f),
      m_shape(nullptr),
      m_boundingSphereRadius(0.5f),
      m_active(true),
      m_visible(true)
{
}

Object::Object(const glm::vec3& pos, std::shared_ptr<ColoredShape> shape) 
    : m_id(s_nextID++),
      m_name("Object_" + std::to_string(m_id)),
      m_pos(pos),
      m_rotation(0.0f, 0.0f, 0.0f),
      m_scale(1.0f, 1.0f, 1.0f),
      m_shape(shape),
      m_boundingSphereRadius(0.5f),
      m_active(true),
      m_visible(true)
{
    if (m_shape) {
        SyncShapeTransform();
    }
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
    
    // 应用旋转（欧拉角：Yaw-Pitch-Roll）
    model = glm::rotate(model, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)); // Yaw
    model = glm::rotate(model, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f)); // Pitch
    model = glm::rotate(model, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f)); // Roll
    
    // 应用缩放
    model = glm::scale(model, m_scale);
    
    return model;
}

void Object::Draw(Shader& shader) {
    if (!m_active || !m_visible || !m_shape) return;
    
    // 同步形状的变换
    SyncShapeTransform();
    
    // 绘制形状
    m_shape->draw(shader);
}

void Object::SyncShapeTransform() {
    if (!m_shape) return;
    
    m_shape->setPosition(m_pos);
    m_shape->setRotation(m_rotation);
    m_shape->setScale(m_scale);
}

// ============================================================================
// GeometryObject 类实现
// ============================================================================

GeometryObject::GeometryObject(const glm::vec3& pos, std::shared_ptr<ColoredShape> shape)
    : Object(pos, shape)
{
    m_name = "GeometryObject_" + std::to_string(m_id);
}

// ============================================================================
// Bezier3DObject 类实现
// ============================================================================

Bezier3DObject::Bezier3DObject(const glm::vec3& pos, std::shared_ptr<ColoredShape> shape)
    : Object(pos, shape), m_animationTime(0.0f)
{
    m_name = "Bezier3DObject_" + std::to_string(m_id);
}

void Bezier3DObject::SetControlPoints(const std::vector<glm::vec3>& controlPoints) {
    m_controlPoints = controlPoints;
}

void Bezier3DObject::Update(float deltaTime) {
    // 更新动画时间
    m_animationTime += deltaTime;
    if (m_animationTime > 1.0f) {
        m_animationTime -= 1.0f;
    }

    // 根据贝塞尔曲线计算新位置
    if (m_controlPoints.size() >= 2) {
        // 简单的二阶贝塞尔曲线插值
        float t = m_animationTime;
        glm::vec3 p0 = m_controlPoints[0];
        glm::vec3 p1 = m_controlPoints.size() > 1 ? m_controlPoints[1] : glm::vec3(0.0f);
        glm::vec3 p2 = m_controlPoints.size() > 2 ? m_controlPoints[2] : p1;

        // 二阶贝塞尔公式: B(t) = (1-t)^2*P0 + 2(1-t)t*P1 + t^2*P2
        float oneMinusT = 1.0f - t;
        glm::vec3 newPos = 
            oneMinusT * oneMinusT * p0 + 
            2.0f * oneMinusT * t * p1 + 
            t * t * p2;

        SetPosition(newPos);
    }
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
