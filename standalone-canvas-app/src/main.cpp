#include "Canvas.h"
#include "VectorRenderer.h"
#include "StrokePoint.h"
#include "ToolWheel.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>
#include <cstdio>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using namespace VectorSketch;

// Global state
Canvas canvas;
VectorRenderer renderer;
ToolWheel toolWheel;
bool isDrawing = false;
glm::vec2 lastMousePos(0.0f);         // Screen space
glm::vec2 lastWorldPos(0.0f);        // World space
glm::vec2 panStart(0.0f);
bool isPanning = false;

// File dialog state
bool showSaveDialog = false;
bool showOpenDialog = false;

// Lasso tool state
std::vector<glm::vec2> lassoPoints; // Screen space points for lasso drawing
bool isDrawingLasso = false;
bool isMovingSelection = false;
glm::vec2 moveStartPos(0.0f);

auto startTime = std::chrono::high_resolution_clock::now();

float getCurrentTime() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
    return duration.count() / 1000.0f;
}

// Open native file dialog using zenity (Ubuntu/GNOME)
std::string openNativeFileDialog(bool isSave) {
    std::string command;
    
    if (isSave) {
        // Save dialog with .mm filter
        command = "zenity --file-selection --save --confirm-overwrite "
                  "--title=\"Guardar Dibujo\" "
                  "--filename=\"drawing.mm\" "
                  "--file-filter=\"Mind Map Files (*.mm) | *.mm\" "
                  "--file-filter=\"All Files | *\" "
                  "2>/dev/null";
    } else {
        // Open dialog with .mm filter
        command = "zenity --file-selection "
                  "--title=\"Abrir Dibujo\" "
                  "--file-filter=\"Mind Map Files (*.mm) | *.mm\" "
                  "--file-filter=\"All Files | *\" "
                  "2>/dev/null";
    }
    
    // Execute command and capture output
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        std::cerr << "Error: No se pudo abrir zenity. Asegúrate de que está instalado." << std::endl;
        return "";
    }
    
    char buffer[512];
    std::string result = "";
    
    if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result = buffer;
        // Remove trailing newline
        if (!result.empty() && result[result.length() - 1] == '\n') {
            result.erase(result.length() - 1);
        }
    }
    
    pclose(pipe);
    return result;
}

// Process file dialog requests
void processFileDialogs() {
    static bool processingDialog = false;
    
    if (showSaveDialog && !processingDialog) {
        processingDialog = true;
        showSaveDialog = false;
        
        std::string filepath = openNativeFileDialog(true);
        
        if (!filepath.empty()) {
            // Add .mm extension if not present
            if (filepath.length() < 3 || filepath.substr(filepath.length() - 3) != ".mm") {
                filepath += ".mm";
            }
            
            if (canvas.saveToFile(filepath)) {
                std::cout << "✓ Archivo guardado: " << filepath << std::endl;
            } else {
                std::cerr << "✗ Error al guardar el archivo" << std::endl;
            }
        } else {
            std::cout << "Guardado cancelado" << std::endl;
        }
        
        processingDialog = false;
    }
    
    if (showOpenDialog && !processingDialog) {
        processingDialog = true;
        showOpenDialog = false;
        
        std::string filepath = openNativeFileDialog(false);
        
        if (!filepath.empty()) {
            if (canvas.loadFromFile(filepath)) {
                std::cout << "✓ Archivo cargado: " << filepath << std::endl;
            } else {
                std::cerr << "✗ Error al cargar el archivo" << std::endl;
            }
        } else {
            std::cout << "Carga cancelada" << std::endl;
        }
        
        processingDialog = false;
    }
}

// Simulate pressure based on mouse speed (for demo purposes)
float simulatePressure(const glm::vec2& currentPos, const glm::vec2& lastPos, float deltaTime) {
    float distance = glm::length(currentPos - lastPos);
    float speed = distance / (deltaTime + 0.001f);
    
    // Slower movement = more pressure
    float pressure = 1.0f - glm::clamp(speed / 1000.0f, 0.0f, 0.7f);
    return glm::clamp(pressure, 0.3f, 1.0f);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    glm::vec2 mousePos(static_cast<float>(xpos), static_cast<float>(ypos));
    
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS && !isPanning && !toolWheel.isMouseOverUI()) {
            ToolType currentTool = toolWheel.getCurrentTool();
            
            if (currentTool == ToolType::LASSO) {
                // Lasso mode
                if (canvas.hasSelection()) {
                    // If there's a selection, start moving it
                    isMovingSelection = true;
                    moveStartPos = renderer.screenToWorld(mousePos);
                    std::cout << "Started moving selection" << std::endl;
                } else {
                    // Start drawing lasso
                    isDrawingLasso = true;
                    lassoPoints.clear();
                    lassoPoints.push_back(mousePos);
                    std::cout << "Started drawing lasso" << std::endl;
                }
            } else {
                // Brush or Eraser mode - normal drawing
                isDrawing = true;
                
                // Clear selection when starting to draw
                canvas.clearSelection();
                
                // Start new stroke with current tool settings
                glm::vec3 color = toolWheel.getEffectiveColor();
                float brushWidth = toolWheel.getBrushWidth();
                
                canvas.beginStroke(color, brushWidth);
                
                // Convert screen to world coordinates
                glm::vec2 worldPos = renderer.screenToWorld(mousePos);
                StrokePoint point(worldPos, 1.0f, 0.0f, 0.0f, getCurrentTime());
                canvas.addPointToCurrentStroke(point);
                lastMousePos = mousePos;
                lastWorldPos = worldPos;
            }
        } else if (action == GLFW_RELEASE) {
            if (isDrawing) {
                canvas.endStroke();
                isDrawing = false;
            } else if (isDrawingLasso) {
                // Complete lasso and select strokes
                std::vector<glm::vec2> worldLassoPoints;
                for (const auto& screenPt : lassoPoints) {
                    worldLassoPoints.push_back(renderer.screenToWorld(screenPt));
                }
                canvas.selectStrokesInPolygon(worldLassoPoints);
                isDrawingLasso = false;
                lassoPoints.clear();
            } else if (isMovingSelection) {
                // Finish moving selection
                isMovingSelection = false;
                std::cout << "Finished moving selection" << std::endl;
            }
        }
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE || button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            // Don't start panning if right-click is clearing a selection
            if (button == GLFW_MOUSE_BUTTON_RIGHT && canvas.hasSelection()) {
                canvas.clearSelection();
                std::cout << "Selection cleared with right-click" << std::endl;
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
        // Normal drawing (Brush/Eraser)
        glm::vec2 worldPos = renderer.screenToWorld(mousePos);
        
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
        canvas.addPointToCurrentStroke(point);
        lastWorldPos = worldPos;
    } else if (isDrawingLasso) {
        // Add points to lasso path
        if (glm::distance(mousePos, lassoPoints.back()) > 3.0f) { // Sample every 3 pixels
            lassoPoints.push_back(mousePos);
        }
    } else if (isMovingSelection && !isPanning) {
        // Move selected strokes
        glm::vec2 worldPos = renderer.screenToWorld(mousePos);
        glm::vec2 delta = worldPos - moveStartPos;
        
        if (glm::length(delta) > 0.001f) { // Only move if delta is significant
            canvas.moveSelectedStrokes(delta);
            moveStartPos = worldPos;
        }
    } else if (isPanning) {
        glm::vec2 delta = mousePos - panStart;
        renderer.pan(delta);
        panStart = mousePos;
    }
    
    lastMousePos = mousePos;
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    glm::vec2 mousePos(static_cast<float>(xpos), static_cast<float>(ypos));
    
    float zoomFactor = 1.0f + static_cast<float>(yoffset) * 0.1f;
    renderer.zoom(zoomFactor, mousePos);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        // Check for Ctrl modifier
        bool ctrlPressed = (mods & GLFW_MOD_CONTROL) != 0;
        bool shiftPressed = (mods & GLFW_MOD_SHIFT) != 0;
        
        if (ctrlPressed && key == GLFW_KEY_S) {
            // Ctrl + S: Save
            showSaveDialog = true;
            std::cout << "Open save dialog" << std::endl;
        } else if (ctrlPressed && key == GLFW_KEY_O) {
            // Ctrl + O: Open
            showOpenDialog = true;
            std::cout << "Open load dialog" << std::endl;
        } else if (ctrlPressed && shiftPressed && key == GLFW_KEY_Z) {
            // Ctrl + Shift + Z: Redo
            if (canvas.canRedo()) {
                canvas.redo();
                std::cout << "Redo" << std::endl;
            }
        } else if (ctrlPressed && key == GLFW_KEY_Z) {
            // Ctrl + Z: Undo
            if (canvas.canUndo()) {
                canvas.undo();
                std::cout << "Undo" << std::endl;
            }
        } else if (key == GLFW_KEY_C) {
            // Clear canvas
            canvas.clear();
            std::cout << "Canvas cleared" << std::endl;
        } else if (key == GLFW_KEY_R) {
            // Reset view
            renderer.resetView();
            std::cout << "View reset" << std::endl;
        } else if (key == GLFW_KEY_ESCAPE) {
            // ESC: Clear selection or exit
            if (canvas.hasSelection()) {
                canvas.clearSelection();
                std::cout << "Selection cleared" << std::endl;
            } else {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
        }
    }
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    renderer.resize(width, height);
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x MSAA
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Vector Sketch - Infinite Canvas", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    
    // Set callbacks
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    
    // Initialize renderer
    if (!renderer.initialize(1280, 720)) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Setup ImGui style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    std::cout << "=== Vector Sketch POC ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  Left Mouse: Draw strokes" << std::endl;
    std::cout << "  Middle/Right Mouse: Pan canvas" << std::endl;
    std::cout << "  Scroll: Zoom in/out" << std::endl;
    std::cout << "  Ctrl+S: Save to .mm file" << std::endl;
    std::cout << "  Ctrl+O: Open .mm file" << std::endl;
    std::cout << "  Ctrl+Z: Undo (up to 7 actions)" << std::endl;
    std::cout << "  Ctrl+Shift+Z: Redo" << std::endl;
    std::cout << "  C: Clear canvas" << std::endl;
    std::cout << "  R: Reset view" << std::endl;
    std::cout << "  ESC: Exit" << std::endl;
    std::cout << std::endl;
    std::cout << "Features:" << std::endl;
    std::cout << "  ✓ Stroke sampling with pressure/tilt simulation" << std::endl;
    std::cout << "  ✓ Bézier curve smoothing" << std::endl;
    std::cout << "  ✓ GPU-accelerated rendering (OpenGL)" << std::endl;
    std::cout << "  ✓ Infinite canvas with pan/zoom" << std::endl;
    std::cout << std::endl;
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Poll events
        glfwPollEvents();
        
        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // Get window size for UI
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        
        // Render canvas
        renderer.beginFrame();
        canvas.render(renderer);
        renderer.endFrame();
        
        // Render UI on top
        toolWheel.render(display_w, display_h);
        
        // Render lasso and selection visualization
        ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
        
        // Draw lasso while being drawn
        if (isDrawingLasso && lassoPoints.size() > 1) {
            for (size_t i = 0; i < lassoPoints.size() - 1; i++) {
                draw_list->AddLine(
                    ImVec2(lassoPoints[i].x, lassoPoints[i].y),
                    ImVec2(lassoPoints[i+1].x, lassoPoints[i+1].y),
                    IM_COL32(100, 200, 255, 255), 2.0f
                );
            }
            // Close the loop
            if (lassoPoints.size() > 2) {
                draw_list->AddLine(
                    ImVec2(lassoPoints.back().x, lassoPoints.back().y),
                    ImVec2(lassoPoints.front().x, lassoPoints.front().y),
                    IM_COL32(100, 200, 255, 150), 2.0f
                );
            }
        }
        
        // Draw selection highlight (blue outline around selected strokes)
        if (canvas.hasSelection()) {
            const auto& selectedIndices = canvas.getSelectedStrokes();
            for (size_t idx : selectedIndices) {
                // Get stroke and draw its points with highlight
                // Note: This is a simple visualization, can be enhanced
            }
            
            // Draw instruction text
            ImVec2 textPos(10, display_h - 30);
            draw_list->AddText(textPos, IM_COL32(100, 200, 255, 255), 
                               "Selection active - Click and drag to move");
        }
        
        // Process file dialog requests (native system dialogs)
        processFileDialogs();
        
        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        // Swap buffers
        glfwSwapBuffers(window);
    }
    
    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    // Cleanup GLFW
    glfwTerminate();
    return 0;
}
