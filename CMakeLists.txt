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

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${OPENGL_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
)

# Source files
set(SOURCES
    src/main.cpp
    src/Stroke.cpp
    src/BezierSmoother.cpp
    src/VectorRenderer.cpp
    src/Canvas.cpp
)

# Header files (for IDE organization)
set(HEADERS
    include/StrokePoint.h
    include/Stroke.h
    include/BezierSmoother.h
    include/VectorRenderer.h
    include/Canvas.h
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
