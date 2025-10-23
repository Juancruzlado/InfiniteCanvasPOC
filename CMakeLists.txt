cmake_minimum_required(VERSION 3.10)
project(VectorSketch VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# Download ImGui if not present
include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.1
)
FetchContent_MakeAvailable(imgui)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${OPENGL_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# ImGui sources
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# Source files
set(SOURCES
    src/main.cpp
    src/Stroke.cpp
    src/BezierSmoother.cpp
    src/VectorRenderer.cpp
    src/Canvas.cpp
    src/ToolWheel.cpp
    ${IMGUI_SOURCES}
)

# Header files (for IDE organization)
set(HEADERS
    include/StrokePoint.h
    include/Stroke.h
    include/BezierSmoother.h
    include/VectorRenderer.h
    include/Canvas.h
    include/ToolWheel.h
)

# Create executable
add_executable(VectorSketch ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(VectorSketch
    ${OPENGL_LIBRARIES}
    GLEW::GLEW
    glfw
    glm::glm
)

# Compiler warnings
if(MSVC)
    target_compile_options(VectorSketch PRIVATE /W4)
else()
    target_compile_options(VectorSketch PRIVATE -Wall -Wextra -pedantic)
endif()

# Print build info
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
