#pragma once
#include <GL/glew.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "shader.hpp"

// 基础图形类
class Shape {
public:
    /**
     * @brief 构造基础形状
     */
    Shape();

    /**
     * @brief 绘制形状
     * @param shader 当前使用的着色器对象引用（必须已被 use()）
     *
     * 约定：着色器需包含以下 uniform：
     *  - mat4 model    : 模型变换矩阵（由 getModelMatrix() 提供）
     *  - mat4 view     : 视图矩阵（由摄像机提供，在外部上传）
     *  - mat4 projection: 投影矩阵（由摄像机提供，在外部上传）
     *  - vec3 color    : 顶点/片段颜色（在 draw 内部上传）
     */
    virtual void draw(Shader& shader) = 0;
    virtual ~Shape() = default;
    
    /**
     * @brief 设置位置（世界坐标）
     * @param position 三维位置向量 (x, y, z)
     */
    void setPosition(const glm::vec3& position);

    /**
     * @brief 设置旋转（欧拉角）
     * @param rotation Euler 角向量 (pitch, yaw, roll)，单位为度
     */
    void setRotation(const glm::vec3& rotation);  // 欧拉角

    /**
     * @brief 设置缩放
     * @param scale 缩放向量 (sx, sy, sz)
     */
    void setScale(const glm::vec3& scale);
    
    /**
     * @brief 获取模型矩阵（用于上传到 shader 的 model uniform）
     * @return 4x4 模型变换矩阵
     */
    glm::mat4 getModelMatrix() const;

protected:
    glm::vec3 m_position;
    glm::vec3 m_rotation;  // 欧拉角：pitch, yaw, roll（度）
    glm::vec3 m_scale;
};

// 带颜色的基础图形类
class ColoredShape : public Shape {
public:
    /**
     * @brief 基于颜色的形状构造
     * @param color 颜色向量 (r,g,b)，范围通常为 [0,1]
     */
    ColoredShape(const glm::vec3& color);

    /**
     * @brief 设置颜色
     * @param color 颜色向量 (r,g,b)
     */
    void setColor(const glm::vec3& color);

    /**
     * @brief 获取当前颜色
     * @return 颜色向量 (r,g,b)
     */
    glm::vec3 getColor() const;

protected:
    glm::vec3 m_color;
};

// 点类
class Point : public ColoredShape {
public:
    /**
     * @brief 构造一个点
     * @param x 点的 X 坐标
     * @param y 点的 Y 坐标
     * @param z 点的 Z 坐标
     * @param color 点的颜色 (r,g,b)
     */
    Point(float x, float y, float z, const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f));

    /**
     * @brief 绘制点
     * @param shader 着色器引用，用于设置 model 与 color uniform
     */
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
    /**
     * @brief 构造一条线段
     * @param startX 起点 X
     * @param startY 起点 Y
     * @param startZ 起点 Z
     * @param endX 终点 X
     * @param endY 终点 Y
     * @param endZ 终点 Z
     * @param color 线段颜色
     */
    Line(float startX, float startY, float startZ, 
         float endX, float endY, float endZ,
         const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f));

    /**
     * @brief 绘制线段
     * @param shader 着色器引用，用于设置 model 与 color uniform
     */
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
    /**
     * @brief 构造三角形
     * @param x1..z3 三个顶点位置
     * @param color 三角形颜色
     */
    Triangle(float x1, float y1, float z1,
             float x2, float y2, float z2,
             float x3, float y3, float z3,
             const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f));

    /**
     * @brief 绘制三角形
     * @param shader 着色器引用，用于设置 model 与 color uniform
     */
    virtual void draw(Shader& shader) override;
    virtual ~Triangle();
    
private:
    GLuint VAO, VBO;
    glm::vec3 vertices[3];
};

// 矩形类
class Quad : public ColoredShape {
public:
    /**
     * @brief 构造四边形（用四个顶点）
     */
    Quad(float x1, float y1, float z1,
         float x2, float y2, float z2,
         float x3, float y3, float z3,
         float x4, float y4, float z4,
         const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f));

    /**
     * @brief 绘制四边形
     * @param shader 着色器引用，用于设置 model 与 color uniform
     */
    virtual void draw(Shader& shader) override;
    virtual ~Quad();
    
private:
    GLuint VAO, VBO;
    glm::vec3 vertices[4];
};

// 立方体类
class Cube : public ColoredShape {
public:
    /**
     * @brief 构造立方体
     * @param size 立方体边长
     * @param color 颜色
     */
    Cube(float size, const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f));

    /**
     * @brief 绘制立方体
     * @param shader 着色器引用，用于设置 model 与 color uniform
     */
    virtual void draw(Shader& shader) override;
    virtual ~Cube();

private:
    GLuint VAO, VBO;
    std::vector<float> vertices;
};

// 球体类
class Sphere : public ColoredShape {
public:
    /**
     * @brief 构造球体网格
     * @param radius 球半径
     * @param sectors 经向分段数（类似经度）
     * @param stacks 纬向分段数（类似纬度）
     * @param color 球体基色
     */
    Sphere(float radius, int sectors = 36, int stacks = 18, const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f));

    /**
     * @brief 绘制球体
     * @param shader 着色器引用，用于设置 model 与 color uniform
     */
    virtual void draw(Shader& shader) override;
    virtual ~Sphere();

private:
    GLuint VAO, VBO, EBO;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    int sectorCount;
    int stackCount;
};