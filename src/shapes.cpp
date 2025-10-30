#include "shapes.hpp"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>

// Shape implementation
/**
 * @brief 基础构造函数，初始化变换为单位变换
 */
Shape::Shape() : m_position(0.0f, 0.0f, 0.0f), m_rotation(0.0f, 0.0f, 0.0f), m_scale(1.0f, 1.0f, 1.0f) {
}

void Shape::setPosition(const glm::vec3& position) {
    m_position = position;
}

void Shape::move(const glm::vec3& offset) {
    m_position += offset;
}

void Shape::setRotation(const glm::vec3& rotation) {
    m_rotation = rotation;
}

void Shape::setScale(const glm::vec3& scale) {
    m_scale = scale;
}

glm::mat4 Shape::getModelMatrix() const {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, m_position);
    model = glm::rotate(model, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, m_scale);
    return model;
}

// ColoredShape implementation
ColoredShape::ColoredShape(const glm::vec3& color) : m_color(color) {
}

void ColoredShape::setColor(const glm::vec3& color) {
    m_color = color;
}

glm::vec3 ColoredShape::getColor() const {
    return m_color;
}

// Point implementation
Point::Point(float x, float y, float z, const glm::vec3& color) : ColoredShape(color), position(x, y, z) {
    // 生成并绑定VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // 生成并绑定VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    // 准备顶点数据（位置+颜色）
    float pointData[6] = {
        position.x, position.y, position.z,
        m_color.r, m_color.g, m_color.b
    };
    
    // 填充VBO数据
    glBufferData(GL_ARRAY_BUFFER, sizeof(pointData), pointData, GL_STATIC_DRAW);
    
    // 设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // 解绑VAO
    glBindVertexArray(0);
}

/**
 * @brief 绘制单个点
 * @param shader 当前激活的着色器引用。函数会使用 shader.setMat4("model", ...) 和 shader.setVec3("color", ...)
 *               因此传入的着色器需要定义对应 uniform（model, color）。
 */
void Point::draw(Shader& shader) {
    // 上传 model
    glm::mat4 model = getModelMatrix();
    shader.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, 1);
    glBindVertexArray(0);
}

Point::~Point() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

// Point2D implementation
Point2D::Point2D(float x, float y, const glm::vec3& color) : Point(x, y, 0.0f, color) {}

// Point3D implementation
Point3D::Point3D(float x, float y, float z, const glm::vec3& color) : Point(x, y, z, color) {}

// Line implementation
Line::Line(float startX, float startY, float startZ,
           float endX, float endY, float endZ,
           const glm::vec3& color)
    : ColoredShape(color), startPoint(startX, startY, startZ), endPoint(endX, endY, endZ) {
    
    // 生成并绑定VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // 生成并绑定VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    // 准备顶点数据（位置+颜色）
    float lineData[12] = {
        startPoint.x, startPoint.y, startPoint.z,
        m_color.r, m_color.g, m_color.b,
        endPoint.x, endPoint.y, endPoint.z,
        m_color.r, m_color.g, m_color.b
    };
    
    // 填充VBO数据
    glBufferData(GL_ARRAY_BUFFER, sizeof(lineData), lineData, GL_STATIC_DRAW);
    
    // 设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // 解绑VAO
    glBindVertexArray(0);
}

/**
 * @brief 绘制线段
 * @param shader 着色器引用，会上传 model 矩阵与 color
 */
void Line::draw(Shader& shader) {
    glm::mat4 model = getModelMatrix();
    shader.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);
}

Line::~Line() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

// Triangle implementation
Triangle::Triangle(float x1, float y1, float z1,
                   float x2, float y2, float z2,
                   float x3, float y3, float z3,
                   const glm::vec3& color) : ColoredShape(color) 
{
    vertices[0] = glm::vec3(x1, y1, z1);
    vertices[1] = glm::vec3(x2, y2, z2);
    vertices[2] = glm::vec3(x3, y3, z3);
    
    // 生成并绑定VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // 生成并绑定VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    // 准备顶点数据（位置+颜色）
    float triangleData[18];
    for (int i = 0; i < 3; i++) {
        triangleData[i*6]     = vertices[i].x;
        triangleData[i*6 + 1] = vertices[i].y;
        triangleData[i*6 + 2] = vertices[i].z;
        triangleData[i*6 + 3] = m_color.r;
        triangleData[i*6 + 4] = m_color.g;
        triangleData[i*6 + 5] = m_color.b;
    }
    
    // 填充VBO数据
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleData), triangleData, GL_STATIC_DRAW);
    
    // 设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // 解绑VAO
    glBindVertexArray(0);
}

/**
 * @brief 构造三角形
 * @param p1..p3 三个顶点位置，基于现有的点
 * @param color 三角形颜色
 */
Triangle::Triangle(
    Point&& p1, Point&& p2, Point&& p3, 
    const glm::vec3& color) : ColoredShape(color) 
{
    this->vertices[0] = p1.getPosition();
    this->vertices[1] = p2.getPosition();
    this->vertices[2] = p3.getPosition();

    // 生成并绑定VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // 生成并绑定VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    // 准备顶点数据（位置+颜色）
    float triangleData[18];
    for (int i = 0; i < 3; i++) {
        triangleData[i*6]     = vertices[i].x;
        triangleData[i*6 + 1] = vertices[i].y;
        triangleData[i*6 + 2] = vertices[i].z;
        triangleData[i*6 + 3] = m_color.r;
        triangleData[i*6 + 4] = m_color.g;
        triangleData[i*6 + 5] = m_color.b;
    }
    
    // 填充VBO数据
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleData), triangleData, GL_STATIC_DRAW);
    
    // 设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // 解绑VAO
    glBindVertexArray(0);
}

/**
 * @brief 绘制三角形
 * @param shader 着色器引用，会上传 model 矩阵与 color
 */
void Triangle::draw(Shader& shader) {
    glm::mat4 model = getModelMatrix();
    shader.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

Triangle::~Triangle() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

// Quad implementation
Quad::Quad(float x1, float y1, float z1,
           float x2, float y2, float z2,
           float x3, float y3, float z3,
           float x4, float y4, float z4,
           const glm::vec3& color) : ColoredShape(color) {
    vertices[0] = glm::vec3(x1, y1, z1);
    vertices[1] = glm::vec3(x2, y2, z2);
    vertices[2] = glm::vec3(x3, y3, z3);
    vertices[3] = glm::vec3(x4, y4, z4);
    
    // 生成并绑定VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // 生成并绑定VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    // 准备顶点数据（位置+颜色）
    float quadData[24];
    for (int i = 0; i < 4; i++) {
        quadData[i*6]     = vertices[i].x;
        quadData[i*6 + 1] = vertices[i].y;
        quadData[i*6 + 2] = vertices[i].z;
        quadData[i*6 + 3] = m_color.r;
        quadData[i*6 + 4] = m_color.g;
        quadData[i*6 + 5] = m_color.b;
    }
    
    // 填充VBO数据
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadData), quadData, GL_STATIC_DRAW);
    
    // 设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // 解绑VAO
    glBindVertexArray(0);
}

Quad::Quad(Point p1, Point p2, Point p3, Point p4,
         const glm::vec3& color) : ColoredShape(color)
{
    vertices[0] = p1.getPosition();
    vertices[1] = p2.getPosition();
    vertices[2] = p3.getPosition();
    vertices[3] = p4.getPosition();
    
    // 生成并绑定VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // 生成并绑定VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    // 准备顶点数据（位置+颜色）
    float quadData[24];
    for (int i = 0; i < 4; i++) {
        quadData[i*6]     = vertices[i].x;
        quadData[i*6 + 1] = vertices[i].y;
        quadData[i*6 + 2] = vertices[i].z;
        quadData[i*6 + 3] = m_color.r;
        quadData[i*6 + 4] = m_color.g;
        quadData[i*6 + 5] = m_color.b;
    }
    
    // 填充VBO数据
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadData), quadData, GL_STATIC_DRAW);
    
    // 设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // 解绑VAO
    glBindVertexArray(0);
}

/**
 * @brief 绘制四边形（使用 GL_QUADS）
 * @param shader 着色器引用，会上传 model 矩阵与 color
 */
void Quad::draw(Shader& shader) {
    glm::mat4 model = getModelMatrix();
    shader.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawArrays(GL_QUADS, 0, 4);
    glBindVertexArray(0);
}

Quad::~Quad() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

// Cube implementation
Cube::Cube(float size, const glm::vec3& color) : ColoredShape(color) {
    float halfSize = size / 2.0f;
    
    // 立方体顶点数据（位置+法线）
    float cubeVertices[] = {
        // 前面
        -halfSize, -halfSize,  halfSize,  0.0f,  0.0f,  1.0f,  m_color.r, m_color.g, m_color.b,
         halfSize, -halfSize,  halfSize,  0.0f,  0.0f,  1.0f,  m_color.r, m_color.g, m_color.b,
         halfSize,  halfSize,  halfSize,  0.0f,  0.0f,  1.0f,  m_color.r, m_color.g, m_color.b,
         halfSize,  halfSize,  halfSize,  0.0f,  0.0f,  1.0f,  m_color.r, m_color.g, m_color.b,
        -halfSize,  halfSize,  halfSize,  0.0f,  0.0f,  1.0f,  m_color.r, m_color.g, m_color.b,
        -halfSize, -halfSize,  halfSize,  0.0f,  0.0f,  1.0f,  m_color.r, m_color.g, m_color.b,

        // 左面
        -halfSize, -halfSize, -halfSize, -1.0f,  0.0f,  0.0f,  m_color.r, m_color.g, m_color.b,
        -halfSize, -halfSize,  halfSize, -1.0f,  0.0f,  0.0f,  m_color.r, m_color.g, m_color.b,
        -halfSize,  halfSize,  halfSize, -1.0f,  0.0f,  0.0f,  m_color.r, m_color.g, m_color.b,
        -halfSize,  halfSize,  halfSize, -1.0f,  0.0f,  0.0f,  m_color.r, m_color.g, m_color.b,
        -halfSize,  halfSize, -halfSize, -1.0f,  0.0f,  0.0f,  m_color.r, m_color.g, m_color.b,
        -halfSize, -halfSize, -halfSize, -1.0f,  0.0f,  0.0f,  m_color.r, m_color.g, m_color.b,

        // 后面
         halfSize, -halfSize, -halfSize,  0.0f,  0.0f, -1.0f,  m_color.r, m_color.g, m_color.b,
        -halfSize, -halfSize, -halfSize,  0.0f,  0.0f, -1.0f,  m_color.r, m_color.g, m_color.b,
        -halfSize,  halfSize, -halfSize,  0.0f,  0.0f, -1.0f,  m_color.r, m_color.g, m_color.b,
        -halfSize,  halfSize, -halfSize,  0.0f,  0.0f, -1.0f,  m_color.r, m_color.g, m_color.b,
         halfSize,  halfSize, -halfSize,  0.0f,  0.0f, -1.0f,  m_color.r, m_color.g, m_color.b,
         halfSize, -halfSize, -halfSize,  0.0f,  0.0f, -1.0f,  m_color.r, m_color.g, m_color.b,

        // 右面
         halfSize, -halfSize,  halfSize,  1.0f,  0.0f,  0.0f,  m_color.r, m_color.g, m_color.b,
         halfSize, -halfSize, -halfSize,  1.0f,  0.0f,  0.0f,  m_color.r, m_color.g, m_color.b,
         halfSize,  halfSize, -halfSize,  1.0f,  0.0f,  0.0f,  m_color.r, m_color.g, m_color.b,
         halfSize,  halfSize, -halfSize,  1.0f,  0.0f,  0.0f,  m_color.r, m_color.g, m_color.b,
         halfSize,  halfSize,  halfSize,  1.0f,  0.0f,  0.0f,  m_color.r, m_color.g, m_color.b,
         halfSize, -halfSize,  halfSize,  1.0f,  0.0f,  0.0f,  m_color.r, m_color.g, m_color.b,

        // 上面
        -halfSize,  halfSize,  halfSize,  0.0f,  1.0f,  0.0f,  m_color.r, m_color.g, m_color.b,
         halfSize,  halfSize,  halfSize,  0.0f,  1.0f,  0.0f,  m_color.r, m_color.g, m_color.b,
         halfSize,  halfSize, -halfSize,  0.0f,  1.0f,  0.0f,  m_color.r, m_color.g, m_color.b,
         halfSize,  halfSize, -halfSize,  0.0f,  1.0f,  0.0f,  m_color.r, m_color.g, m_color.b,
        -halfSize,  halfSize, -halfSize,  0.0f,  1.0f,  0.0f,  m_color.r, m_color.g, m_color.b,
        -halfSize,  halfSize,  halfSize,  0.0f,  1.0f,  0.0f,  m_color.r, m_color.g, m_color.b,

        // 下面
        -halfSize, -halfSize, -halfSize,  0.0f, -1.0f,  0.0f,  m_color.r, m_color.g, m_color.b,
         halfSize, -halfSize, -halfSize,  0.0f, -1.0f,  0.0f,  m_color.r, m_color.g, m_color.b,
         halfSize, -halfSize,  halfSize,  0.0f, -1.0f,  0.0f,  m_color.r, m_color.g, m_color.b,
         halfSize, -halfSize,  halfSize,  0.0f, -1.0f,  0.0f,  m_color.r, m_color.g, m_color.b,
        -halfSize, -halfSize,  halfSize,  0.0f, -1.0f,  0.0f,  m_color.r, m_color.g, m_color.b,
        -halfSize, -halfSize, -halfSize,  0.0f, -1.0f,  0.0f,  m_color.r, m_color.g, m_color.b
    };

    // 生成并绑定VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // 生成并绑定VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // 填充VBO数据
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // 设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // 解绑VAO
    glBindVertexArray(0);
}

/**
 * @brief 绘制立方体网格（三角形面）
 * @param shader 着色器引用，会上传 model 矩阵与 color
 */
void Cube::draw(Shader& shader) {
    glm::mat4 model = getModelMatrix();
    shader.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

/**
 * @brief 移动立方体到指定坐标。
 * @param pos 坐标位置。
 */
/**
 * @brief 改变立方体的姿态。
 * @param pose 姿态。
 */
void Cube::transpose(const glm::vec3& pose) {

}

Cube::~Cube() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

// Sphere implementation
Sphere::Sphere(float radius, int sectors, int stacks, const glm::vec3& color) 
    : ColoredShape(color), sectorCount(sectors), stackCount(stacks) {
    
    float sectorStep = 2 * M_PI / sectorCount;
    float stackStep = M_PI / stackCount;
    
    for (int i = 0; i <= stackCount; ++i) {
        float stackAngle = M_PI / 2 - i * stackStep;  // 从 pi/2 到 -pi/2
        float xy = radius * cosf(stackAngle);         // r * cos(u)
        float z = radius * sinf(stackAngle);          // r * sin(u)

        // 通过扇区添加 (sectorCount+1) 个顶点
        for (int j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * sectorStep;       // 从 0 到 2pi

            // 顶点位置 (x, y, z)
            float x = xy * cosf(sectorAngle);         // r * cos(u) * cos(v)
            float y = xy * sinf(sectorAngle);         // r * cos(u) * sin(v)
            
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            
            // 法线向量 (标准化的顶点位置)
            glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);
            
            // 颜色
            vertices.push_back(m_color.r);
            vertices.push_back(m_color.g);
            vertices.push_back(m_color.b);
        }
    }

    // 生成索引
    for (int i = 0; i < stackCount; ++i) {
        int k1 = i * (sectorCount + 1);     // 当前堆栈的起始索引
        int k2 = k1 + sectorCount + 1;      // 下一堆栈的起始索引

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            // 2个三角形构成一个四边形
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stackCount - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    // 生成并绑定VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // 生成并绑定VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    // 生成并绑定EBO
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // 设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // 解绑VAO
    glBindVertexArray(0);
}

/**
 * @brief 绘制球体（使用索引绘制）
 * @param shader 着色器引用，会上传 model 矩阵与 color
 */
void Sphere::draw(Shader& shader) {
    glm::mat4 model = getModelMatrix();
    shader.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, (unsigned int)indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

Sphere::~Sphere() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}