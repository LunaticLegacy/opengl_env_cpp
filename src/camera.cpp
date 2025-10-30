#include "camera.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

Camera::Camera(const glm::vec3& position, const glm::vec3& up, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(2.5f), MouseSensitivity(0.1f), Zoom(45.0f),
      NearPlane(0.1f), FarPlane(100.0f), firstMouse(true), lastX(400.0), lastY(300.0)
{
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    invertX = false;
    invertY = false;
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const {
    // 使用 lookAt（方向由 Front 给出）
    return glm::lookAt(Position, Position + Front, Up);
}

glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const {
    return glm::perspective(glm::radians(Zoom), aspectRatio, NearPlane, FarPlane);
}

void Camera::processKeyboard(Camera_Movement direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    switch (direction)
    {
    case FORWARD:
        Position += Front * velocity;
        break;
    case BACKWARD:
        Position -= Front * velocity;
        break;
    case LEFT:
        Position -= Right * velocity;
        break;
    case RIGHT:
        Position += Right * velocity;
        break;
    case UP:
        Position[1] += velocity;
        break;
    case DOWN:
        Position[1] -= velocity;
        break;
    default:
        break;
    }        
}

void Camera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    if (invertX) xoffset = -xoffset;
    if (invertY) yoffset = -yoffset;

    Yaw += xoffset;
    Pitch += yoffset;

    // 限制俯仰角，避免翻转
    if (constrainPitch) {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    updateCameraVectors();
}

void Camera::processMouseCallback(double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = (float)(xpos - lastX);
    float yoffset = (float)(lastY - ypos); // 反转 y
    lastX = xpos;
    lastY = ypos;

    processMouseMovement(xoffset, yoffset);
}

void Camera::setPerspective(float fovDegrees, float nearPlane, float farPlane) {
    Zoom = fovDegrees;
    NearPlane = nearPlane;
    FarPlane = farPlane;
}

void Camera::setInvertX(bool invert) {
    invertX = invert;
}

void Camera::setInvertY(bool invert) {
    invertY = invert;
}

void Camera::toggleInvertX() {
    invertX = !invertX;
}

void Camera::toggleInvertY() {
    invertY = !invertY;
}

bool Camera::isInvertX() const { return invertX; }

bool Camera::isInvertY() const { return invertY; }

void Camera::updateCameraVectors() {
    // 使用四元数从 yaw/pitch 构造方向向量
    // 先构造欧拉角对应的四元数
    glm::quat qPitch = glm::angleAxis(glm::radians(Pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat qYaw = glm::angleAxis(glm::radians(Yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat orientation = qYaw * qPitch;

    // 将初始前向向量 (0,0,-1) 旋转到新的方向
    Front = glm::normalize(orientation * glm::vec3(0.0f, 0.0f, -1.0f));
    // 右向量和上向量
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}