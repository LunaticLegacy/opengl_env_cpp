#include "shape/bezier.hpp"
#include "shape/objects.hpp"
#include "shader.hpp"
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

static glm::vec3 evaluateBezier(const std::vector<glm::vec3>& P, float t) {
    // de Casteljau's algorithm (general n-degree Bezier)
    if (P.empty()) return glm::vec3(0.0f);
    std::vector<glm::vec3> tmp = P;
    int n = (int)tmp.size() - 1;
    for (int r = 1; r <= n; ++r) {
        for (int i = 0; i <= n - r; ++i) {
            tmp[i] = tmp[i] * (1.0f - t) + tmp[i + 1] * t;
        }
    }
    return tmp[0];
}

BezierCurve::BezierCurve(const glm::vec3& color)
    : ColoredShape(color), VAO(0), VBO(0), m_samples(64) {
    // 初始化 GL 对象
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
}

BezierCurve::~BezierCurve() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
}

void BezierCurve::SetControlPoints(const std::vector<glm::vec3>& controlPoints) {
    m_controlPoints = controlPoints;
    Rebuild();
}

void BezierCurve::Rebuild() {
    m_vertices.clear();
    if (m_controlPoints.size() < 2) {
        // nothing to draw
        return;
    }

    // 采样 m_samples 个点
    for (int i = 0; i <= m_samples; ++i) {
        float t = (float)i / (float)m_samples;
        glm::vec3 p = evaluateBezier(m_controlPoints, t);
        // position
        m_vertices.push_back(p.x);
        m_vertices.push_back(p.y);
        m_vertices.push_back(p.z);
        // normal (unused for lines)
        m_vertices.push_back(0.0f);
        m_vertices.push_back(0.0f);
        m_vertices.push_back(1.0f);
        // color
        m_vertices.push_back(m_color.r);
        m_vertices.push_back(m_color.g);
        m_vertices.push_back(m_color.b);
    }

    // upload to GL
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(float), m_vertices.data(), GL_STATIC_DRAW);

    // attribute layout: location0 pos, location1 normal, location2 color
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);
    // std::cout << "BezierCurve::Rebuild(): uploaded " << (m_vertices.size() / 9) << " vertices (samples=" << m_samples << ")" << std::endl;
}

void BezierCurve::draw(Shader& shader) {
    if (m_vertices.empty()) return;
    glm::mat4 model = getModelMatrix();
    shader.setMat4("model", model);

    glBindVertexArray(VAO);
    GLsizei count = (GLsizei)(m_vertices.size() / 9);
    // std::cout << "BezierCurve::draw(): drawing " << count << " vertices" << std::endl;
    glDrawArrays(GL_LINE_STRIP, 0, count);
    glBindVertexArray(0);
}