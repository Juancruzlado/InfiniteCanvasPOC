cmake_minimum_required(VERSION 3.10)
project(StandaloneCanvasApp VERSION 1.0.0 LANGUAGES CXX C)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(GLEW REQUIRED)

# Set include directories
include_directories(
    ${OPENGL_INCLUDE_DIRS}
    ${GLFW3_INCLUDE_DIR}
    ${GLM_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
)

# GLEW is already found by find_package


# Add ImGui sources
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# Create ImGui library
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# Add source files
set(SOURCES
    src/main.cpp
    src/Canvas.cpp
    src/Stroke.cpp
    src/VectorRenderer.cpp
    src/BezierSmoother.cpp
    src/ToolWheel.cpp
)

# Add executable
add_executable(StandaloneCanvas ${SOURCES})

# Add include directories
target_include_directories(StandaloneCanvas PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OPENGL_INCLUDE_DIRS}
    ${GLFW_SOURCE_DIR}/include
    ${GLM_INCLUDE_DIRS}
    ${GLAD_INCLUDE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# Link libraries
target_link_libraries(StandaloneCanvas
    PRIVATE
    OpenGL::GL
    glfw
    GLEW::GLEW
    glm::glm
    imgui
    X11
    Xrandr
    Xinerama
    Xcursor
    m
    dl
    pthread
    ${CMAKE_THREAD_LIBS_INIT}
)

# Copy shaders to build directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/shaders)
file(GLOB SHADER_FILES ${CMAKE_SOURCE_DIR}/shaders/*.glsl)
file(COPY ${SHADER_FILES} DESTINATION ${CMAKE_BINARY_DIR}/shaders)

# Copy fonts to build directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/fonts)
file(GLOB FONT_FILES ${CMAKE_SOURCE_DIR}/fonts/*.ttf)
file(COPY ${FONT_FILES} DESTINATION ${CMAKE_BINARY_DIR}/fonts)

# Set output directory
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# Install target
install(TARGETS StandaloneCanvas DESTINATION bin)
