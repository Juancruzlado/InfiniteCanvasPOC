#!/bin/bash
set -e

# Check if we need to set up dependencies
if [ ! -d "glfw" ] || [ ! -d "glad" ] || [ ! -d "imgui" ]; then
    echo "Setting up dependencies..."
    chmod +x setup_dependencies.sh
    ./setup_dependencies.sh
fi

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "Configuring build..."
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=../install \
    -DGLFW_BUILD_DOCS=OFF \
    -DGLFW_BUILD_TESTS=OFF \
    -DGLFW_BUILD_EXAMPLES=OFF

# Build the project
echo "Building project..."
cmake --build . --config Release -- -j$(nproc)

# Install the application
echo "Installing..."
cmake --install .

echo "\nBuild complete! Executable is in the install/bin directory."
echo "Run it with: cd install/bin && ./StandaloneCanvas"
