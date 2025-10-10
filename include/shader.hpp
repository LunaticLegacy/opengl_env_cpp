#pragma once
#include <GL/glew.h>
#include <string>
#include <glm/glm.hpp>

/**
 * @brief 简单的着色器封装：从顶点/片段着色器文件构建 program，并提供常用 uniform 上传方法
 */
class Shader {
public:
    // 程序 ID（OpenGL shader program）
    unsigned int ID;

    /**
     * @brief 构造 Shader
     * @param vertexPath 顶点着色器文件路径（相对于运行目录或工程路径）
     * @param fragmentPath 片段着色器文件路径
     *
     * 该构造器会读取文件、编译顶点/片段着色器并链接为 program，若失败会打印错误信息。
     */
    Shader(const char* vertexPath, const char* fragmentPath);

    /**
     * @brief 激活/使用该着色器程序（会调用 glUseProgram(ID)）
     */
    void use();

    /**
     * @brief 设置 float 类型 uniform
     * @param name uniform 名称
     * @param value 要设置的浮点值
     */
    void setFloat(const std::string &name, float value) const;

    /**
     * @brief 设置 vec3 类型 uniform
     * @param name uniform 名称
     * @param value 三维向量 (x,y,z)
     */
    void setVec3(const std::string &name, const glm::vec3 &value) const;

    /**
     * @brief 设置 mat4 类型 uniform
     * @param name uniform 名称
     * @param mat 4x4 矩阵（按列主序传递给 OpenGL）
     */
    void setMat4(const std::string &name, const glm::mat4 &mat) const;

private:
    /**
     * @brief 编译或链接出错时打印详细信息
     * @param shader 着色器/程序 ID
     * @param type 错误类型标识（"VERTEX", "FRAGMENT", "PROGRAM"）
     */
    void checkCompileErrors(unsigned int shader, std::string type);
};