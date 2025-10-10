#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

class Camera {
public:
    Camera(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 3.0f),
           const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f, float pitch = 0.0f);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspectRatio) const;

    void processKeyboard(Camera_Movement direction, float deltaTime);
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

    void setPerspective(float fovDegrees, float nearPlane, float farPlane);

    // 可选：反转鼠标轴
    void setInvertX(bool invert);
    void setInvertY(bool invert);
    void toggleInvertX();
    void toggleInvertY();
    bool isInvertX() const;
    bool isInvertY() const;

    glm::vec3 Position;

private:
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // Euler angles
    float Yaw;
    float Pitch;

    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom; // fov

    // projection params
    float NearPlane;
    float FarPlane;

    void updateCameraVectors();
    bool invertX;
    bool invertY;
};
