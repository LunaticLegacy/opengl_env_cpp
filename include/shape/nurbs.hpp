#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "shape/shapes.hpp"

/**
 * @brief 基于任意阶次的 NURBS 曲线
 *
 * 支持：
 *  - 任意阶次（degree）和自定义节点向量
 *  - 权重设置（默认全为 1，即退化为 B-Spline）
 *  - 将曲线拆分为等效的 Bézier 片段控制点
 */
class NURBSCurve : public ColoredShape {
public:
    explicit NURBSCurve(int degree = 3, const glm::vec3& color = glm::vec3(1.0f, 0.4f, 0.8f));
    ~NURBSCurve();

    // 基本设置
    void SetDegree(int degree);
    void SetControlPoints(const std::vector<glm::vec3>& controlPoints);
    void SetWeights(const std::vector<float>& weights);
    void SetKnotVector(const std::vector<float>& knotVector);
    void SetResolution(int samples);

    // 重新生成 VBO
    void Rebuild();

    // 采样：退化为 B-Spline（全部权重为 1）
    std::vector<glm::vec3> SampleBSpline(int samples) const;

    // 转换：拆分为等效的 Bézier 控制多边形（逐段）
    std::vector<std::vector<glm::vec3>> ToBezierControlPolygons() const;

    virtual void draw(Shader& shader) override;

private:
    bool canEvaluate() const;
    void ensureKnotVector();

    // 基础算法
    int findSpan(int n, float u, const std::vector<float>& knots) const;
    std::vector<float> basisFunctions(int span, float u, const std::vector<float>& knots) const;
    int knotMultiplicity(float knot, const std::vector<float>& knots) const;
    glm::vec3 evaluatePoint(float u, const std::vector<float>* weightsOverride = nullptr) const;

    // 拆分/插值相关
    void insertKnotOnce(float u, std::vector<float>& knots, std::vector<glm::vec4>& homoPts) const;

    std::vector<glm::vec3> m_controlPoints;
    std::vector<float> m_weights;
    std::vector<float> m_knots;
    int m_degree;
    int m_samples;

    GLuint VAO;
    GLuint VBO;
    std::vector<float> m_vertices;
};
