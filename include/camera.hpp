#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/**
 * @brief 摄像机移动方向枚举
 *
 * 用于 processKeyboard 的 direction 参数，表示摄像机在本地坐标系下的移动方向：
 * - FORWARD: 沿摄像机当前前向移动（正向）
 * - BACKWARD: 沿摄像机当前前向移动（反向）
 * - LEFT: 沿摄像机右向的反方向移动
 * - RIGHT: 沿摄像机右向移动
 */
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

/**
 * @brief 简单的四元数/欧拉混合摄像机类
 *
 * 说明：摄像机内部使用角度（Yaw, Pitch）计算方向，并使用四元数旋转基础向量生成 Front。
 * 提供：获取视图矩阵与投影矩阵、处理键盘/鼠标输入、以及可选的鼠标轴反转功能。
 */
class Camera {
public:
    /**
     * @brief 构造摄像机
     * @param position 摄像机世界空间位置
     * @param up 世界空间的上向量（通常为 (0,1,0)）
     * @param yaw 绕 Y 轴的偏航角（度），默认 -90° 使默认视向为 -Z
     * @param pitch 绕 X 轴的俯仰角（度）
     */
    Camera(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 3.0f),
           const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f, float pitch = 0.0f);

    /**
     * @brief 获取视图矩阵（传递给 shader 的 view uniform）
     * @return 4x4 视图矩阵
     */
    glm::mat4 getViewMatrix() const;

    /**
     * @brief 获取透视投影矩阵（传递给 shader 的 projection uniform）
     * @param aspectRatio 宽高比（width/height）
     * @return 4x4 投影矩阵
     */
    glm::mat4 getProjectionMatrix(float aspectRatio) const;

    /**
     * @brief 处理键盘输入以移动摄像机
     * @param direction 摄像机移动方向（FORWARD/BACKWARD/LEFT/RIGHT）
     * @param deltaTime 两帧之间的时间间隔（秒），用于按速度缩放移动量
     */
    void processKeyboard(Camera_Movement direction, float deltaTime);

    /**
     * @brief 处理鼠标移动以旋转摄像机
     * @param xoffset 鼠标 X 方向偏移，正值表示向右（像素或 GLFW 提供的单位）
     * @param yoffset 鼠标 Y 方向偏移，正值表示向上（像素或 GLFW 提供的单位）
     * @param constrainPitch 是否约束俯仰角以避免翻转（默认为 true）
     */
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

    /**
     * @brief 设置透视投影参数
     * @param fovDegrees 垂直视场角（度）
     * @param nearPlane 近裁剪面距离
     * @param farPlane 远裁剪面距离
     */
    void setPerspective(float fovDegrees, float nearPlane, float farPlane);

    /**
     * @brief 设置是否反转鼠标 X/Y 轴
     * @param invert 对应轴是否反向
     */
    void setInvertX(bool invert);
    void setInvertY(bool invert);
    void toggleInvertX();
    void toggleInvertY();
    bool isInvertX() const;
    bool isInvertY() const;

    // 摄像机位置（公有以便外部快速访问或修改）
    glm::vec3 Position;

private:
    glm::vec3 Front;   ///< 摄像机当前朝向
    glm::vec3 Up;      ///< 摄像机当前上向量
    glm::vec3 Right;   ///< 摄像机当前右向量
    glm::vec3 WorldUp; ///< 世界上向量（通常为 (0,1,0)）

    // 欧拉角
    float Yaw;   ///< 偏航角（度）
    float Pitch; ///< 俯仰角（度）

    // 摄像机选项
    float MovementSpeed;    ///< 平移速度（单位/秒）
    float MouseSensitivity; ///< 鼠标灵敏度（缩放偏移）
    float Zoom;             ///< 视场角（度）

    // 投影参数
    float NearPlane;
    float FarPlane;

    /**
     * @brief 根据当前 Yaw/Pitch 更新 Front/Right/Up 向量
     * 内部使用四元数对基础向量进行旋转以获得准确且连续的方向
     */
    void updateCameraVectors();

    // 鼠标轴反转标志
    bool invertX;
    bool invertY;
};
