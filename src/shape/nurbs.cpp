#include "shape/nurbs.hpp"

#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.hpp"

namespace {
    constexpr float kEpsilon = 1e-5f;
}

NURBSCurve::NURBSCurve(int degree, const glm::vec3& color)
    : ColoredShape(color),
      m_degree(std::max(1, degree)),
      m_samples(96),
      VAO(0),
      VBO(0) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
}

NURBSCurve::~NURBSCurve() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
}

void NURBSCurve::SetDegree(int degree) {
    m_degree = std::max(1, degree);
    Rebuild();
}

void NURBSCurve::SetControlPoints(const std::vector<glm::vec3>& controlPoints) {
    m_controlPoints = controlPoints;
    if (m_weights.size() != m_controlPoints.size()) {
        m_weights.assign(m_controlPoints.size(), 1.0f);
    }
    Rebuild();
}

void NURBSCurve::SetWeights(const std::vector<float>& weights) {
    m_weights.assign(m_controlPoints.size(), 1.0f);
    for (size_t i = 0; i < m_controlPoints.size() && i < weights.size(); ++i) {
        m_weights[i] = weights[i];
    }
    Rebuild();
}

void NURBSCurve::SetKnotVector(const std::vector<float>& knotVector) {
    m_knots = knotVector;
    if (!std::is_sorted(m_knots.begin(), m_knots.end())) {
        std::sort(m_knots.begin(), m_knots.end());
    }
    Rebuild();
}

void NURBSCurve::SetResolution(int samples) {
    m_samples = std::max(2, samples);
    Rebuild();
}

bool NURBSCurve::canEvaluate() const {
    return m_controlPoints.size() >= static_cast<size_t>(m_degree + 1);
}

void NURBSCurve::ensureKnotVector() {
    int n = static_cast<int>(m_controlPoints.size());
    int needed = n + m_degree + 1;
    if (needed <= 0) return;
    if ((int)m_knots.size() == needed) return;

    m_knots.clear();
    m_knots.reserve(needed);
    for (int i = 0; i < needed; ++i) {
        if (i <= m_degree) {
            m_knots.push_back(0.0f);
        } else if (i >= n) {
            m_knots.push_back(1.0f);
        } else {
            float value = static_cast<float>(i - m_degree) / static_cast<float>(n - m_degree);
            m_knots.push_back(value);
        }
    }
}

int NURBSCurve::findSpan(int n, float u, const std::vector<float>& knots) const {
    if (u >= knots[n + 1]) return n;
    if (u <= knots[m_degree]) return m_degree;

    int low = m_degree;
    int high = n + 1;
    int mid = (low + high) / 2;

    while (u < knots[mid] || u >= knots[mid + 1]) {
        if (u < knots[mid]) {
            high = mid;
        } else {
            low = mid;
        }
        mid = (low + high) / 2;
    }
    return mid;
}

std::vector<float> NURBSCurve::basisFunctions(int span, float u, const std::vector<float>& knots) const {
    std::vector<float> N(static_cast<size_t>(m_degree + 1), 0.0f);
    std::vector<float> left(static_cast<size_t>(m_degree + 1), 0.0f);
    std::vector<float> right(static_cast<size_t>(m_degree + 1), 0.0f);

    N[0] = 1.0f;
    for (int j = 1; j <= m_degree; ++j) {
        left[j] = u - knots[span + 1 - j];
        right[j] = knots[span + j] - u;
        float saved = 0.0f;
        for (int r = 0; r < j; ++r) {
            float denom = right[r + 1] + left[j - r];
            float temp = denom != 0.0f ? N[r] / denom : 0.0f;
            N[r] = saved + right[r + 1] * temp;
            saved = left[j - r] * temp;
        }
        N[j] = saved;
    }
    return N;
}

int NURBSCurve::knotMultiplicity(float knot, const std::vector<float>& knots) const {
    int count = 0;
    for (float k : knots) {
        if (std::abs(k - knot) < kEpsilon) {
            ++count;
        }
    }
    return count;
}

glm::vec3 NURBSCurve::evaluatePoint(float u, const std::vector<float>* weightsOverride) const {
    if (!canEvaluate()) return glm::vec3(0.0f);

    int n = static_cast<int>(m_controlPoints.size()) - 1;
    auto knots = m_knots;
    if (static_cast<int>(knots.size()) != static_cast<int>(m_controlPoints.size()) + m_degree + 1) {
        // fall back to generated knots
        knots.clear();
        int needed = static_cast<int>(m_controlPoints.size()) + m_degree + 1;
        for (int i = 0; i < needed; ++i) {
            if (i <= m_degree) knots.push_back(0.0f);
            else if (i >= static_cast<int>(m_controlPoints.size())) knots.push_back(1.0f);
            else knots.push_back(static_cast<float>(i - m_degree) / static_cast<float>(static_cast<int>(m_controlPoints.size()) - m_degree));
        }
    }

    int span = findSpan(n, u, knots);
    auto N = basisFunctions(span, u, knots);

    glm::vec3 numerator(0.0f);
    float denominator = 0.0f;
    for (int j = 0; j <= m_degree; ++j) {
        int idx = span - m_degree + j;
        if (idx < 0 || idx >= static_cast<int>(m_controlPoints.size())) continue;
        float w = (weightsOverride && idx < static_cast<int>(weightsOverride->size()))
                    ? (*weightsOverride)[idx]
                    : (idx < static_cast<int>(m_weights.size()) ? m_weights[idx] : 1.0f);
        float coeff = N[j] * w;
        numerator += coeff * m_controlPoints[idx];
        denominator += coeff;
    }

    if (std::abs(denominator) < kEpsilon) return glm::vec3(0.0f);
    return numerator / denominator;
}

void NURBSCurve::Rebuild() {
    m_vertices.clear();
    if (!canEvaluate()) return;

    ensureKnotVector();
    if (m_knots.empty()) return;

    float uStart = m_knots[m_degree];
    float uEnd = m_knots[m_knots.size() - m_degree - 1];
    if (uEnd - uStart < kEpsilon) return;

    for (int i = 0; i <= m_samples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(m_samples);
        float u = uStart + (uEnd - uStart) * t;
        // 避免落在末端重复节点产生的 0/0
        if (i == m_samples) u = uEnd - kEpsilon;
        glm::vec3 p = evaluatePoint(u);

        m_vertices.push_back(p.x);
        m_vertices.push_back(p.y);
        m_vertices.push_back(p.z);
        m_vertices.push_back(0.0f);
        m_vertices.push_back(0.0f);
        m_vertices.push_back(1.0f);
        m_vertices.push_back(m_color.r);
        m_vertices.push_back(m_color.g);
        m_vertices.push_back(m_color.b);
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(float), m_vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);
}

std::vector<glm::vec3> NURBSCurve::SampleBSpline(int samples) const {
    std::vector<glm::vec3> result;
    if (!canEvaluate()) return result;
    int sampleCount = std::max(2, samples);

    std::vector<float> unitWeights(m_controlPoints.size(), 1.0f);
    std::vector<float> knots = m_knots;
    if (static_cast<int>(knots.size()) != static_cast<int>(m_controlPoints.size()) + m_degree + 1) {
        // generate open uniform
        knots.clear();
        int needed = static_cast<int>(m_controlPoints.size()) + m_degree + 1;
        for (int i = 0; i < needed; ++i) {
            if (i <= m_degree) knots.push_back(0.0f);
            else if (i >= static_cast<int>(m_controlPoints.size())) knots.push_back(1.0f);
            else knots.push_back(static_cast<float>(i - m_degree) / static_cast<float>(static_cast<int>(m_controlPoints.size()) - m_degree));
        }
    }

    float uStart = knots[m_degree];
    float uEnd = knots[knots.size() - m_degree - 1];
    if (uEnd - uStart < kEpsilon) return result;

    for (int i = 0; i <= sampleCount; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(sampleCount);
        float u = uStart + (uEnd - uStart) * t;
        if (i == sampleCount) u = uEnd - kEpsilon;
        result.push_back(evaluatePoint(u, &unitWeights));
    }
    return result;
}

void NURBSCurve::insertKnotOnce(float u, std::vector<float>& knots, std::vector<glm::vec4>& homoPts) const {
    int p = m_degree;
    int n = static_cast<int>(homoPts.size()) - 1;
    int m = n + p + 1;
    if (p <= 0 || n < p) return;

    int k = findSpan(n, u, knots);
    int s = knotMultiplicity(u, knots);

    std::vector<float> UQ(knots.size() + 1);
    std::vector<glm::vec4> Q(homoPts.size() + 1);

    for (int i = 0; i <= k; ++i) UQ[i] = knots[i];
    UQ[k + 1] = u;
    for (int i = k + 1; i <= m; ++i) UQ[i + 1] = knots[i];

    for (int i = 0; i <= k - p; ++i) Q[i] = homoPts[i];
    for (int i = k - s; i <= n; ++i) Q[i + 1] = homoPts[i];

    std::vector<glm::vec4> R(static_cast<size_t>(p - s + 1));
    for (int i = 0; i <= p - s; ++i) {
        R[i] = homoPts[k - p + i];
    }

    // r = 1 insertion
    int L = k - p + 1;
    for (int i = 0; i <= p - 1 - s; ++i) {
        float alpha = (u - knots[L + i]) / (knots[i + k + 1] - knots[L + i]);
        R[i] = alpha * R[i + 1] + (1.0f - alpha) * R[i];
    }
    Q[L] = R[0];
    Q[k - s] = R[p - 1 - s];

    if (L + 1 <= k - s - 1) {
        for (int i = L + 1; i <= k - s - 1; ++i) {
            Q[i] = R[i - L];
        }
    }

    knots.swap(UQ);
    homoPts.swap(Q);
}

std::vector<std::vector<glm::vec3>> NURBSCurve::ToBezierControlPolygons() const {
    std::vector<std::vector<glm::vec3>> result;
    if (!canEvaluate()) return result;

    int p = m_degree;
    int n = static_cast<int>(m_controlPoints.size());
    if (n < p + 1) return result;

    std::vector<float> knots = m_knots;
    int needed = n + p + 1;
    if (static_cast<int>(knots.size()) != needed) {
        knots.clear();
        for (int i = 0; i < needed; ++i) {
            if (i <= p) knots.push_back(0.0f);
            else if (i >= n) knots.push_back(1.0f);
            else knots.push_back(static_cast<float>(i - p) / static_cast<float>(n - p));
        }
    }

    std::vector<glm::vec4> homoPts(static_cast<size_t>(n));
    for (int i = 0; i < n; ++i) {
        float w = (i < static_cast<int>(m_weights.size())) ? m_weights[i] : 1.0f;
        if (std::abs(w) < kEpsilon) w = kEpsilon;
        homoPts[i] = glm::vec4(m_controlPoints[i] * w, w);
    }

    // 提升每个内部节点的重数到 p+1
    for (size_t idx = p + 1; idx < knots.size() - p - 1; /* 手动控制 */) {
        float u = knots[idx];
        int mult = knotMultiplicity(u, knots);
        int neededInsert = (p + 1) - mult;
        if (neededInsert > 0) {
            for (int r = 0; r < neededInsert; ++r) {
                insertKnotOnce(u, knots, homoPts);
            }
            // 插入后，当前位置的 knot 值还相同，重新计数
            continue;
        } else {
            idx += mult;
        }
    }

    // 计算片段数量（非零跨度数量）
    int m = static_cast<int>(knots.size()) - 1;
    int spanCount = 0;
    for (int i = p; i < m - p; ++i) {
        if (std::abs(knots[i + 1] - knots[i]) > kEpsilon) ++spanCount;
    }
    if (spanCount == 0) return result;

    result.reserve(static_cast<size_t>(spanCount));
    int offset = 0;
    for (int s = 0; s < spanCount; ++s) {
        std::vector<glm::vec3> segment;
        segment.reserve(static_cast<size_t>(p + 1));
        for (int j = 0; j <= p; ++j) {
            glm::vec4 pw = homoPts[offset + j];
            float w = std::abs(pw.w) < kEpsilon ? kEpsilon : pw.w;
            segment.emplace_back(pw.x / w, pw.y / w, pw.z / w);
        }
        result.push_back(std::move(segment));
        offset += p;
    }
    return result;
}

void NURBSCurve::draw(Shader& shader) {
    if (m_vertices.empty()) return;
    glm::mat4 model = getModelMatrix();
    shader.setMat4("model", model);

    glBindVertexArray(VAO);
    GLsizei count = static_cast<GLsizei>(m_vertices.size() / 9);
    glDrawArrays(GL_LINE_STRIP, 0, count);
    glBindVertexArray(0);
}
