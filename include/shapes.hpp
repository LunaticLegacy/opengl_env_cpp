#pragma once
#include <GL/glew.h>
#include <vector>
#include <glm/glm.hpp>

// 基础图形类
class Shape {
public:
    virtual void draw() = 0;
    virtual ~Shape() = default;
};

// 点类
class Point : public Shape {
public:
    Point(float x, float y, float z);
    virtual void draw() override;
    virtual ~Point();
    
private:
    GLuint VAO, VBO;
    glm::vec3 position;
};

// 2D点类
class Point2D : public Point {
public:
    Point2D(float x, float y);
};

// 3D点类
class Point3D : public Point {
public:
    Point3D(float x, float y, float z);
};

// 线段类
class Line : public Shape {
public:
    Line(float startX, float startY, float startZ, 
         float endX, float endY, float endZ);
    virtual void draw() override;
    virtual ~Line();
    
private:
    GLuint VAO, VBO;
    glm::vec3 startPoint;
    glm::vec3 endPoint;
};

// 三角形类
class Triangle : public Shape {
public:
    Triangle(float x1, float y1, float z1,
             float x2, float y2, float z2,
             float x3, float y3, float z3);
    virtual void draw() override;
    virtual ~Triangle();
    
private:
    GLuint VAO, VBO;
    glm::vec3 vertices[3];
};

// 矩形类
class Quad : public Shape {
public:
    Quad(float x1, float y1, float z1,
         float x2, float y2, float z2,
         float x3, float y3, float z3,
         float x4, float y4, float z4);
    virtual void draw() override;
    virtual ~Quad();
    
private:
    GLuint VAO, VBO;
    glm::vec3 vertices[4];
};