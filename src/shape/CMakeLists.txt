# src/shape/CMakeLists.txt

# 收集shape模块的源文件
set(SHAPE_SOURCES
    shapes.cpp
)

# 创建对象库
add_library(shape_lib OBJECT
    ${SHAPE_SOURCES}
)

# 设置包含目录
target_include_directories(shape_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/shape
    ${CMAKE_SOURCE_DIR}/include/basic
    ${CMAKE_SOURCE_DIR}/lib/glew-2.2.0/include
)

# 检查GLM库是否存在
if(EXISTS "${CMAKE_SOURCE_DIR}/lib/glm")
    target_include_directories(shape_lib PUBLIC ${CMAKE_SOURCE_DIR}/lib/glm)
endif()