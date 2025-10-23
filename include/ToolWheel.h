#pragma once

#include <glm/glm.hpp>

namespace VectorSketch {

// Tool types in the wheel
enum class ToolType {
    BRUSH,
    // Future tools can be added here:
    // ERASER,
    // SELECTOR,
    // etc.
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
    
    // Check if mouse is over UI (to prevent drawing)
    bool isMouseOverUI() const { return mouseOverUI; }
    
private:
    ToolType currentTool;
    float brushWidth;
    bool mouseOverUI;
    bool wheelVisible;
};

} // namespace VectorSketch
