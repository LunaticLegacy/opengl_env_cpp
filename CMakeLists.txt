cmake_minimum_required(VERSION 3.15)
project(opengl_test)

set(CMAKE_CXX_STANDARD 17)

# 头文件路径
include_directories(
    "${CMAKE_SOURCE_DIR}/lib/glew-2.2.0/include"
    "${CMAKE_SOURCE_DIR}/lib/glfw-3.4.bin.WIN64/include"
    "${CMAKE_SOURCE_DIR}/lib/glm"
    "${CMAKE_SOURCE_DIR}/include"
)

# 库文件路径
link_directories(
    "${CMAKE_SOURCE_DIR}/lib/glew-2.2.0/lib/Release/x64"
    "${CMAKE_SOURCE_DIR}/lib/glfw-3.4.bin.WIN64/lib-mingw-w64"
)

# 确定编译的文件
add_executable(opengl_test
    ./main.cpp
    ./src/shapes.cpp
    ./src/camera.cpp
    ./src/shader.cpp
    ./src/light.cpp
)

# 所有需要导入的dll文件
set(ALL_DLL_FILES
    "${CMAKE_SOURCE_DIR}/lib/glew-2.2.0/bin/Release/x64/glew32.dll"
    "${CMAKE_SOURCE_DIR}/lib/glfw-3.4.bin.WIN64/lib-mingw-w64/glfw3.dll"
)

# 链接 GLEW + OpenGL + GLFW（Windows）
target_link_libraries(opengl_test
    PRIVATE
    glew32
    glfw3
    opengl32
)

# 编译后复制所有DLL到目标
foreach(dll_file ${ALL_DLL_FILES})
    add_custom_command(TARGET opengl_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${dll_file}"
        $<TARGET_FILE_DIR:opengl_test>
    )
endforeach()

# 复制着色器文件到构建目录
configure_file(shaders/vertex.glsl ${CMAKE_CURRENT_BINARY_DIR}/shaders/vertex.glsl COPYONLY)
configure_file(shaders/fragment.glsl ${CMAKE_CURRENT_BINARY_DIR}/shaders/fragment.glsl COPYONLY)