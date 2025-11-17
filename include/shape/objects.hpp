#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <optional>
#include <memory>
#include <string>
#include <functional>

#include "shape/shapes.hpp"

// ============================================================================
// 对象基类 - 所有可绘制的游戏对象的基类
// ============================================================================
class Object {
public:
    /**
     * @brief 默认构造函数
     */
    Object();

    /**
     * @brief 根据位置构造对象
     * @param pos 初始位置
     */
    explicit Object(const glm::vec3& pos);

    /**
     * @brief 根据位置和形状构造对象
     * @param pos 初始位置
     * @param shape 关联的形状
     */
    Object(const glm::vec3& pos, std::shared_ptr<ColoredShape> shape);

    /**
     * @brief 虚析构函数
     */
    virtual ~Object() = default;

    // -------- 基本属性 --------
    
    /**
     * @brief 获取对象ID
     */
    uint64_t GetID() const { return m_id; }

    /**
     * @brief 设置对象名称
     */
    void SetName(const std::string& name) { m_name = name; }

    /**
     * @brief 获取对象名称
     */
    const std::string& GetName() const { return m_name; }

    /**
     * @brief 检查是否有关联的形状
     */
    bool HaveShape() const { return m_shape != nullptr; }

    /**
     * @brief 获取关联的形状
     */
    std::shared_ptr<ColoredShape> GetShape() const { return m_shape; }

    /**
     * @brief 设置关联的形状
     */
    void SetShape(std::shared_ptr<ColoredShape> shape) { m_shape = shape; }

    // -------- 位置和变换 --------
    
    /**
     * @brief 获取对象位置（世界坐标）
     */
    glm::vec3 GetPosition() const { return m_pos; }

    /**
     * @brief 设置对象位置
     */
    void SetPosition(const glm::vec3& pos);

    /**
     * @brief 相对移动对象
     */
    void Move(const glm::vec3& offset) { SetPosition(m_pos + offset); }

    /**
     * @brief 获取对象旋转（欧拉角，度数）
     */
    glm::vec3 GetRotation() const { return m_rotation; }

    /**
     * @brief 设置对象旋转（欧拉角，度数）
     */
    void SetRotation(const glm::vec3& rotation);

    /**
     * @brief 获取对象缩放
     */
    glm::vec3 GetScale() const { return m_scale; }

    /**
     * @brief 设置对象缩放
     */
    void SetScale(const glm::vec3& scale);

    /**
     * @brief 获取模型矩阵
     */
    glm::mat4 GetModelMatrix() const;

    // -------- 物理和碰撞 --------
    
    /**
     * @brief 设置包围球半径（用于碰撞检测和视锥体剔除）
     */
    void SetBoundingSphereRadius(float radius) { m_boundingSphereRadius = radius; }

    /**
     * @brief 获取包围球半径
     */
    float GetBoundingSphereRadius() const { return m_boundingSphereRadius; }

    /**
     * @brief 获取包围球中心（世界坐标）
     */
    glm::vec3 GetBoundingSphereCentre() const { return m_pos; }

    // -------- 活跃状态 --------
    
    /**
     * @brief 设置对象是否活跃（活跃对象会被绘制和更新）
     */
    void SetActive(bool active) { m_active = active; }

    /**
     * @brief 检查对象是否活跃
     */
    bool IsActive() const { return m_active; }

    /**
     * @brief 设置对象是否可见
     */
    void SetVisible(bool visible) { m_visible = visible; }

    /**
     * @brief 检查对象是否可见
     */
    bool IsVisible() const { return m_visible; }

    // -------- 生命周期 --------
    
    /**
     * @brief 更新对象（每帧调用）
     */
    virtual void Update(float deltaTime) {}

    /**
     * @brief 绘制对象
     */
    virtual void Draw(Shader& shader);

    /**
     * @brief 获取对象类型（用于RTTI）
     */
    virtual std::string GetTypeName() const { return "Object"; }

protected:
    static uint64_t s_nextID;
    uint64_t m_id;
    std::string m_name;

    // 位置和变换
    glm::vec3 m_pos;
    glm::vec3 m_rotation;   // 欧拉角：pitch, yaw, roll（度）
    glm::vec3 m_scale;

    // 形状
    std::shared_ptr<ColoredShape> m_shape;

    // 碰撞检测和视锥体剔除
    float m_boundingSphereRadius;

    // 状态
    bool m_active;
    bool m_visible;

    /**
     * @brief 同步形状的变换（用于内部管理）
     */
    void SyncShapeTransform();
};

// ============================================================================
// 几何对象 - 用于表示简单的几何形状
// ============================================================================

class GeometryObject : public Object {
public:
    /**
     * @brief 构造几何对象
     */
    GeometryObject(const glm::vec3& pos, std::shared_ptr<ColoredShape> shape);

    virtual ~GeometryObject() = default;

    virtual std::string GetTypeName() const override { return "GeometryObject"; }
};

// ============================================================================
// 3D贝塞尔曲线/曲面对象
// ============================================================================
class Bezier3DObject : public Object {
public:
    /**
     * @brief 构造贝塞尔3D对象
     */
    Bezier3DObject(const glm::vec3& pos, std::shared_ptr<ColoredShape> shape);

    /**
     * @brief 设置控制点
     */
    void SetControlPoints(const std::vector<glm::vec3>& controlPoints);

    /**
     * @brief 获取控制点
     */
    const std::vector<glm::vec3>& GetControlPoints() const { return m_controlPoints; }

    /**
     * @brief 更新贝塞尔曲线（参数 t 范围 [0,1]）
     */
    virtual void Update(float deltaTime) override;

    virtual std::string GetTypeName() const override { return "Bezier3DObject"; }

private:
    std::vector<glm::vec3> m_controlPoints;
    float m_animationTime;
};

// ============================================================================
// 灯光对象 - 用于表示光源
// ============================================================================

class LightObject : public Object {
public:
    /**
     * @brief 构造灯光对象
     */
    LightObject(const glm::vec3& pos, std::shared_ptr<class Light> light);

    /**
     * @brief 获取灯光
     */
    std::shared_ptr<class Light> GetLight() const { return m_light; }

    virtual void Draw(Shader& shader) override;

    virtual std::string GetTypeName() const override { return "LightObject"; }

private:
    std::shared_ptr<class Light> m_light;
};

