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
# Select GLEW library depending on generator / compiler
if(MSVC)
    # Use MSVC prebuilt libs (static import or static lib)
    set(GLEW_LIBRARIES ${GLEW_DIR}/lib/Release/x64/glew32s.lib)
    set(GLEW_DLL ${GLEW_DIR}/bin/Release/x64/glew32.dll)
else()
    # Assume MinGW: prefer import library if present, otherwise link against the DLL import lib if available
    # Check common MinGW names under the prebuilt release folder
    find_library(GLEW_MINGW_LIB NAMES libglew32.a glew32.dll.a PATHS ${GLEW_DIR}/lib/Release/x64 ${GLEW_DIR}/lib/Release  NO_DEFAULT_PATH)
    if(GLEW_MINGW_LIB)
        set(GLEW_LIBRARIES ${GLEW_MINGW_LIB})
    else()
        # Fall back: if prebuilt MSVC static lib exists in the package use it and define GLEW_STATIC
        if(EXISTS ${GLEW_DIR}/lib/Release/x64/glew32s.lib)
            set(GLEW_LIBRARIES ${GLEW_DIR}/lib/Release/x64/glew32s.lib)
        else()
            # As last resort try the generic import name (may fail if no import lib present)
            set(GLEW_LIBRARIES -lglew32)
        endif()
    endif()
    set(GLEW_DLL ${GLEW_DIR}/bin/Release/x64/glew32.dll)
endif()

# GLFW
set(GLFW_DIR ${THIRD_PARTY_DIR}/glfw-3.4.bin.WIN64)
set(GLFW_INCLUDE_DIRS ${GLFW_DIR}/include)
# Select GLFW library depending on compiler
if(MSVC)
    # prefer vc libraries
    set(GLFW_LIBRARIES ${GLFW_DIR}/lib-vc2019/glfw3.lib)
else()
    # use MinGW prebuilt static lib (libglfw3.a) or DLL import
    set(GLFW_LIBRARIES ${GLFW_DIR}/lib-mingw-w64/libglfw3.a)
    set(GLFW_DLL ${GLFW_DIR}/lib-mingw-w64/glfw3.dll)
endif()

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

# If we're linking the static GLEW library, define GLEW_STATIC for compilation so symbols are resolved correctly
if(GLEW_LIBRARIES)
    string(FIND "${GLEW_LIBRARIES}" "glew32s" _glew_static_found)
    if(NOT _glew_static_found EQUAL -1)
        target_compile_definitions(${PROJECT_NAME} PRIVATE GLEW_STATIC)
    endif()
endif()

# 链接库
target_link_libraries(${PROJECT_NAME}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${GLFW_LIBRARIES}
    # 添加MinGW需要的系统库
    -lglu32
    -lopengl32
    -lgdi32
    -luser32
    -lkernel32
    -lshell32
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
    ${GLFW_DIR}/lib-mingw-w64/glfw3.dll
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
)