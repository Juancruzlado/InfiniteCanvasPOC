# Electron + C++ Native Addon Integration Guide

## Summary

✅ **Your C++ code IS suitable for Electron integration with ONE fix needed**

### OS Dependencies Found:
- ❌ **zenity file dialogs** (Linux-specific) - lines 44-84 in `main.cpp`
- ✅ Everything else is cross-platform (OpenGL, GLFW, GLEW, GLM, ImGui, C++17)

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                      Electron App                            │
│                                                               │
│  ┌────────────────────┐         ┌─────────────────────────┐ │
│  │  Renderer Process  │         │    Main Process          │ │
│  │  (Chromium UI)     │         │    (Node.js)            │ │
│  │                    │         │                          │ │
│  │  • Note List       │◄────────┤  • IPC Handler          │ │
│  │  • Markdown Editor │         │  • File System          │ │
│  │  • "Open Canvas" btn│        │  • Native Module Loader │ │
│  └────────────────────┘         └──────────┬──────────────┘ │
│                                              │                │
└──────────────────────────────────────────────┼───────────────┘
                                               │ require()
                                               ▼
                    ┌─────────────────────────────────────────┐
                    │   infinitecanvas.node                   │
                    │   (Native C++ Node Addon)              │
                    │                                         │
                    │  ┌──────────────────────────────────┐  │
                    │  │  N-API Wrapper Functions         │  │
                    │  │  • init()                        │  │
                    │  │  • openWindow()                  │  │
                    │  │  • saveDrawing(path)             │  │
                    │  │  • loadDrawing(path)             │  │
                    │  └──────────┬───────────────────────┘  │
                    │             │                           │
                    │             ▼                           │
                    │  ┌──────────────────────────────────┐  │
                    │  │  Your Existing C++ Code          │  │
                    │  │  • Canvas                        │  │
                    │  │  • VectorRenderer                │  │
                    │  │  • OpenGL/GLFW Window           │  │
                    │  └──────────────────────────────────┘  │
                    └─────────────────────────────────────────┘
```

---

## Implementation Steps

### 1. Fix OS-Specific Code

**Remove zenity dependency** - Electron will handle file dialogs:

```cpp
// OLD (main.cpp lines 44-84)
std::string openNativeFileDialog(bool isSave) {
    // Uses zenity - Linux only ❌
    command = "zenity --file-selection...";
}

// NEW - Remove this function, handle from JavaScript
// Electron provides dialog.showSaveDialog() / dialog.showOpenDialog()
```

### 2. Create N-API Wrapper

Create `src/node_addon.cpp` to expose C++ functionality to Node.js:

```cpp
#include <napi.h>
#include "Canvas.h"
#include "VectorRenderer.h"
#include <GLFW/glfw3.h>
#include <thread>

using namespace VectorSketch;

// Global state
static Canvas* g_canvas = nullptr;
static VectorRenderer* g_renderer = nullptr;
static GLFWwindow* g_window = nullptr;
static std::thread* g_renderThread = nullptr;

// Initialize canvas and renderer
Napi::Value Init(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (g_canvas != nullptr) {
        return Napi::Boolean::New(env, false); // Already initialized
    }
    
    g_canvas = new Canvas();
    g_renderer = new VectorRenderer();
    
    return Napi::Boolean::New(env, true);
}

// Open native OpenGL window (runs in separate thread)
Napi::Value OpenWindow(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (g_window != nullptr) {
        return Napi::String::New(env, "Window already open");
    }
    
    // Run GLFW window in separate thread
    g_renderThread = new std::thread([]() {
        if (!glfwInit()) {
            return;
        }
        
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        
        g_window = glfwCreateWindow(1280, 720, "Infinite Canvas", nullptr, nullptr);
        if (!g_window) {
            glfwTerminate();
            return;
        }
        
        glfwMakeContextCurrent(g_window);
        glfwSwapInterval(1);
        
        g_renderer->initialize(1280, 720);
        
        // Main render loop
        while (!glfwWindowShouldClose(g_window)) {
            glfwPollEvents();
            g_renderer->beginFrame();
            g_canvas->render(*g_renderer);
            g_renderer->endFrame();
            glfwSwapBuffers(g_window);
        }
        
        glfwTerminate();
        g_window = nullptr;
    });
    
    return Napi::String::New(env, "Window opened");
}

// Save drawing to file
Napi::Value SaveDrawing(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    std::string filepath = info[0].As<Napi::String>().Utf8Value();
    bool success = g_canvas->saveToFile(filepath);
    
    return Napi::Boolean::New(env, success);
}

// Load drawing from file
Napi::Value LoadDrawing(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    std::string filepath = info[0].As<Napi::String>().Utf8Value();
    bool success = g_canvas->loadFromFile(filepath);
    
    return Napi::Boolean::New(env, success);
}

// Export functions to Node.js
Napi::Object InitModule(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "init"),
                Napi::Function::New(env, Init));
    exports.Set(Napi::String::New(env, "openWindow"),
                Napi::Function::New(env, OpenWindow));
    exports.Set(Napi::String::New(env, "saveDrawing"),
                Napi::Function::New(env, SaveDrawing));
    exports.Set(Napi::String::New(env, "loadDrawing"),
                Napi::Function::New(env, LoadDrawing));
    
    return exports;
}

NODE_API_MODULE(infinitecanvas, InitModule)
```

### 3. Create `binding.gyp` (node-gyp config)

```python
{
  "targets": [
    {
      "target_name": "infinitecanvas",
      "sources": [
        "src/node_addon.cpp",
        "src/Canvas.cpp",
        "src/Stroke.cpp",
        "src/BezierSmoother.cpp",
        "src/VectorRenderer.cpp",
        "src/ToolWheel.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "include",
        "/usr/include"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "libraries": [
        "-lGL",
        "-lGLEW",
        "-lglfw",
        "-lglm"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ],
      "cflags": [ "-std=c++17" ],
      "cflags_cc": [ "-std=c++17" ]
    }
  ]
}
```

### 4. Electron App Structure

```
electron-notes-app/
├── package.json
├── main.js                    # Electron main process
├── preload.js                 # Secure bridge
├── renderer.js                # UI logic
├── index.html                 # UI layout
└── build/
    └── Release/
        └── infinitecanvas.node  # Compiled C++ addon
```

**package.json:**
```json
{
  "name": "infinite-notes",
  "version": "1.0.0",
  "main": "main.js",
  "scripts": {
    "build": "node-gyp rebuild",
    "start": "electron ."
  },
  "dependencies": {
    "node-addon-api": "^8.0.0"
  },
  "devDependencies": {
    "electron": "^28.0.0",
    "node-gyp": "^10.0.0"
  }
}
```

**main.js (Electron main process):**
```javascript
const { app, BrowserWindow, ipcMain, dialog } = require('electron');
const path = require('path');
const canvas = require('./build/Release/infinitecanvas.node');

let mainWindow;

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1200,
    height: 800,
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      nodeIntegration: false,
      contextIsolation: true
    }
  });

  mainWindow.loadFile('index.html');
  
  // Initialize C++ canvas
  canvas.init();
}

// IPC Handlers
ipcMain.handle('open-canvas', async () => {
  return canvas.openWindow();
});

ipcMain.handle('save-drawing', async () => {
  const result = await dialog.showSaveDialog({
    filters: [{ name: 'Mind Map', extensions: ['mm'] }]
  });
  
  if (!result.canceled) {
    return canvas.saveDrawing(result.filePath);
  }
  return false;
});

ipcMain.handle('load-drawing', async () => {
  const result = await dialog.showOpenDialog({
    filters: [{ name: 'Mind Map', extensions: ['mm'] }]
  });
  
  if (!result.canceled && result.filePaths.length > 0) {
    return canvas.loadDrawing(result.filePaths[0]);
  }
  return false;
});

app.whenReady().then(createWindow);
```

**preload.js (Secure IPC bridge):**
```javascript
const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('canvasAPI', {
  openCanvas: () => ipcRenderer.invoke('open-canvas'),
  saveDrawing: () => ipcRenderer.invoke('save-drawing'),
  loadDrawing: () => ipcRenderer.invoke('load-drawing')
});
```

**renderer.js (UI logic):**
```javascript
document.getElementById('openCanvasBtn').addEventListener('click', async () => {
  const result = await window.canvasAPI.openCanvas();
  console.log('Canvas opened:', result);
});

document.getElementById('saveBtn').addEventListener('click', async () => {
  const success = await window.canvasAPI.saveDrawing();
  console.log('Save result:', success);
});
```

**index.html (Simple UI):**
```html
<!DOCTYPE html>
<html>
<head>
  <title>Infinite Notes</title>
  <link href="https://cdn.jsdelivr.net/npm/tailwindcss@2.2.19/dist/tailwind.min.css" rel="stylesheet">
</head>
<body class="bg-gray-900 text-white">
  <div class="flex h-screen">
    <!-- Sidebar -->
    <div class="w-64 bg-gray-800 p-4">
      <h2 class="text-xl font-bold mb-4">📝 Notes</h2>
      <button id="newNoteBtn" class="w-full bg-blue-600 p-2 rounded mb-2">
        + New Note
      </button>
      <button id="openCanvasBtn" class="w-full bg-purple-600 p-2 rounded">
        🎨 Open Canvas
      </button>
    </div>
    
    <!-- Main Editor -->
    <div class="flex-1 p-8">
      <h1 class="text-3xl font-bold mb-4">Markdown Editor</h1>
      <textarea 
        class="w-full h-96 bg-gray-800 p-4 rounded"
        placeholder="Write your notes here...">
      </textarea>
      
      <div class="mt-4 space-x-2">
        <button id="saveBtn" class="bg-green-600 px-4 py-2 rounded">
          💾 Save Drawing
        </button>
        <button id="loadBtn" class="bg-yellow-600 px-4 py-2 rounded">
          📂 Load Drawing
        </button>
      </div>
    </div>
  </div>
  
  <script src="renderer.js"></script>
</body>
</html>
```

---

## Build & Run

```bash
# 1. Install dependencies
npm install

# 2. Build C++ addon
npm run build

# 3. Run Electron app
npm start
```

---

## Key Benefits

✅ **Electron UI** - Easy web-based note editor  
✅ **Native C++ Canvas** - Your existing OpenGL renderer  
✅ **Separate Window** - Canvas spawns in its own GLFW window  
✅ **File Dialogs** - Electron's native dialogs (no zenity)  
✅ **Cross-Platform** - Works on Linux, macOS, Windows  

---

## Potential Challenges

⚠️ **Threading** - GLFW window runs in separate thread  
⚠️ **Context Management** - OpenGL context belongs to C++ thread  
⚠️ **IPC Overhead** - Communication between Electron and C++  

---

## Alternative: Offscreen Rendering

Instead of spawning a separate window, you could:
1. Render to an OpenGL framebuffer (offscreen)
2. Send texture data to Electron as image
3. Display in `<canvas>` element

This gives you more control but is more complex.

---

## Next Steps

1. **Remove zenity** from `main.cpp`
2. **Create `src/node_addon.cpp`** with the N-API wrapper
3. **Create `binding.gyp`** for node-gyp
4. **Set up Electron app** structure
5. **Build and test** the integration

Would you like me to create these files for you?
