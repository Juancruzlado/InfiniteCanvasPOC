#pragma once

#include <glm/glm.hpp>

namespace VectorSketch {

// Tool types in the wheel
enum class ToolType {
    BRUSH,
    ERASER
};

// Tool wheel UI state
class ToolWheel {
public:
    ToolWheel();
    
    // Render the tool wheel UI
    void render(int windowWidth, int windowHeight);
    
    // Get current tool
    ToolType getCurrentTool() const { return currentTool; }
    
    // Get current brush width
    float getBrushWidth() const { return brushWidth; }
    
    // Get current color (RGB)
    glm::vec3 getCurrentColor() const { return currentColor; }
    
    // Get effective drawing color (white for eraser, currentColor for brush)
    glm::vec3 getEffectiveColor() const { 
        return (currentTool == ToolType::ERASER) ? glm::vec3(1.0f, 1.0f, 1.0f) : currentColor; 
    }
    
    // Check if mouse is over UI (to prevent drawing)
    bool isMouseOverUI() const { return mouseOverUI; }
    
private:
    ToolType currentTool;
    float brushWidth;
    glm::vec3 currentColor;
    bool mouseOverUI;
    bool wheelVisible;
};

} // namespace VectorSketch
