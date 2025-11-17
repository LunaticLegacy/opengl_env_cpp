#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

// 光源类型枚举
enum LightType {
    POINT_LIGHT = 0,
    DIRECTIONAL_LIGHT,
    SPOT_LIGHT
};

/**
 * @brief 光源基类
 */
class Light {
public:
    // 光源属性
    glm::vec3 position;     // 位置
    glm::vec3 direction;    // 方向（对于方向光和聚光灯）
    glm::vec3 ambient;      // 环境光强度
    glm::vec3 diffuse;      // 漫反射强度
    glm::vec3 specular;     // 镜面反射强度
    
    // 衰减参数（仅适用于点光源和聚光灯）
    float constant;         // 常数衰减因子
    float linear;           // 一次衰减因子
    float quadratic;        // 二次衰减因子
    
    // 聚光灯参数（仅适用于聚光灯）
    float cutOff;           // 内切光角（弧度）
    float outerCutOff;      // 外切光角（弧度）
    
    LightType type;         // 光源类型
    
    /**
     * @brief 构造函数
     * @param _type 光源类型
     * @param _position 光源位置
     * @param _direction 光源方向
     */
    Light(LightType _type, glm::vec3 _position = glm::vec3(0.0f), glm::vec3 _direction = glm::vec3(0.0f))
        : type(_type), position(_position), direction(_direction),
          ambient(glm::vec3(0.1f)), diffuse(glm::vec3(0.5f)), specular(glm::vec3(1.0f)),
          constant(1.0f), linear(0.09f), quadratic(0.032f),
          cutOff(glm::cos(glm::radians(12.5f))), outerCutOff(glm::cos(glm::radians(15.0f))) {}
    
    /**
     * @brief 设置光源在着色器中的uniform变量
     * @param shaderProgram 着色器程序ID
     * @param name 光源名称（例如"light[0]"或"dirLight"）
     */
    virtual void setUniform(unsigned int shaderProgram, const std::string& name) const;
    
    /**
     * @brief 设置光源类型为点光源
     */
    void setTypePoint();
    
    /**
     * @brief 设置光源类型为方向光
     */
    void setTypeDirectional();
    
    /**
     * @brief 设置光源类型为聚光灯
     */
    void setTypeSpot();
    
    /**
     * @brief 设置光源颜色
     * @param _ambient 环境光颜色
     * @param _diffuse 漫反射颜色
     * @param _specular 镜面反射颜色
     */
    void setColor(const glm::vec3& _ambient, const glm::vec3& _diffuse, const glm::vec3& _specular);
    
    /**
     * @brief 设置衰减参数
     * @param _constant 常数衰减因子
     * @param _linear 一次衰减因子
     * @param _quadratic 二次衰减因子
     */
    void setAttenuation(float _constant, float _linear, float _quadratic);
    
    /**
     * @brief 设置聚光灯角度
     * @param innerAngle 内切光角（角度）
     * @param outerAngle 外切光角（角度）
     */
    void setSpotAngle(float innerAngle, float outerAngle);

    /**
     * @brief 设置光源位置
     * @param _position 新位置
     */
    void SetPosition(const glm::vec3& _position) { position = _position; }


    /**
     * @brief 移动光源位置
     * @param _position 新位置
     */
    void Move(const glm::vec3& _vec) { SetPosition(position - _vec); }

    /**
     * @brief 获取光源位置
     */
    glm::vec3 GetPosition() const { return position; }

    /**
     * @brief 设置光源方向
     * @param _direction 新方向
     */
    void SetDirection(const glm::vec3& _direction) { direction = _direction; }

    /**
     * @brief 获取光源方向
     */
    glm::vec3 GetDirection() const { return direction; }
};