cmake_minimum_required(VERSION 3.15)
project(opengl_test)

set(CMAKE_CXX_STANDARD 17)

# 设置包含目录
set(INCLUDE_DIRS 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/basic
    ${CMAKE_SOURCE_DIR}/include/light
    ${CMAKE_SOURCE_DIR}/include/shape
    ${CMAKE_SOURCE_DIR}/lib/glew/include
    ${CMAKE_SOURCE_DIR}/lib/glfw/include
    ${CMAKE_SOURCE_DIR}/lib/imgui
    ${CMAKE_SOURCE_DIR}/lib/imgui/backends
)

# 检查GLM库是否存在
if(EXISTS "${CMAKE_SOURCE_DIR}/lib/glm")
    list(APPEND INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/lib/glm)
else()
    message(WARNING "GLM library not found in lib/glm. Please install GLM or adjust the path.")
endif()

# 添加包含目录
include_directories(${INCLUDE_DIRS})

# 添加子目录
add_subdirectory(src)

# 创建可执行文件
add_executable(opengl_test 
    main.cpp
)

# 链接库
target_link_libraries(opengl_test 
    PRIVATE 
    opengl_engine
)

# 库
set(GLEW_LIB_PATH "${CMAKE_SOURCE_DIR}/lib/glew/lib/Release/x64/libglew32.a")
# 注意：GLFW 是静态链接库，不需要 DLL
# set(GLFW_LIB_PATH "${CMAKE_SOURCE_DIR}/lib/glfw/src/libglfw3.a")

# 动态库
set(GLEW_DLL_PATH "${CMAKE_SOURCE_DIR}/lib/glew/bin/Release/x64/glew32.dll")
# GLFW 不提供预编译的 DLL，需要自己编译或使用静态链接
# set(GLFW_DLL_PATH "${CMAKE_SOURCE_DIR}/lib/glfw/lib-mingw-w64/glfw3.dll")


# 复制DLL文件（如果存在）
if(EXISTS ${GLEW_DLL_PATH})
    add_custom_command(TARGET opengl_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${GLEW_DLL_PATH}"
        $<TARGET_FILE_DIR:opengl_test>
    )
endif()

# 注释掉 GLFW DLL 复制部分，因为 GLFW 使用静态链接
# if(EXISTS ${GLFW_DLL_PATH})
#     add_custom_command(TARGET opengl_test POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy_if_different
#         "${GLFW_DLL_PATH}"
#         $<TARGET_FILE_DIR:opengl_test>
#     )
#endif()

# 复制着色器文件到构建目录
configure_file(shaders/vertex.glsl ${CMAKE_CURRENT_BINARY_DIR}/shaders/vertex.glsl COPYONLY)
configure_file(shaders/fragment.glsl ${CMAKE_CURRENT_BINARY_DIR}/shaders/fragment.glsl COPYONLY)

# 添加调试信息
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(opengl_test PRIVATE DEBUG)
endif()