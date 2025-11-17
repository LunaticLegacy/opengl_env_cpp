#include "shape/object_manager.hpp"
#include "shape/objects.hpp"
#include "shader.hpp"
#include "basic/glwindow.hpp"
#include "basic/camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/norm.hpp>

#include <iostream>
#include <algorithm>

#define GLM_ENABLE_EXPERIMENTAL

// ============================================================================
// ObjectManager 实现
// ============================================================================

void ObjectManager::AddObject(std::shared_ptr<Object> obj) {
    if (!obj) return;
    m_objects.push_back(obj);
}

bool ObjectManager::RemoveObjectByID(uint64_t id) {
    auto it = std::find_if(m_objects.begin(), m_objects.end(), 
        [id](const std::shared_ptr<Object>& obj) { return obj->GetID() == id; });
    
    if (it != m_objects.end()) {
        m_objectTags.erase(id);
        m_objects.erase(it);
        return true;
    }
    return false;
}

bool ObjectManager::RemoveObjectByName(const std::string& name) {
    auto it = std::find_if(m_objects.begin(), m_objects.end(),
        [&name](const std::shared_ptr<Object>& obj) { return obj->GetName() == name; });
    
    if (it != m_objects.end()) {
        m_objectTags.erase((*it)->GetID());
        m_objects.erase(it);
        return true;
    }
    return false;
}

void ObjectManager::RemoveAllObjects() {
    m_objects.clear();
    m_objectTags.clear();
}

std::shared_ptr<Object> ObjectManager::GetObjectByID(uint64_t id) const {
    auto it = std::find_if(m_objects.begin(), m_objects.end(),
        [id](const std::shared_ptr<Object>& obj) { return obj->GetID() == id; });
    
    return (it != m_objects.end()) ? *it : nullptr;
}

std::shared_ptr<Object> ObjectManager::GetObjectByName(const std::string& name) const {
    auto it = std::find_if(m_objects.begin(), m_objects.end(),
        [&name](const std::shared_ptr<Object>& obj) { return obj->GetName() == name; });
    
    return (it != m_objects.end()) ? *it : nullptr;
}

void ObjectManager::UpdateAll(float deltaTime) {
    for (auto& obj : m_objects) {
        if (obj->IsActive()) {
            obj->Update(deltaTime);
        }
    }
}

void ObjectManager::DrawAll(Shader& shader) {
    for (auto& obj : m_objects) {
        if (obj->IsActive() && obj->IsVisible()) {
            obj->Draw(shader);
        }
    }
}

std::vector<std::shared_ptr<Object>> ObjectManager::GetObjectsByTypeName(const std::string& typeName) const {
    std::vector<std::shared_ptr<Object>> result;
    for (const auto& obj : m_objects) {
        if (obj->GetTypeName() == typeName) {
            result.push_back(obj);
        }
    }
    return result;
}

std::vector<std::shared_ptr<Object>> ObjectManager::GetActiveObjects() const {
    std::vector<std::shared_ptr<Object>> result;
    for (const auto& obj : m_objects) {
        if (obj->IsActive()) {
            result.push_back(obj);
        }
    }
    return result;
}

std::vector<std::shared_ptr<Object>> ObjectManager::GetVisibleObjects() const {
    std::vector<std::shared_ptr<Object>> result;
    for (const auto& obj : m_objects) {
        if (obj->IsVisible()) {
            result.push_back(obj);
        }
    }
    return result;
}

std::shared_ptr<Object> ObjectManager::RayCast(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, float maxDistance) {
    float closestDistance = maxDistance;
    std::shared_ptr<Object> closestObject = nullptr;

    for (auto& obj : m_objects) {
        if (!obj->IsActive() || !obj->IsVisible()) continue;

        glm::vec3 sphereCenter = obj->GetBoundingSphereCentre();
        float sphereRadius = obj->GetBoundingSphereRadius();

        float distance;
        if (glm::intersectRaySphere(rayOrigin, rayDirection, sphereCenter, sphereRadius, distance)) {
            if (distance > 0.0f && distance < closestDistance) {
                closestDistance = distance;
                closestObject = obj;
            }
        }
    }

    return closestObject;
}

std::vector<std::shared_ptr<Object>> ObjectManager::GetVisibleObjectsInFrustum(const Camera& camera, float aspectRatio) {
    std::vector<std::shared_ptr<Object>> result;
    
    glm::mat4 projection = camera.getProjectionMatrix(aspectRatio);
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 vpMatrix = projection * view;

    for (auto& obj : m_objects) {
        if (!obj->IsActive() || !obj->IsVisible()) continue;

        glm::vec3 objPos = obj->GetBoundingSphereCentre();
        float radius = obj->GetBoundingSphereRadius();

        // 简单的包围球与视锥体测试
        glm::vec4 posHomogeneous = vpMatrix * glm::vec4(objPos, 1.0f);
        glm::vec3 ndc = glm::vec3(posHomogeneous) / posHomogeneous.w;

        // 检查是否在NDC空间内（加上一个缓冲区）
        if (ndc.x >= -1.0f - radius && ndc.x <= 1.0f + radius &&
            ndc.y >= -1.0f - radius && ndc.y <= 1.0f + radius &&
            ndc.z >= -1.0f - radius && ndc.z <= 1.0f + radius) {
            result.push_back(obj);
        }
    }

    return result;
}

std::vector<std::shared_ptr<Object>> ObjectManager::GetObjectsInRadius(const glm::vec3& center, float radius) {
    std::vector<std::shared_ptr<Object>> result;
    float radiusSq = radius * radius;

    for (auto& obj : m_objects) {
        if (!obj->IsActive()) continue;

        glm::vec3 objPos = obj->GetBoundingSphereCentre();
        float distSq = glm::distance2(objPos, center);

        if (distSq <= radiusSq) {
            result.push_back(obj);
        }
    }

    return result;
}

bool ObjectManager::CheckCollision(const Object& obj1, const Object& obj2) {
    glm::vec3 center1 = obj1.GetBoundingSphereCentre();
    glm::vec3 center2 = obj2.GetBoundingSphereCentre();
    float radius1 = obj1.GetBoundingSphereRadius();
    float radius2 = obj2.GetBoundingSphereRadius();

    float distance = glm::length(center2 - center1);
    return distance <= (radius1 + radius2);
}

std::vector<std::shared_ptr<Object>> ObjectManager::CheckCollisionsWithObject(const Object& obj) {
    std::vector<std::shared_ptr<Object>> collisions;

    for (auto& other : m_objects) {
        if (other->GetID() == obj.GetID()) continue;
        if (!other->IsActive()) continue;

        if (CheckCollision(obj, *other)) {
            collisions.push_back(other);
        }
    }

    return collisions;
}

void ObjectManager::TagObject(uint64_t id, const std::string& tag) {
    auto it = std::find_if(m_objects.begin(), m_objects.end(),
        [id](const std::shared_ptr<Object>& obj) { return obj->GetID() == id; });
    
    if (it != m_objects.end()) {
        auto& tags = m_objectTags[id];
        auto tagIt = std::find(tags.begin(), tags.end(), tag);
        if (tagIt == tags.end()) {
            tags.push_back(tag);
        }
    }
}

void ObjectManager::UntagObject(uint64_t id, const std::string& tag) {
    auto tagMapIt = m_objectTags.find(id);
    if (tagMapIt != m_objectTags.end()) {
        auto& tags = tagMapIt->second;
        auto it = std::find(tags.begin(), tags.end(), tag);
        if (it != tags.end()) {
            tags.erase(it);
        }
    }
}

std::vector<std::shared_ptr<Object>> ObjectManager::GetObjectsByTag(const std::string& tag) {
    std::vector<std::shared_ptr<Object>> result;

    for (auto& tagEntry : m_objectTags) {
        auto& tags = tagEntry.second;
        auto it = std::find(tags.begin(), tags.end(), tag);
        if (it != tags.end()) {
            auto obj = GetObjectByID(tagEntry.first);
            if (obj) {
                result.push_back(obj);
            }
        }
    }

    return result;
}

void ObjectManager::PrintDebugInfo() const {
    std::cout << "=== Object Manager Debug Info ===" << std::endl;
    std::cout << "Total objects: " << m_objects.size() << std::endl;

    for (const auto& obj : m_objects) {
        std::cout << "  - " << obj->GetName() 
                  << " (ID: " << obj->GetID() 
                  << ", Type: " << obj->GetTypeName()
                  << ", Active: " << (obj->IsActive() ? "Yes" : "No")
                  << ", Visible: " << (obj->IsVisible() ? "Yes" : "No") << ")" << std::endl;
    }
}

ObjectManager::SceneStats ObjectManager::GetSceneStats() const {
    SceneStats stats = {};
    stats.totalObjects = m_objects.size();

    for (const auto& obj : m_objects) {
        if (obj->IsActive()) stats.activeObjects++;
        if (obj->IsVisible()) stats.visibleObjects++;

        if (obj->GetTypeName() == "GeometryObject") {
            stats.geometryObjects++;
        } else if (obj->GetTypeName() == "Bezier3DObject") {
            stats.bezierObjects++;
        } else if (obj->GetTypeName() == "LightObject") {
            stats.lightObjects++;
        }
    }

    return stats;
}

void ObjectManager::ClearAllTags() {
    m_objectTags.clear();
}

bool ObjectManager::IsPointInFrustum(const glm::vec3& point, const Camera& camera, float aspectRatio) {
    glm::mat4 projection = camera.getProjectionMatrix(aspectRatio);
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 vpMatrix = projection * view;

    glm::vec4 posHomogeneous = vpMatrix * glm::vec4(point, 1.0f);
    glm::vec3 ndc = glm::vec3(posHomogeneous) / posHomogeneous.w;

    return ndc.x >= -1.0f && ndc.x <= 1.0f &&
           ndc.y >= -1.0f && ndc.y <= 1.0f &&
           ndc.z >= -1.0f && ndc.z <= 1.0f;
}

std::vector<ObjectManager::FrustumPlane> ObjectManager::ExtractFrustumPlanes(const glm::mat4& vpMatrix) {
    std::vector<FrustumPlane> planes;

    // 这是一个简化的实现，实际的视锥体提取会更复杂
    // 但对于基本的剔除已经足够

    return planes;
}
