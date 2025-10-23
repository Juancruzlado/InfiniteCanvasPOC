#pragma once

#include "Stroke.h"
#include "VectorRenderer.h"
#include <vector>
#include <memory>

namespace VectorSketch {

// Infinite canvas that manages all strokes
class Canvas {
public:
    Canvas() = default;
    
    // Start a new stroke
    void beginStroke(const glm::vec3& color = glm::vec3(0.0f, 0.0f, 0.0f), float width = 2.0f);
    
    // Add point to current stroke
    void addPointToCurrentStroke(const StrokePoint& point);
    
    // End current stroke
    void endStroke();
    
    // Clear all strokes
    void clear();
    
    // Render all strokes
    void render(VectorRenderer& renderer);
    
    // Get stroke count
    size_t getStrokeCount() const { return strokes.size(); }
    
    bool isDrawing() const { return currentStroke != nullptr; }
    
private:
    std::vector<std::shared_ptr<Stroke>> strokes;
    std::shared_ptr<Stroke> currentStroke;
};

} // namespace VectorSketch
