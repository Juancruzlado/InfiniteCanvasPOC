#pragma once

#include "Stroke.h"
#include "BezierSmoother.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <vector>
#include <memory>

namespace VectorSketch {

// GPU-accelerated vector renderer using OpenGL
class VectorRenderer {
public:
    VectorRenderer();
    ~VectorRenderer();
    
    // Initialize renderer with window dimensions
    bool initialize(int width, int height);
    
    // Begin frame rendering
    void beginFrame();
    
    // Render a stroke
    void renderStroke(const Stroke& stroke);
    
    // End frame rendering
    void endFrame();
    
    // Update viewport on window resize
    void resize(int width, int height);
    
    // Camera controls for infinite canvas
    void setViewTransform(const glm::mat4& transform);
    void pan(const glm::vec2& delta);
    void zoom(float factor, const glm::vec2& center);
    void resetView();
    
    glm::mat4 getViewTransform() const { return viewTransform; }
    
    // Coordinate transformation
    glm::vec2 screenToWorld(const glm::vec2& screenPos) const;
    
private:
    void createShaders();
    void updateProjection();
    
    GLuint shaderProgram;
    GLuint vao, vbo;
    
    int windowWidth, windowHeight;
    glm::mat4 projectionMatrix;
    glm::mat4 viewTransform;
    
    // Shader uniform locations
    GLint uMVP;
    GLint uColor;
    GLint uLineWidth;
};

} // namespace VectorSketch
