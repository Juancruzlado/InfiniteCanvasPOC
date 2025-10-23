#include "Canvas.h"
#include <iostream>

namespace VectorSketch {

void Canvas::beginStroke(const glm::vec3& color, float width) {
    currentStroke = std::make_shared<Stroke>();
    currentStroke->setColor(color);
    currentStroke->setBaseWidth(width);
}

void Canvas::addPointToCurrentStroke(const StrokePoint& point) {
    if (currentStroke) {
        currentStroke->addPoint(point);
    }
}

void Canvas::endStroke() {
    if (currentStroke && !currentStroke->isEmpty()) {
        strokes.push_back(currentStroke);
        
        // Initialize history with first stroke if empty
        if (history.empty()) {
            std::vector<std::shared_ptr<Stroke>> initial;
            history.push_back(initial);
            historyIndex = 0;
        }
        
        saveToHistory();  // Save state after completing stroke
    }
    currentStroke = nullptr;
}

void Canvas::clear() {
    strokes.clear();
    currentStroke = nullptr;
    
    // Initialize history if empty
    if (history.empty()) {
        std::vector<std::shared_ptr<Stroke>> initial;
        history.push_back(initial);
        historyIndex = 0;
    }
    
    saveToHistory();  // Save cleared state
}

void Canvas::saveToHistory() {
    // Remove any states after current index (when doing new action after undo)
    if (historyIndex < history.size() - 1) {
        history.erase(history.begin() + historyIndex + 1, history.end());
    }
    
    // Deep copy current strokes
    std::vector<std::shared_ptr<Stroke>> snapshot;
    for (const auto& stroke : strokes) {
        auto strokeCopy = std::make_shared<Stroke>(*stroke);
        snapshot.push_back(strokeCopy);
    }
    
    // Add to history
    history.push_back(snapshot);
    
    // Limit history size
    if (history.size() > MAX_HISTORY) {
        history.erase(history.begin());
    } else {
        historyIndex++;
    }
    
    // Ensure index is within bounds
    if (historyIndex >= history.size()) {
        historyIndex = history.size() - 1;
    }
    
    std::cout << "Saved to history: " << strokes.size() << " strokes, index=" << historyIndex << ", total=" << history.size() << std::endl;
}

void Canvas::loadFromHistory(size_t index) {
    if (index >= history.size()) return;
    
    strokes.clear();
    
    // Deep copy from history
    for (const auto& stroke : history[index]) {
        auto strokeCopy = std::make_shared<Stroke>(*stroke);
        strokes.push_back(strokeCopy);
    }
}

void Canvas::undo() {
    if (history.empty() || historyIndex == 0) {
        std::cout << "Cannot undo: history empty or at beginning" << std::endl;
        return;
    }
    if (!canUndo()) return;
    
    historyIndex--;
    
    // Bounds check
    if (historyIndex >= history.size()) {
        std::cout << "Undo: Index out of bounds, resetting" << std::endl;
        historyIndex = 0;
        return;
    }
    
    std::cout << "Undo: Moving to history index " << historyIndex << " (total: " << history.size() << ")" << std::endl;
    loadFromHistory(historyIndex);
}

void Canvas::redo() {
    if (history.empty()) {
        std::cout << "Cannot redo: history empty" << std::endl;
        return;
    }
    if (!canRedo()) return;
    
    historyIndex++;
    
    // Bounds check
    if (historyIndex >= history.size()) {
        std::cout << "Redo: Index out of bounds, capping" << std::endl;
        historyIndex = history.size() - 1;
        return;
    }
    
    std::cout << "Redo: Moving to history index " << historyIndex << " (total: " << history.size() << ")" << std::endl;
    loadFromHistory(historyIndex);
}

void Canvas::render(VectorRenderer& renderer) {
    // Render all completed strokes
    for (const auto& stroke : strokes) {
        renderer.renderStroke(*stroke);
    }
    
    // Render current stroke being drawn
    if (currentStroke && !currentStroke->isEmpty()) {
        renderer.renderStroke(*currentStroke);
    }
}

} // namespace VectorSketch
