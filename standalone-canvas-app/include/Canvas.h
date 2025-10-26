#pragma once

#include "Stroke.h"
#include "VectorRenderer.h"
#include <vector>
#include <memory>
#include <string>
#include <set>

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
    
    // Selection system
    void selectStrokesInPolygon(const std::vector<glm::vec2>& lassoPoints);
    void clearSelection();
    void moveSelectedStrokes(const glm::vec2& delta);
    bool hasSelection() const { return !selectedStrokes.empty(); }
    const std::set<size_t>& getSelectedStrokes() const { return selectedStrokes; }
    
private:
    void saveToHistory();
    void loadFromHistory(size_t index);
    
    std::vector<std::shared_ptr<Stroke>> strokes;
    std::shared_ptr<Stroke> currentStroke;
    
    // Selection system
    std::set<size_t> selectedStrokes; // Indices of selected strokes
    
    // History for undo/redo (max 7 states)
    static constexpr size_t MAX_HISTORY = 7;
    std::vector<std::vector<std::shared_ptr<Stroke>>> history;
    size_t historyIndex = 0;
    
    // Helper: Check if point is inside polygon (for lasso)
    bool pointInPolygon(const glm::vec2& point, const std::vector<glm::vec2>& polygon) const;
};

} // namespace VectorSketch
