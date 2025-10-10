cmake_minimum_required(VERSION 3.15)
project(opengl_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置可执行文件输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# 查找OpenGL
find_package(OpenGL REQUIRED)

# 设置第三方库目录
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/lib)

# GLEW
set(GLEW_DIR ${THIRD_PARTY_DIR}/glew-2.2.0)
set(GLEW_INCLUDE_DIRS ${GLEW_DIR}/include)
set(GLEW_LIBRARIES ${GLEW_DIR}/lib/Release/x64/glew32s.lib)

# GLFW
set(GLFW_DIR ${THIRD_PARTY_DIR}/glfw-3.4.bin.WIN64)
set(GLFW_INCLUDE_DIRS ${GLFW_DIR}/include)
set(GLFW_LIBRARIES 
    ${GLFW_DIR}/lib-vc2022/glfw3.lib
)

# GLM
set(GLM_DIR ${THIRD_PARTY_DIR}/glm)

# 包含目录
include_directories(
    ${OPENGL_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
    ${GLM_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

# 查找所有源文件
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "main.cpp"
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接库
target_link_libraries(${PROJECT_NAME}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${GLFW_LIBRARIES}
)

# 复制着色器文件到构建目录
file(GLOB SHADERS "shaders/*")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${SHADERS}
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/
)

# 复制必要的DLL文件到构建目录
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${GLEW_DIR}/bin/Release/x64/glew32.dll
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${GLFW_DIR}/lib-vc2022/glfw3.dll
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
)