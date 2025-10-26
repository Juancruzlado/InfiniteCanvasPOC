#!/bin/bash
set -e

# Create necessary directories
mkdir -p libs
cd libs

# Clone GLFW
if [ ! -d "glfw" ]; then
    git clone https://github.com/glfw/glfw.git
    cd glfw
    git checkout 3.3.8  # Use a stable version
    cd ..
fi

# Clone GLAD
if [ ! -d "glad" ]; then
    git clone https://github.com/Dav1dde/glad.git
    cd glad
    python3 -m pip install --user glad
    python3 -m glad --profile=core --out-path=generated --generator=c --spec=gl
    mkdir -p include/glad
    mkdir -p include/KHR
    cp generated/src/glad.c .
    cp generated/include/glad/gl.h include/glad/
    cp generated/include/KHR/khrplatform.h include/KHR/
    cd ..
fi

# Clone Dear ImGui
if [ ! -d "imgui" ]; then
    git clone https://github.com/ocornut/imgui.git
    cd imgui
    git checkout v1.89.5  # Use a stable version
    cd ..
fi

# Create symbolic links
cd ..
ln -sf libs/glfw glfw
ln -sf libs/glad glad
ln -sf libs/imgui imgui

echo "Dependencies set up successfully!"
