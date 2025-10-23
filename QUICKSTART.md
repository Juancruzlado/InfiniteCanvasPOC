# Quick Start Guide

## 📦 Install Dependencies

The dependencies are not currently installed on your system. Run one of the following commands based on your Linux distribution:

### Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential cmake libglfw3-dev libglew-dev libglm-dev
```

### Fedora/RHEL
```bash
sudo dnf install gcc-c++ cmake glfw-devel glew-devel glm-devel
```

### Arch Linux
```bash
sudo pacman -S base-devel cmake glfw-x11 glew glm
```

## 🔨 Build

Once dependencies are installed, build the project:

```bash
./build.sh
```

Or manually:
```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

## 🚀 Run

```bash
./build/VectorSketch
```

## 🎨 Controls

- **Left Mouse**: Draw strokes
- **Middle/Right Mouse**: Pan the canvas
- **Scroll Wheel**: Zoom in/out
- **C**: Clear canvas
- **R**: Reset view
- **ESC**: Exit

## 🎯 What You'll See

1. A white canvas window opens
2. Draw by clicking and dragging with the mouse
3. Your strokes are smoothed into beautiful Bézier curves in real-time
4. Mouse speed affects line thickness (pressure simulation)
5. Pan and zoom to explore your infinite canvas

## ✨ Features Demonstrated

- ✅ Stroke sampling with pressure/tilt data
- ✅ Catmull-Rom Bézier smoothing
- ✅ GPU-accelerated OpenGL rendering
- ✅ Infinite canvas with pan/zoom
- ✅ Real-time rendering

## 🔧 Troubleshooting

**"Package not found" errors during build:**
- Make sure you installed all dependencies (see above)

**"Failed to initialize GLEW":**
- Check OpenGL support: `glxinfo | grep "OpenGL version"`
- Update your graphics drivers

**Black screen or crashes:**
- Verify OpenGL 3.3+ support
- Try software rendering: `LIBGL_ALWAYS_SOFTWARE=1 ./build/VectorSketch`

## 📚 Next Steps

See `README.md` for detailed architecture, extending the POC, and technical details.
