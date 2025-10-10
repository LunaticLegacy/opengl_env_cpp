#include <iostream>
#include <cstdint>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glwindow.hpp"

int main() {
    // 初始化
    GLCore::Initialize();

    // 声明窗口但不创建
    Window window;
    try {
        // 包裹一个try，希望别出问题
        window = Window();
    } catch(std::runtime_error e) {
        std::cerr << "Error in initialize window" << e.what() << std::endl;
    }

    // 关闭
    GLCore::Shutdown();
    return 0;
}