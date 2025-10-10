#pragma once
#include <GL/glew.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "shader.hpp"

// 基础图形类
class Shape {
public:
    Shape();
    // 修改：draw 接受 Shader&，以便在 draw 前设置 model/color 等 uniform
    virtual void draw(Shader& shader) = 0;
    virtual ~Shape() = default;
    
    // 设置变换矩阵
    void setPosition(const glm::vec3& position);
    void setRotation(const glm::vec3& rotation);  // 欧拉角
    void setScale(const glm::vec3& scale);
    
    // 获取变换矩阵
    glm::mat4 getModelMatrix() const;

protected:
    glm::vec3 m_position;
    glm::vec3 m_rotation;  // 欧拉角：pitch, yaw, roll
    glm::vec3 m_scale;
};

// 带颜色的基础图形类
class ColoredShape : public Shape {
public:
    ColoredShape(const glm::vec3& color);
    void setColor(const glm::vec3& color);
    glm::vec3 getColor() const;

protected:
    glm::vec3 m_color;
};

// 点类
class Point : public ColoredShape {
public:
    Point(float x, float y, float z, const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f));
    virtual void draw(Shader& shader) override;
    virtual ~Point();
    
private:
    GLuint VAO, VBO;
    glm::vec3 position;
};

// 2D点类
class Point2D : public Point {
public:
    Point2D(float x, float y, const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f));
};

// 3D点类
class Point3D : public Point {
public:
    Point3D(float x, float y, float z, const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f));
};

// 线段类
class Line : public ColoredShape {
public:
    Line(float startX, float startY, float startZ, 
         float endX, float endY, float endZ,
         const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f));
    virtual void draw(Shader& shader) override;
    virtual ~Line();
    
private:
    GLuint VAO, VBO;
    glm::vec3 startPoint;
    glm::vec3 endPoint;
};

// 三角形类
class Triangle : public ColoredShape {
public:
    Triangle(float x1, float y1, float z1,
             float x2, float y2, float z2,
             float x3, float y3, float z3,
             const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f));
    virtual void draw(Shader& shader) override;
    virtual ~Triangle();
    
private:
    GLuint VAO, VBO;
    glm::vec3 vertices[3];
};

// 矩形类
class Quad : public ColoredShape {
public:
    Quad(float x1, float y1, float z1,
         float x2, float y2, float z2,
         float x3, float y3, float z3,
         float x4, float y4, float z4,
         const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f));
    virtual void draw(Shader& shader) override;
    virtual ~Quad();
    
private:
    GLuint VAO, VBO;
    glm::vec3 vertices[4];
};

// 立方体类
class Cube : public ColoredShape {
public:
    Cube(float size, const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f));
    virtual void draw(Shader& shader) override;
    virtual ~Cube();

private:
    GLuint VAO, VBO;
    std::vector<float> vertices;
};

// 球体类
class Sphere : public ColoredShape {
public:
    Sphere(float radius, int sectors = 36, int stacks = 18, const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f));
    virtual void draw(Shader& shader) override;
    virtual ~Sphere();

private:
    GLuint VAO, VBO, EBO;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    int sectorCount;
    int stackCount;
};