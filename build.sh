#!/bin/bash

# Build script for VectorSketch

set -e  # Exit on error

echo "=== VectorSketch Build Script ==="
echo ""

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

cd build

# Configure
echo "Configuring with CMake..."
cmake ..

# Build
echo "Building..."
cmake --build . -j$(nproc)

echo ""
echo "=== Build Complete ==="
echo "Run the application with: ./build/VectorSketch"
echo ""
