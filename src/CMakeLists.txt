# src/CMakeLists.txt
add_subdirectory(basic)
add_subdirectory(light)
add_subdirectory(shape)

# 创建静态库
file(GLOB IMGUI_SOURCES 
    "${CMAKE_SOURCE_DIR}/lib/imgui/imgui.cpp"
    "${CMAKE_SOURCE_DIR}/lib/imgui/imgui_draw.cpp"
    "${CMAKE_SOURCE_DIR}/lib/imgui/imgui_widgets.cpp"
    "${CMAKE_SOURCE_DIR}/lib/imgui/imgui_tables.cpp"
    "${CMAKE_SOURCE_DIR}/lib/imgui/backends/imgui_impl_glfw.cpp"
    "${CMAKE_SOURCE_DIR}/lib/imgui/backends/imgui_impl_opengl3.cpp"
)

add_library(opengl_engine STATIC
    ${IMGUI_SOURCES}
    $<TARGET_OBJECTS:basic_lib>
    $<TARGET_OBJECTS:light_lib>
    $<TARGET_OBJECTS:shape_lib>
)

target_include_directories(opengl_engine PUBLIC
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
    target_include_directories(opengl_engine PUBLIC ${CMAKE_SOURCE_DIR}/lib/glm)
else()
    message(WARNING "GLM library not found in lib/glm. Please install GLM or adjust the path.")
endif()

# 链接外部库
link_directories(
    "${CMAKE_SOURCE_DIR}/lib/glew/lib/Release/x64"
    "${CMAKE_SOURCE_DIR}/lib/glfw/src"
)

target_link_libraries(opengl_engine 
    PUBLIC
    glew32
    glfw3
    opengl32
)