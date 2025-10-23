# Vector Sketch - Infinite Canvas POC

A C++ proof-of-concept application that simulates the infinite canvas vector sketching feature from Concepts app. Features stroke sampling with pressure/tilt data, Bézier curve smoothing, and GPU-accelerated rendering.

## Features

- ✨ **Stroke Sampling**: Each stroke is sampled as a series of points with pressure and tilt data
- 🎨 **Bézier Smoothing**: Raw stroke points are smoothed into beautiful Bézier curves using Catmull-Rom interpolation
- 🚀 **GPU Acceleration**: OpenGL-based vector rendering for crisp, smooth lines
- ♾️ **Infinite Canvas**: Pan and zoom with unlimited drawing space
- 🖱️ **Pressure Simulation**: Mouse speed simulates pen pressure (works with stylus input too)

## Architecture

```
VectorSketch/
├── include/           # Header files
│   ├── StrokePoint.h  # Point data structure with pressure/tilt
│   ├── Stroke.h       # Stroke collection
│   ├── BezierSmoother.h  # Bézier curve generation
│   ├── VectorRenderer.h  # GPU rendering engine
│   └── Canvas.h       # Infinite canvas manager
└── src/               # Implementation files
    ├── main.cpp       # Application entry point
    ├── Stroke.cpp
    ├── BezierSmoother.cpp
    ├── VectorRenderer.cpp
    └── Canvas.cpp
```

## Technical Details

### Stroke Sampling
Each stroke point captures:
- **Position** (x, y coordinates)
- **Pressure** (0.0 - 1.0, affects line width)
- **Tilt** (x, y components for stylus orientation)
- **Timestamp** (for velocity calculations)

### Bézier Smoothing
- Uses Catmull-Rom spline approach to generate smooth cubic Bézier curves
- Maintains tangent continuity between segments
- Configurable tension parameter for smoothness control

### GPU Rendering
- OpenGL 3.3 Core Profile with GLSL shaders
- Vertex shader transforms points with MVP matrix
- Fragment shader handles color and anti-aliasing
- Dynamic VBO updates for real-time drawing
- Line smoothing with multisampling (MSAA)

## Requirements

### System Dependencies
- **C++17** compiler (GCC 7+, Clang 5+, or MSVC 2017+)
- **CMake** 3.10 or higher
- **OpenGL** 3.3 or higher

### Libraries
- **GLFW3** - Window and input management
- **GLEW** - OpenGL extension loading
- **GLM** - OpenGL Mathematics library

## Installation

### Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential cmake
sudo apt install libglfw3-dev libglew-dev libglm-dev
```

### Fedora/RHEL
```bash
sudo dnf install gcc-c++ cmake
sudo dnf install glfw-devel glew-devel glm-devel
```

### Arch Linux
```bash
sudo pacman -S base-devel cmake
sudo pacman -S glfw-x11 glew glm
```

### macOS
```bash
brew install cmake glfw glew glm
```

## Building

```bash
# Clone or navigate to the project directory
cd /home/jcl/CascadeProjects/C++

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build
cmake --build .

# Or use make directly
make -j$(nproc)
```

## Running

```bash
# From build directory
./VectorSketch
```

## Controls

| Action | Control |
|--------|---------|
| **Draw** | Left Mouse Button (hold and drag) |
| **Pan Canvas** | Middle or Right Mouse Button (drag) |
| **Zoom** | Mouse Scroll Wheel |
| **Clear Canvas** | `C` key |
| **Reset View** | `R` key |
| **Exit** | `ESC` key |

## Usage

1. **Drawing**: Click and hold the left mouse button, then drag to draw strokes
   - Draw faster for thinner lines (less pressure)
   - Draw slower for thicker lines (more pressure)

2. **Navigation**: 
   - Use middle/right mouse to pan around the canvas
   - Scroll to zoom in/out at cursor position

3. **Management**:
   - Press `C` to clear all strokes
   - Press `R` to reset zoom and position

## How It Works

### 1. Stroke Capture
```
User Input → StrokePoint (x, y, pressure, tilt) → Stroke Collection
```

### 2. Smoothing Pipeline
```
Raw Points → Catmull-Rom Tangents → Bézier Control Points → Smooth Curve
```

### 3. Rendering Pipeline
```
Bézier Segments → Tessellation → Vertex Buffer → GPU Shader → Screen
```

### 4. Mathematical Foundation

**Catmull-Rom to Bézier Conversion:**
- For each segment between points P₀ and P₁
- Calculate tangents: T₀ = (P₁ - P₋₁) × tension
- Control points: C₁ = P₀ + T₀/3, C₂ = P₁ - T₁/3

**Bézier Curve Evaluation:**
```
B(t) = (1-t)³P₀ + 3(1-t)²t·C₁ + 3(1-t)t²·C₂ + t³P₁
```

## Extending the POC

### Adding Real Stylus Support
The code is structured to easily integrate tablet/stylus input:
- Replace pressure simulation in `simulatePressure()` with actual pressure data
- Update `StrokePoint` tilt values with real tilt sensor data
- Consider libraries like **libinput** or **XInput2** for Linux stylus support

### Cross-Platform GPU APIs
Current OpenGL implementation can be extended:
- **Metal** (macOS/iOS): Replace VectorRenderer with Metal compute shaders
- **DirectX** (Windows): Use D2D/D3D for vector rendering
- **Vulkan** (All platforms): For maximum performance and control

### Performance Optimizations
- Implement spatial indexing (quadtree) for large canvas
- Add stroke simplification (Douglas-Peucker algorithm)
- Batch rendering for multiple strokes
- Implement LOD system for zoom levels
- Use instanced rendering for stroke segments

### Additional Features
- Multiple layers
- Undo/redo system
- Stroke selection and editing
- Export to SVG/PDF
- Custom brush styles
- Color palette system

## Troubleshooting

### "Failed to initialize GLEW"
- Ensure OpenGL drivers are installed: `glxinfo | grep "OpenGL version"`
- Update graphics drivers

### "Cannot find GLFW/GLEW/GLM"
- Verify installation: `pkg-config --modversion glfw3 glew glm`
- Check CMake can find packages: `cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON`

### Black window or no rendering
- Check OpenGL version: Must be 3.3 or higher
- Try software rendering: `LIBGL_ALWAYS_SOFTWARE=1 ./VectorSketch`

## Performance Notes

- **CPU**: Bézier calculation is done per-stroke on CPU
- **GPU**: All rendering is hardware-accelerated
- **Memory**: Dynamic allocation for stroke data
- **Target**: 60 FPS on modern integrated graphics

## License

This is a proof-of-concept project for educational purposes.

## Future Work

- [ ] Variable width along stroke (pressure mapping)
- [ ] Stroke texture/pattern support
- [ ] Multi-touch gestures
- [ ] Hardware stylus integration
- [ ] Persistence (save/load)
- [ ] Vector export (SVG, PDF)
- [ ] More sophisticated smoothing algorithms

## References

- [Bézier Curves and Splines](https://pomax.github.io/bezierinfo/)
- [Catmull-Rom Splines](https://en.wikipedia.org/wiki/Cubic_Hermite_spline)
- [OpenGL Tutorial](https://learnopengl.com/)
