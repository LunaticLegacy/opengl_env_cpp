#include <light/light.hpp>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

void Light::setUniform(unsigned int shaderProgram, const std::string& name) const {
    // 设置光源类型
    glUseProgram(shaderProgram);
    
    // 设置光源属性
    glUniform3fv(glGetUniformLocation(shaderProgram, (name + ".position").c_str()), 1, &position[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, (name + ".direction").c_str()), 1, &direction[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, (name + ".ambient").c_str()), 1, &ambient[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, (name + ".diffuse").c_str()), 1, &diffuse[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, (name + ".specular").c_str()), 1, &specular[0]);
    
    // 设置衰减参数
    glUniform1f(glGetUniformLocation(shaderProgram, (name + ".constant").c_str()), constant);
    glUniform1f(glGetUniformLocation(shaderProgram, (name + ".linear").c_str()), linear);
    glUniform1f(glGetUniformLocation(shaderProgram, (name + ".quadratic").c_str()), quadratic);
    
    // 设置聚光灯参数
    glUniform1f(glGetUniformLocation(shaderProgram, (name + ".cutOff").c_str()), cutOff);
    glUniform1f(glGetUniformLocation(shaderProgram, (name + ".outerCutOff").c_str()), outerCutOff);
    
    // 传递光源类型（作为整数）
    glUniform1i(glGetUniformLocation(shaderProgram, (name + ".type").c_str()), static_cast<int>(type));
}

void Light::setTypePoint() {
    type = POINT_LIGHT;
}

void Light::setTypeDirectional() {
    type = DIRECTIONAL_LIGHT;
    // 方向光的方向需要设置为非零向量
    if (direction == glm::vec3(0.0f)) {
        direction = glm::vec3(-0.2f, -1.0f, -0.3f);
    }
}

void Light::setTypeSpot() {
    type = SPOT_LIGHT;
    // 聚光灯需要设置位置和方向
    if (direction == glm::vec3(0.0f)) {
        direction = glm::vec3(0.0f, 0.0f, -1.0f);
    }
}

void Light::setColor(const glm::vec3& _ambient, const glm::vec3& _diffuse, const glm::vec3& _specular) {
    ambient = _ambient;
    diffuse = _diffuse;
    specular = _specular;
}

void Light::setAttenuation(float _constant, float _linear, float _quadratic) {
    constant = _constant;
    linear = _linear;
    quadratic = _quadratic;
}

void Light::setSpotAngle(float innerAngle, float outerAngle) {
    cutOff = glm::cos(glm::radians(innerAngle));
    outerCutOff = glm::cos(glm::radians(outerAngle));
}