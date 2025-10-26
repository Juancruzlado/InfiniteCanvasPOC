# Infinite Notes - Electron + C++ Canvas Integration

## 🎯 Overview

This demo integrates your C++ Infinite Canvas with an Electron-based note-taking app. You get:

- **Electron UI** - Modern web-based interface (like Obsidian)
- **Native C++ Canvas** - OpenGL-powered drawing in a separate window
- **Native Node Addon** - Bridge between JavaScript and C++

## 🏗️ Architecture

```
Electron App (Chromium UI)
       ↓
   IPC Bridge
       ↓
  Node.js Main Process
       ↓
 require('infinitecanvas.node')  ← Compiled C++ addon
       ↓
  C++ Native Code
       ↓
  GLFW OpenGL Window ← Separate native window
```

## 📋 Prerequisites

Make sure you have these installed:

```bash
# Ubuntu/Debian
sudo apt install nodejs npm build-essential cmake
sudo apt install libglfw3-dev libglew-dev libglm-dev
sudo apt install libgl1-mesa-dev

# Verify versions
node --version    # Should be v16+
npm --version     # Should be v8+
```

## 🚀 Build & Run

### Step 1: Install Node.js Dependencies

```bash
cd electron-demo
npm install
```

This installs:
- `electron` - Desktop app framework
- `node-addon-api` - C++ to Node.js bridge
- `node-gyp` - C++ build tool

### Step 2: Build C++ Native Addon

```bash
# From the electron-demo directory
npm run build-native
```

This runs `node-gyp rebuild` which:
1. Reads `../binding.gyp` configuration
2. Compiles all C++ source files
3. Links against OpenGL, GLFW, GLEW
4. Produces `../build/Release/infinitecanvas.node`

**Expected output:**
```
gyp info it worked if it ends with ok
...
gyp info ok
```

**If build fails**, check:
- GLFW/GLEW/GLM are installed
- C++17 compiler is available
- No syntax errors in `src/node_addon.cpp`

### Step 3: Run Electron App

```bash
npm start
```

This launches the Electron app with your UI!

## 🎨 Usage

### In the Electron UI:

1. **Create Notes**
   - Click "+ New Note"
   - Edit in the markdown editor
   - Notes persist in the sidebar

2. **Open Canvas**
   - Click "🎨 Open Canvas"
   - A native OpenGL window spawns
   - Draw with your mouse/stylus

3. **Save/Load Drawings**
   - Click "💾 Save Drawing" → native file dialog
   - Click "📂 Load Drawing" → load previous work
   - Files saved as `.mm` (mind map) format

### In the Canvas Window:

**Drawing:**
- Left Mouse: Draw strokes
- Middle/Right Mouse: Pan canvas
- Scroll: Zoom in/out

**Tools:** (via tool wheel in canvas)
- Brush
- Eraser
- Lasso (select & move)

**Shortcuts:**
- `Ctrl+Z`: Undo
- `Ctrl+Shift+Z`: Redo
- `C`: Clear canvas
- `R`: Reset view
- `ESC`: Close canvas window

## 📁 Project Structure

```
electron-demo/
├── package.json         # Node.js dependencies
├── main.js             # Electron main process (Node.js)
├── preload.js          # Secure IPC bridge
├── renderer.js         # UI logic (frontend)
├── index.html          # UI layout (Tailwind CSS)
└── README.md           # This file

Parent directory:
├── binding.gyp                  # node-gyp build config
├── build/Release/
│   └── infinitecanvas.node     # Compiled C++ addon
├── src/
│   ├── node_addon.cpp          # N-API wrapper
│   ├── Canvas.cpp              # Your existing code
│   ├── VectorRenderer.cpp
│   └── ...
└── include/
    └── *.h
```

## 🔧 Development

### Rebuild After C++ Changes

```bash
npm run build-native
npm start
```

### Debug C++ Code

Add debug output in `src/node_addon.cpp`:

```cpp
std::cout << "Debug: " << someVariable << std::endl;
```

Electron will show this in the terminal.

### Debug JavaScript

Open DevTools in `main.js`:

```javascript
mainWindow.webContents.openDevTools();
```

## 🐛 Troubleshooting

### "Cannot find module 'infinitecanvas.node'"

**Problem:** C++ addon not built

**Solution:**
```bash
cd .. && node-gyp rebuild
```

### "Failed to initialize GLFW"

**Problem:** OpenGL context issues

**Solution:**
- Make sure you're not running in a headless environment
- Check `libglfw3` is installed
- Try running the standalone C++ app first

### "Canvas not initialized"

**Problem:** `init()` not called before other operations

**Solution:** The addon auto-initializes in `main.js`, but you can manually call:
```javascript
canvas.init();
```

### Build errors (missing headers)

**Problem:** Missing development libraries

**Solution:**
```bash
sudo apt install libglfw3-dev libglew-dev libglm-dev
```

## 🎓 How It Works

### 1. N-API Wrapper (`src/node_addon.cpp`)

Exposes C++ functions to Node.js:

```cpp
Napi::Value OpenWindow(const Napi::CallbackInfo& info) {
    // Spawn GLFW window in separate thread
    g_renderThread = new std::thread([]() {
        // Initialize GLFW, OpenGL
        // Run render loop
    });
}

NODE_API_MODULE(infinitecanvas, InitModule)
```

### 2. Electron Main Process (`main.js`)

Loads the addon and handles IPC:

```javascript
const canvas = require('../build/Release/infinitecanvas.node');
canvas.init();

ipcMain.handle('open-canvas', async () => {
  return canvas.openWindow();
});
```

### 3. Renderer Process (`renderer.js`)

UI calls into the addon:

```javascript
document.getElementById('openCanvasBtn').addEventListener('click', async () => {
  await window.canvasAPI.openCanvas();
});
```

### 4. Threading Model

```
Main Thread (Node.js Event Loop)
  ↓
  Spawns →  Render Thread (C++ GLFW)
              ↓
              OpenGL Context
              ↓
              Your Canvas Code
```

The canvas runs in a **separate thread** to avoid blocking Node.js.

## 🚀 Next Steps

### Add More Features:

1. **Export to PNG/SVG**
   ```cpp
   Napi::Value ExportPNG(const Napi::CallbackInfo& info) {
       // Render to framebuffer, save as image
   }
   ```

2. **Real-Time Sync**
   - Send canvas updates back to Electron
   - Display thumbnail in notes list

3. **Multiple Canvases**
   - Link drawings to specific notes
   - Manage multiple `.mm` files

4. **Cloud Sync**
   - Save to cloud storage
   - Collaborative editing

## 📚 Resources

- [Node-API Docs](https://nodejs.org/api/n-api.html)
- [Electron Docs](https://www.electronjs.org/docs)
- [node-gyp Guide](https://github.com/nodejs/node-gyp)

## ✅ What We Achieved

✅ **Cross-platform UI** (Electron)  
✅ **Native C++ rendering** (OpenGL)  
✅ **Seamless integration** (Node.js addon)  
✅ **No OS-specific code** (removed zenity)  
✅ **Professional workflow** (save/load with native dialogs)  

## 🎉 Success!

Your Obsidian-like note app with a native C++ infinite canvas is ready!

The best part: Your existing C++ code runs **unchanged** in its own window, while Electron handles the UI.
