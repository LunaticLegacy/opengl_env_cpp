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

# 复制DLL文件
set(ALL_DLL_FILES
    "${CMAKE_SOURCE_DIR}/lib/glew-2.2.0/bin/Release/x64/glew32.dll"
    "${CMAKE_SOURCE_DIR}/lib/glfw-3.4.bin.WIN64/lib-mingw-w64/glfw3.dll"
)

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

# 添加调试信息
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(opengl_test PRIVATE DEBUG)
endif()