#pragma once
#include <GL/glew.h>
#include <string>
#include <glm/glm.hpp>

class Shader {
public:
    // 程序ID
    unsigned int ID;

    // 构造器读取并构建着色器
    Shader(const char* vertexPath, const char* fragmentPath);

    // 使用/激活程序
    void use();

    // uniform工具函数
    void setFloat(const std::string &name, float value) const;
    void setInt(const std::string &name, int value) const;
    void setBool(const std::string &name, bool value) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setVec4(const std::string &name, const glm::vec4 &value) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;

private:
    // 检查着色器错误
    void checkCompileErrors(unsigned int shader, std::string type);
};