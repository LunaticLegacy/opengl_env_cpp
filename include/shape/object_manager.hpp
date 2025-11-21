#pragma once
#include <vector>
#include <memory>
#include <map>
#include <algorithm>
#include <stdexcept>

#include "objects.hpp"
#include "basic/camera.hpp"

#define GLM_ENABLE_EXPERIMENTAL

// ============================================================================
// 对象管理器 - 统一管理场景中的所有对象
// ============================================================================

class Window;

class ObjectManager {
public:
    /**
     * @brief 获取单例实例
     */
    static ObjectManager& GetInstance() {
        static ObjectManager instance;
        return instance;
    }

    // 删除复制构造函数和赋值操作符
    ObjectManager(const ObjectManager&) = delete;
    ObjectManager& operator=(const ObjectManager&) = delete;

    // -------- 对象管理 --------

    /**
     * @brief 添加对象到场景
     */
    void AddObject(std::shared_ptr<Object> obj);

    /**
     * @brief 根据ID移除对象
     */
    bool RemoveObjectByID(uint64_t id);

    /**
     * @brief 根据名称移除对象
     */
    bool RemoveObjectByName(const std::string& name);

    /**
     * @brief 移除所有对象
     */
    void RemoveAllObjects();

    /**
     * @brief 根据ID获取对象
     */
    std::shared_ptr<Object> GetObjectByID(uint64_t id) const;

    /**
     * @brief 根据名称获取对象
     */
    std::shared_ptr<Object> GetObjectByName(const std::string& name) const;

    /**
     * @brief 获取所有对象
     */
    const std::vector<std::shared_ptr<Object>>& GetAllObjects() const { return m_objects; }

    /**
     * @brief 获取对象总数
     */
    size_t GetObjectCount() const { return m_objects.size(); }

    // -------- 更新和绘制 --------

    /**
     * @brief 更新所有活跃对象
     */
    void UpdateAll(float deltaTime);

    /**
     * @brief 绘制所有可见对象
     */
    void DrawAll(Shader& shader);

    /**
     * @brief 绘制特定类型的对象
     */
    template <typename T>
    void DrawOfType(Shader& shader) {
        for (auto& obj : m_objects) {
            if (!obj->IsActive() || !obj->IsVisible()) continue;
            
            auto typedObj = std::dynamic_pointer_cast<T>(obj);
            if (typedObj) {
                typedObj->Draw(shader);
            }
        }
    }

    // -------- 查询和过滤 --------

    /**
     * @brief 获取特定类型的所有对象
     */
    template <typename T>
    std::vector<std::shared_ptr<T>> GetObjectsOfType() const {
        std::vector<std::shared_ptr<T>> result;
        for (const auto& obj : m_objects) {
            auto typedObj = std::dynamic_pointer_cast<T>(obj);
            if (typedObj) {
                result.push_back(typedObj);
            }
        }
        return result;
    }

    /**
     * @brief 获取特定类型名称的所有对象
     */
    std::vector<std::shared_ptr<Object>> GetObjectsByTypeName(const std::string& typeName) const;

    /**
     * @brief 获取活跃的对象列表
     */
    std::vector<std::shared_ptr<Object>> GetActiveObjects() const;

    /**
     * @brief 获取可见的对象列表
     */
    std::vector<std::shared_ptr<Object>> GetVisibleObjects() const;

    // -------- 空间查询 --------

    /**
     * @brief 检测点与对象的碰撞（射线检测）
     * @param rayOrigin 射线起点
     * @param rayDirection 射线方向（应归一化）
     * @param camera 用于计算射线的摄像机
     * @return 最近的被击中对象指针，如果没有则返回 nullptr
     */
    std::shared_ptr<Object> RayCast(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, float maxDistance = 1000.0f);

    /**
     * @brief 视锥体剔除 - 返回在摄像机视野内的对象
     * @param camera 摄像机
     * @return 在视野内的对象列表
     */
    std::vector<std::shared_ptr<Object>> GetVisibleObjectsInFrustum(const Camera& camera, float aspectRatio);

    /**
     * @brief 根据位置查找周围对象（范围内的对象）
     * @param center 搜索中心
     * @param radius 搜索半径
     * @return 范围内的对象列表
     */
    std::vector<std::shared_ptr<Object>> GetObjectsInRadius(const glm::vec3& center, float radius);

    /**
     * @brief 检测两个对象是否碰撞（基于包围球）
     */
    bool CheckCollision(const Object& obj1, const Object& obj2);

    /**
     * @brief 检测对象与场景中其他对象的碰撞
     */
    std::vector<std::shared_ptr<Object>> CheckCollisionsWithObject(const Object& obj);

    // -------- 分组和标记 --------

    /**
     * @brief 为对象添加标签
     */
    void TagObject(uint64_t id, const std::string& tag);

    /**
     * @brief 为对象移除标签
     */
    void UntagObject(uint64_t id, const std::string& tag);

    /**
     * @brief 获取具有特定标签的所有对象
     */
    std::vector<std::shared_ptr<Object>> GetObjectsByTag(const std::string& tag);

    // -------- 统计和调试 --------

    /**
     * @brief 打印所有对象的信息
     */
    void PrintDebugInfo() const;

    /**
     * @brief 获取场景统计信息
     */
    struct SceneStats {
        size_t totalObjects;
        size_t activeObjects;
        size_t visibleObjects;
        size_t geometryObjects;
        size_t bezierObjects;
        size_t lightObjects;
    };

    SceneStats GetSceneStats() const;

    /**
     * @brief 清除所有标签
     */
    void ClearAllTags();

private:
    ObjectManager() = default;
    ~ObjectManager() = default;

    std::vector<std::shared_ptr<Object>> m_objects;
    std::map<uint64_t, std::vector<std::string>> m_objectTags;

    /**
     * @brief 辅助函数：检查点是否在视锥体内
     */
    bool IsPointInFrustum(const glm::vec3& point, const Camera& camera, float aspectRatio);

    /**
     * @brief 辅助函数：计算视锥体平面
     */
    struct FrustumPlane {
        glm::vec3 normal;
        float distance;
    };

    std::vector<FrustumPlane> ExtractFrustumPlanes(const glm::mat4& vpMatrix);

    
};
