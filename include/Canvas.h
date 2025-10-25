#pragma once

#include "Stroke.h"
#include "VectorRenderer.h"
#include <vector>
#include <memory>
#include <string>

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
    
    // Undo/Redo operations
    void undo();
    void redo();
    bool canUndo() const { return !history.empty() && historyIndex > 0; }
    bool canRedo() const { return !history.empty() && historyIndex < history.size() - 1; }
    
    // Render all strokes
    void render(VectorRenderer& renderer);
    
    // Get stroke count
    size_t getStrokeCount() const { return strokes.size(); }
    
    bool isDrawing() const { return currentStroke != nullptr; }
    
    // File operations
    bool saveToFile(const std::string& filepath);
    bool loadFromFile(const std::string& filepath);
    
private:
    void saveToHistory();
    void loadFromHistory(size_t index);
    
    std::vector<std::shared_ptr<Stroke>> strokes;
    std::shared_ptr<Stroke> currentStroke;
    
    // History for undo/redo (max 7 states)
    static constexpr size_t MAX_HISTORY = 7;
    std::vector<std::vector<std::shared_ptr<Stroke>>> history;
    size_t historyIndex = 0;
};

} // namespace VectorSketch
