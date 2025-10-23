#include "VectorRenderer.h"
#include <iostream>
#include <vector>

namespace VectorSketch {

static const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;

uniform mat4 uMVP;

void main() {
    gl_Position = uMVP * vec4(aPos, 0.0, 1.0);
}
)";

static const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

uniform vec3 uColor;

void main() {
    FragColor = vec4(uColor, 1.0);
}
)";

VectorRenderer::VectorRenderer() 
    : shaderProgram(0), vao(0), vbo(0), 
      windowWidth(800), windowHeight(600),
      viewTransform(1.0f) {
}

VectorRenderer::~VectorRenderer() {
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
    if (shaderProgram) glDeleteProgram(shaderProgram);
}

bool VectorRenderer::initialize(int width, int height) {
    windowWidth = width;
    windowHeight = height;
    
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return false;
    }
    
    // Create shaders
    createShaders();
    
    // Create VAO and VBO
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    // Set up OpenGL state
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    
    updateProjection();
    
    return true;
}

void VectorRenderer::createShaders() {
    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    
    // Check compilation
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex shader compilation failed:\n" << infoLog << std::endl;
    }
    
    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment shader compilation failed:\n" << infoLog << std::endl;
    }
    
    // Link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed:\n" << infoLog << std::endl;
    }
    
    // Clean up
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // Get uniform locations
    uMVP = glGetUniformLocation(shaderProgram, "uMVP");
    uColor = glGetUniformLocation(shaderProgram, "uColor");
}

void VectorRenderer::updateProjection() {
    // Orthographic projection for 2D canvas
    projectionMatrix = glm::ortho(0.0f, (float)windowWidth, 
                                  (float)windowHeight, 0.0f, 
                                  -1.0f, 1.0f);
}

void VectorRenderer::beginFrame() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // White background
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgram(shaderProgram);
}

void VectorRenderer::renderStroke(const Stroke& stroke) {
    if (stroke.isEmpty()) return;
    
    // Smooth the stroke into Bézier curves
    auto segments = BezierSmoother::smooth(stroke);
    if (segments.empty()) return;
    
    // Tesselate into line segments
    auto points = BezierSmoother::tesselate(segments, 15);
    if (points.size() < 2) return;
    
    // Upload to GPU
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec2), 
                 points.data(), GL_DYNAMIC_DRAW);
    
    // Set uniforms
    glm::mat4 mvp = projectionMatrix * viewTransform;
    glUniformMatrix4fv(uMVP, 1, GL_FALSE, &mvp[0][0]);
    
    glm::vec3 color = stroke.getColor();
    glUniform3f(uColor, color.r, color.g, color.b);
    
    // Set line width based on stroke width
    glLineWidth(stroke.getBaseWidth());
    
    // Draw
    glBindVertexArray(vao);
    glDrawArrays(GL_LINE_STRIP, 0, points.size());
    glBindVertexArray(0);
}

void VectorRenderer::endFrame() {
    glUseProgram(0);
}

void VectorRenderer::resize(int width, int height) {
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
    updateProjection();
}

void VectorRenderer::setViewTransform(const glm::mat4& transform) {
    viewTransform = transform;
}

void VectorRenderer::pan(const glm::vec2& delta) {
    // Get current scale from view matrix
    float scaleX = glm::length(glm::vec3(viewTransform[0]));
    
    // Scale the delta by inverse of current zoom
    // When zoomed in 2x, a 10-pixel pan should move 5 world units
    glm::vec2 scaledDelta = delta / scaleX;
    
    viewTransform = glm::translate(viewTransform, glm::vec3(scaledDelta, 0.0f));
}

void VectorRenderer::zoom(float factor, const glm::vec2& center) {
    // Zoom around the cursor position (in screen space)
    // Strategy: Translate so center is at origin, scale, translate back
    
    // Get current scale
    float currentScale = glm::length(glm::vec3(viewTransform[0]));
    
    // Extract current translation
    glm::vec3 currentTranslation(viewTransform[3][0], viewTransform[3][1], 0.0f);
    
    // Calculate new scale
    float newScale = currentScale * factor;
    
    // To keep the point under cursor fixed:
    // worldPos = (screenPos - translation) / scale
    // We want: newWorldPos = oldWorldPos
    // So: (screenPos - newTrans) / newScale = (screenPos - oldTrans) / oldScale
    // Therefore: newTrans = screenPos - (screenPos - oldTrans) * newScale / oldScale
    
    glm::vec2 newTranslation2D = center - (center - glm::vec2(currentTranslation)) * (newScale / currentScale);
    
    // Build new view transform
    viewTransform = glm::mat4(1.0f);
    viewTransform = glm::translate(viewTransform, glm::vec3(newTranslation2D, 0.0f));
    viewTransform = glm::scale(viewTransform, glm::vec3(newScale, newScale, 1.0f));
}

void VectorRenderer::resetView() {
    viewTransform = glm::mat4(1.0f);
}

glm::vec2 VectorRenderer::screenToWorld(const glm::vec2& screenPos) const {
    // Convert screen coordinates to world coordinates
    // Our orthographic projection uses screen pixel coordinates directly,
    // so we only need to invert the view transform, not the projection
    
    glm::mat4 invView = glm::inverse(viewTransform);
    glm::vec4 worldPos = invView * glm::vec4(screenPos.x, screenPos.y, 0.0f, 1.0f);
    
    return glm::vec2(worldPos.x, worldPos.y);
}

} // namespace VectorSketch
