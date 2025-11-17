# src/basic/CMakeLists.txt

# 收集basic模块的源文件
file(GLOB BASIC_SOURCES
    *.cpp
)


# 创建对象库
add_library(basic_lib OBJECT
    ${BASIC_SOURCES}
)

# 设置包含目录
target_include_directories(basic_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/basic
    ${CMAKE_SOURCE_DIR}/lib/glew-2.2.0/include
    ${CMAKE_SOURCE_DIR}/lib/glfw-3.4.bin.WIN64/include
)

# 检查GLM库是否存在
if(EXISTS "${CMAKE_SOURCE_DIR}/lib/glm")
    target_include_directories(basic_lib PUBLIC ${CMAKE_SOURCE_DIR}/lib/glm)
endif()