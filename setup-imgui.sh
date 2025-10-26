#!/bin/bash
# Download and extract ImGui for the Node.js addon build

set -e

IMGUI_VERSION="v1.90.1"
IMGUI_DIR="imgui"

echo "📦 Setting up ImGui ${IMGUI_VERSION}..."

# Check if ImGui already exists
if [ -d "$IMGUI_DIR" ]; then
    echo "✅ ImGui already exists at $IMGUI_DIR"
    exit 0
fi

# Download ImGui
echo "⬇️  Downloading ImGui ${IMGUI_VERSION}..."
curl -L "https://github.com/ocornut/imgui/archive/refs/tags/${IMGUI_VERSION}.tar.gz" -o imgui.tar.gz

# Extract
echo "📂 Extracting..."
tar -xzf imgui.tar.gz
mv "imgui-${IMGUI_VERSION:1}" "$IMGUI_DIR"  # Remove 'v' prefix

# Cleanup
rm imgui.tar.gz

echo "✅ ImGui setup complete!"
echo "   Location: $IMGUI_DIR"
