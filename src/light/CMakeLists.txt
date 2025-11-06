# src/light/CMakeLists.txt

# 收集light模块的源文件
set(LIGHT_SOURCES
    light.cpp
)

# 创建对象库
add_library(light_lib OBJECT
    ${LIGHT_SOURCES}
)

# 设置包含目录
target_include_directories(light_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/light
    ${CMAKE_SOURCE_DIR}/lib/glew-2.2.0/include
)