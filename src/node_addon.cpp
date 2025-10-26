/**
 * Node.js Native Addon (N-API) Wrapper
 * Exposes C++ Infinite Canvas to Electron
 * 
 * Usage from JavaScript:
 *   const canvas = require('./build/Release/infinitecanvas.node');
 *   canvas.init();
 *   canvas.openWindow();
 */

#include <napi.h>
#include "Canvas.h"
#include "VectorRenderer.h"
#include "ToolWheel.h"
#include "StrokePoint.h"
#include <GLFW/glfw3.h>
#include <thread>
#include <iostream>
#include <chrono>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using namespace VectorSketch;

// ============================================================================
// Global State
// ============================================================================

static Canvas* g_canvas = nullptr;
static VectorRenderer* g_renderer = nullptr;
static ToolWheel* g_toolWheel = nullptr;
static GLFWwindow* g_window = nullptr;
static std::thread* g_renderThread = nullptr;

// Drawing state
static bool isDrawing = false;
static glm::vec2 lastMousePos(0.0f);
static glm::vec2 lastWorldPos(0.0f);
static bool isPanning = false;
static glm::vec2 panStart(0.0f);

// Lasso tool state
static std::vector<glm::vec2> lassoPoints;
static bool isDrawingLasso = false;
static bool isMovingSelection = false;
static glm::vec2 moveStartPos(0.0f);

// Timing
static auto startTime = std::chrono::high_resolution_clock::now();

// ============================================================================
// Helper Functions
// ============================================================================

float getCurrentTime() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
    return duration.count() / 1000.0f;
}

float simulatePressure(const glm::vec2& currentPos, const glm::vec2& lastPos, float deltaTime) {
    float distance = glm::length(currentPos - lastPos);
    float speed = distance / (deltaTime + 0.001f);
    float pressure = 1.0f - glm::clamp(speed / 1000.0f, 0.0f, 0.7f);
    return glm::clamp(pressure, 0.3f, 1.0f);
}

// ============================================================================
// GLFW Callbacks
// ============================================================================

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    glm::vec2 mousePos(static_cast<float>(xpos), static_cast<float>(ypos));
    
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS && !isPanning && !g_toolWheel->isMouseOverUI()) {
            ToolType currentTool = g_toolWheel->getCurrentTool();
            
            if (currentTool == ToolType::LASSO) {
                if (g_canvas->hasSelection()) {
                    isMovingSelection = true;
                    moveStartPos = g_renderer->screenToWorld(mousePos);
                } else {
                    isDrawingLasso = true;
                    lassoPoints.clear();
                    lassoPoints.push_back(mousePos);
                }
            } else {
                isDrawing = true;
                g_canvas->clearSelection();
                
                glm::vec3 color = g_toolWheel->getEffectiveColor();
                float brushWidth = g_toolWheel->getBrushWidth();
                
                g_canvas->beginStroke(color, brushWidth);
                
                glm::vec2 worldPos = g_renderer->screenToWorld(mousePos);
                StrokePoint point(worldPos, 1.0f, 0.0f, 0.0f, getCurrentTime());
                g_canvas->addPointToCurrentStroke(point);
                lastMousePos = mousePos;
                lastWorldPos = worldPos;
            }
        } else if (action == GLFW_RELEASE) {
            if (isDrawing) {
                g_canvas->endStroke();
                isDrawing = false;
            } else if (isDrawingLasso) {
                std::vector<glm::vec2> worldLassoPoints;
                for (const auto& screenPt : lassoPoints) {
                    worldLassoPoints.push_back(g_renderer->screenToWorld(screenPt));
                }
                g_canvas->selectStrokesInPolygon(worldLassoPoints);
                isDrawingLasso = false;
                lassoPoints.clear();
            } else if (isMovingSelection) {
                isMovingSelection = false;
            }
        }
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE || button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            if (button == GLFW_MOUSE_BUTTON_RIGHT && g_canvas->hasSelection()) {
                g_canvas->clearSelection();
            } else {
                isPanning = true;
                panStart = mousePos;
            }
        } else if (action == GLFW_RELEASE) {
            isPanning = false;
        }
    }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    glm::vec2 mousePos(static_cast<float>(xpos), static_cast<float>(ypos));
    
    if (isDrawing && !isPanning) {
        glm::vec2 worldPos = g_renderer->screenToWorld(mousePos);
        float deltaTime = 0.016f;
        float pressure = simulatePressure(worldPos, lastWorldPos, deltaTime);
        
        glm::vec2 direction = worldPos - lastWorldPos;
        float tiltX = 0.0f;
        float tiltY = 0.0f;
        
        if (glm::length(direction) > 0.1f) {
            direction = glm::normalize(direction);
            tiltX = direction.x * 0.3f;
            tiltY = direction.y * 0.3f;
        }
        
        StrokePoint point(worldPos, pressure, tiltX, tiltY, getCurrentTime());
        g_canvas->addPointToCurrentStroke(point);
        lastWorldPos = worldPos;
    } else if (isDrawingLasso) {
        if (glm::distance(mousePos, lassoPoints.back()) > 3.0f) {
            lassoPoints.push_back(mousePos);
        }
    } else if (isMovingSelection && !isPanning) {
        glm::vec2 worldPos = g_renderer->screenToWorld(mousePos);
        glm::vec2 delta = worldPos - moveStartPos;
        
        if (glm::length(delta) > 0.001f) {
            g_canvas->moveSelectedStrokes(delta);
            moveStartPos = worldPos;
        }
    } else if (isPanning) {
        glm::vec2 delta = mousePos - panStart;
        g_renderer->pan(delta);
        panStart = mousePos;
    }
    
    lastMousePos = mousePos;
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    glm::vec2 mousePos(static_cast<float>(xpos), static_cast<float>(ypos));
    
    float zoomFactor = 1.0f + static_cast<float>(yoffset) * 0.1f;
    g_renderer->zoom(zoomFactor, mousePos);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        bool ctrlPressed = (mods & GLFW_MOD_CONTROL) != 0;
        bool shiftPressed = (mods & GLFW_MOD_SHIFT) != 0;
        
        if (ctrlPressed && shiftPressed && key == GLFW_KEY_Z) {
            if (g_canvas->canRedo()) {
                g_canvas->redo();
            }
        } else if (ctrlPressed && key == GLFW_KEY_Z) {
            if (g_canvas->canUndo()) {
                g_canvas->undo();
            }
        } else if (key == GLFW_KEY_C) {
            g_canvas->clear();
        } else if (key == GLFW_KEY_R) {
            g_renderer->resetView();
        } else if (key == GLFW_KEY_ESCAPE) {
            if (g_canvas->hasSelection()) {
                g_canvas->clearSelection();
            } else {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
        }
    }
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    g_renderer->resize(width, height);
}

// ============================================================================
// N-API Exported Functions
// ============================================================================

/**
 * Initialize canvas and renderer
 * JavaScript: canvas.init()
 */
Napi::Value Init(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (g_canvas != nullptr) {
        return Napi::Boolean::New(env, false); // Already initialized
    }
    
    g_canvas = new Canvas();
    g_renderer = new VectorRenderer();
    g_toolWheel = new ToolWheel();
    
    std::cout << "✓ Canvas initialized" << std::endl;
    return Napi::Boolean::New(env, true);
}

/**
 * Open native OpenGL window (spawns in separate thread)
 * JavaScript: canvas.openWindow()
 */
Napi::Value OpenWindow(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (g_window != nullptr) {
        return Napi::String::New(env, "Window already open");
    }
    
    if (g_canvas == nullptr || g_renderer == nullptr) {
        Napi::Error::New(env, "Canvas not initialized. Call init() first.")
            .ThrowAsJavaScriptException();
        return env.Null();
    }
    
    // Run GLFW window in separate thread to avoid blocking Node.js event loop
    g_renderThread = new std::thread([]() {
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return;
        }
        
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 4);
        
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
        
        g_window = glfwCreateWindow(1280, 720, "Infinite Canvas", nullptr, nullptr);
        if (!g_window) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return;
        }
        
        glfwMakeContextCurrent(g_window);
        glfwSwapInterval(1);
        
        // Set callbacks
        glfwSetMouseButtonCallback(g_window, mouseButtonCallback);
        glfwSetCursorPosCallback(g_window, cursorPosCallback);
        glfwSetScrollCallback(g_window, scrollCallback);
        glfwSetKeyCallback(g_window, keyCallback);
        glfwSetFramebufferSizeCallback(g_window, framebufferSizeCallback);
        
        if (!g_renderer->initialize(1280, 720)) {
            std::cerr << "Failed to initialize renderer" << std::endl;
            glfwTerminate();
            return;
        }
        
        // Initialize ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        
        // Setup ImGui style
        ImGui::StyleColorsDark();
        
        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(g_window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
        
        std::cout << "✓ Canvas window opened" << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "  Left Mouse: Draw" << std::endl;
        std::cout << "  Middle/Right Mouse: Pan" << std::endl;
        std::cout << "  Scroll: Zoom" << std::endl;
        std::cout << "  Ctrl+Z: Undo" << std::endl;
        std::cout << "  ESC: Close window" << std::endl;
        
        // Main render loop
        while (!glfwWindowShouldClose(g_window)) {
            glfwPollEvents();
            
            // Start ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            
            int display_w, display_h;
            glfwGetFramebufferSize(g_window, &display_w, &display_h);
            
            g_renderer->beginFrame();
            g_canvas->render(*g_renderer);
            g_renderer->endFrame();
            
            g_toolWheel->render(display_w, display_h);
            
            // Render ImGui
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            
            glfwSwapBuffers(g_window);
        }
        
        std::cout << "✓ Canvas window closed" << std::endl;
        
        // Cleanup ImGui
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        
        glfwTerminate();
        g_window = nullptr;
    });
    
    return Napi::String::New(env, "Window opened in separate thread");
}

/**
 * Save drawing to file
 * JavaScript: canvas.saveDrawing('/path/to/file.mm')
 */
Napi::Value SaveDrawing(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (g_canvas == nullptr) {
        Napi::Error::New(env, "Canvas not initialized").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    std::string filepath = info[0].As<Napi::String>().Utf8Value();
    bool success = g_canvas->saveToFile(filepath);
    
    if (success) {
        std::cout << "✓ Saved: " << filepath << std::endl;
    } else {
        std::cerr << "✗ Failed to save: " << filepath << std::endl;
    }
    
    return Napi::Boolean::New(env, success);
}

/**
 * Load drawing from file
 * JavaScript: canvas.loadDrawing('/path/to/file.mm')
 */
Napi::Value LoadDrawing(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (g_canvas == nullptr) {
        Napi::Error::New(env, "Canvas not initialized").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    std::string filepath = info[0].As<Napi::String>().Utf8Value();
    bool success = g_canvas->loadFromFile(filepath);
    
    if (success) {
        std::cout << "✓ Loaded: " << filepath << std::endl;
    } else {
        std::cerr << "✗ Failed to load: " << filepath << std::endl;
    }
    
    return Napi::Boolean::New(env, success);
}

/**
 * Clear canvas
 * JavaScript: canvas.clear()
 */
Napi::Value Clear(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (g_canvas == nullptr) {
        Napi::Error::New(env, "Canvas not initialized").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    g_canvas->clear();
    std::cout << "✓ Canvas cleared" << std::endl;
    
    return Napi::Boolean::New(env, true);
}

/**
 * Check if window is open
 * JavaScript: canvas.isWindowOpen()
 */
Napi::Value IsWindowOpen(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::Boolean::New(env, g_window != nullptr);
}

// ============================================================================
// Module Initialization
// ============================================================================

Napi::Object InitModule(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "init"),
                Napi::Function::New(env, Init));
    exports.Set(Napi::String::New(env, "openWindow"),
                Napi::Function::New(env, OpenWindow));
    exports.Set(Napi::String::New(env, "saveDrawing"),
                Napi::Function::New(env, SaveDrawing));
    exports.Set(Napi::String::New(env, "loadDrawing"),
                Napi::Function::New(env, LoadDrawing));
    exports.Set(Napi::String::New(env, "clear"),
                Napi::Function::New(env, Clear));
    exports.Set(Napi::String::New(env, "isWindowOpen"),
                Napi::Function::New(env, IsWindowOpen));
    
    return exports;
}

NODE_API_MODULE(infinitecanvas, InitModule)
