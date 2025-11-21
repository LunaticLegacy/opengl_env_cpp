#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "shapes.hpp"

// BezierCurve: 根据控制点生成线状曲线（可视化为线段）
class BezierCurve : public ColoredShape {
public:
    BezierCurve(const glm::vec3& color = glm::vec3(1.0f, 1.0f, 0.0f));
    ~BezierCurve();

    // 设置控制点并重建 VBO
    void SetControlPoints(const std::vector<glm::vec3>& controlPoints);

    // 采样分辨率（默认为 64）
    void SetResolution(int samples) { m_samples = std::max(2, samples); Rebuild(); }

    // 重新构建顶点缓冲
    void Rebuild();

    // draw 实现：绘制为线条
    virtual void draw(Shader& shader) override;

private:
    std::vector<glm::vec3> m_controlPoints;
    std::vector<float> m_vertices; // 每顶点: position(3) normal(3) color(3)
    GLuint VAO, VBO;
    int m_samples;
};

// ControlLines: 在相邻控制点之间绘制直线连接
class ControlLines : public ColoredShape {
public:
    ControlLines(const glm::vec3& color = glm::vec3(0.5f, 0.5f, 0.5f));
    ~ControlLines();

    // 设置控制点并重建 VBO
    void SetControlPoints(const std::vector<glm::vec3>& controlPoints);

    // 重新构建顶点缓冲
    void Rebuild();

    // draw 实现：绘制为线条
    virtual void draw(Shader& shader) override;

private:
    std::vector<glm::vec3> m_controlPoints;
    std::vector<float> m_vertices; // 每顶点: position(3) normal(3) color(3)
    GLuint VAO, VBO;
};