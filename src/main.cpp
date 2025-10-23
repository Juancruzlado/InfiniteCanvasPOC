#include "Canvas.h"
#include "VectorRenderer.h"
#include "StrokePoint.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>

using namespace VectorSketch;

// Global state
Canvas canvas;
VectorRenderer renderer;
bool isDrawing = false;
glm::vec2 lastMousePos(0.0f);         // Screen space
glm::vec2 lastWorldPos(0.0f);        // World space
glm::vec2 panStart(0.0f);
bool isPanning = false;

auto startTime = std::chrono::high_resolution_clock::now();

float getCurrentTime() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
    return duration.count() / 1000.0f;
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
        if (action == GLFW_PRESS && !isPanning) {
            isDrawing = true;
            
            // Start new stroke with random color for visual variety
            float hue = fmod(getCurrentTime() * 50.0f, 360.0f);
            glm::vec3 color(0.0f, 0.0f, 0.0f); // Default black
            
            // Uncomment for colorful strokes:
            // int colorIndex = static_cast<int>(getCurrentTime() * 2) % 5;
            // if (colorIndex == 0) color = glm::vec3(0.2f, 0.4f, 0.8f); // Blue
            // else if (colorIndex == 1) color = glm::vec3(0.8f, 0.2f, 0.4f); // Red
            // else if (colorIndex == 2) color = glm::vec3(0.2f, 0.7f, 0.3f); // Green
            // else if (colorIndex == 3) color = glm::vec3(0.7f, 0.5f, 0.2f); // Orange
            // else color = glm::vec3(0.5f, 0.2f, 0.7f); // Purple
            
            canvas.beginStroke(color, 3.0f);
            
            // Convert screen to world coordinates
            glm::vec2 worldPos = renderer.screenToWorld(mousePos);
            StrokePoint point(worldPos, 1.0f, 0.0f, 0.0f, getCurrentTime());
            canvas.addPointToCurrentStroke(point);
            lastMousePos = mousePos;
            lastWorldPos = worldPos;
        } else if (action == GLFW_RELEASE) {
            if (isDrawing) {
                canvas.endStroke();
                isDrawing = false;
            }
        }
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE || button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            isPanning = true;
            panStart = mousePos;
        } else if (action == GLFW_RELEASE) {
            isPanning = false;
        }
    }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    glm::vec2 mousePos(static_cast<float>(xpos), static_cast<float>(ypos));
    
    if (isDrawing && !isPanning) {
        // Convert screen to world coordinates
        glm::vec2 worldPos = renderer.screenToWorld(mousePos);
        
        // Simulate pressure based on speed (in world space)
        float deltaTime = 0.016f; // Approximate frame time
        float pressure = simulatePressure(worldPos, lastWorldPos, deltaTime);
        
        // Simulate tilt (can be enhanced with actual stylus input)
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
        if (key == GLFW_KEY_C) {
            // Clear canvas
            canvas.clear();
            std::cout << "Canvas cleared" << std::endl;
        } else if (key == GLFW_KEY_R) {
            // Reset view
            renderer.resetView();
            std::cout << "View reset" << std::endl;
        } else if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
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
    
    std::cout << "=== Vector Sketch POC ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  Left Mouse: Draw strokes" << std::endl;
    std::cout << "  Middle/Right Mouse: Pan canvas" << std::endl;
    std::cout << "  Scroll: Zoom in/out" << std::endl;
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
        // Render
        renderer.beginFrame();
        canvas.render(renderer);
        renderer.endFrame();
        
        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Cleanup
    glfwTerminate();
    return 0;
}
