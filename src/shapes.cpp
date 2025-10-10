#include "shapes.hpp"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <iostream>

// Point implementation
Point::Point(float x, float y, float z) : position(x, y, z) {
    // 生成并绑定VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // 生成并绑定VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    // 填充VBO数据
    glBufferData(GL_ARRAY_BUFFER, sizeof(position), &position, GL_STATIC_DRAW);
    
    // 设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 解绑VAO
    glBindVertexArray(0);
}

void Point::draw() {
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, 1);
    glBindVertexArray(0);
}

Point::~Point() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

// Point2D implementation
Point2D::Point2D(float x, float y) : Point(x, y, 0.0f) {}

// Point3D implementation
Point3D::Point3D(float x, float y, float z) : Point(x, y, z) {}

// Line implementation
Line::Line(float startX, float startY, float startZ,
           float endX, float endY, float endZ)
    : startPoint(startX, startY, startZ),
      endPoint(endX, endY, endZ) {
    
    // 生成并绑定VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // 生成并绑定VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    // 准备顶点数据
    glm::vec3 vertices[2] = {startPoint, endPoint};
    
    // 填充VBO数据
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // 设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 解绑VAO
    glBindVertexArray(0);
}

void Line::draw() {
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
                   float x3, float y3, float z3) {
    vertices[0] = glm::vec3(x1, y1, z1);
    vertices[1] = glm::vec3(x2, y2, z2);
    vertices[2] = glm::vec3(x3, y3, z3);
    
    // 生成并绑定VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // 生成并绑定VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    // 填充VBO数据
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // 设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 解绑VAO
    glBindVertexArray(0);
}

void Triangle::draw() {
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
           float x4, float y4, float z4) {
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
    
    // 填充VBO数据
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // 设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 解绑VAO
    glBindVertexArray(0);
}

void Quad::draw() {
    glBindVertexArray(VAO);
    glDrawArrays(GL_QUADS, 0, 4);
    glBindVertexArray(0);
}

Quad::~Quad() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}