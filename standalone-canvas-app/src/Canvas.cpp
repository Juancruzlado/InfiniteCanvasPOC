#include "Canvas.h"
#include <iostream>
#include <fstream>
#include <cstring>

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

bool Canvas::saveToFile(const std::string& filepath) {
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filepath << std::endl;
        return false;
    }
    
    try {
        // Write header: magic number + version
        const char magic[4] = {'M', 'M', 'V', 'S'}; // Mind Map Vector Sketch
        file.write(magic, 4);
        
        uint32_t version = 1;
        file.write(reinterpret_cast<const char*>(&version), sizeof(version));
        
        // Write number of strokes
        uint32_t numStrokes = static_cast<uint32_t>(strokes.size());
        file.write(reinterpret_cast<const char*>(&numStrokes), sizeof(numStrokes));
        
        // Write each stroke
        for (const auto& stroke : strokes) {
            // Write color (3 floats)
            glm::vec3 color = stroke->getColor();
            file.write(reinterpret_cast<const char*>(&color.r), sizeof(float));
            file.write(reinterpret_cast<const char*>(&color.g), sizeof(float));
            file.write(reinterpret_cast<const char*>(&color.b), sizeof(float));
            
            // Write base width
            float width = stroke->getBaseWidth();
            file.write(reinterpret_cast<const char*>(&width), sizeof(float));
            
            // Write number of points
            uint32_t numPoints = static_cast<uint32_t>(stroke->getPointCount());
            file.write(reinterpret_cast<const char*>(&numPoints), sizeof(numPoints));
            
            // Write each point
            const auto& points = stroke->getPoints();
            for (const auto& point : points) {
                file.write(reinterpret_cast<const char*>(&point.position.x), sizeof(float));
                file.write(reinterpret_cast<const char*>(&point.position.y), sizeof(float));
                file.write(reinterpret_cast<const char*>(&point.pressure), sizeof(float));
                file.write(reinterpret_cast<const char*>(&point.tiltX), sizeof(float));
                file.write(reinterpret_cast<const char*>(&point.tiltY), sizeof(float));
                file.write(reinterpret_cast<const char*>(&point.timestamp), sizeof(float));
            }
        }
        
        file.close();
        std::cout << "Saved " << numStrokes << " strokes to " << filepath << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error saving file: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

bool Canvas::loadFromFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for reading: " << filepath << std::endl;
        return false;
    }
    
    try {
        // Read and verify header
        char magic[4];
        file.read(magic, 4);
        if (magic[0] != 'M' || magic[1] != 'M' || magic[2] != 'V' || magic[3] != 'S') {
            std::cerr << "Invalid file format (bad magic number)" << std::endl;
            file.close();
            return false;
        }
        
        uint32_t version;
        file.read(reinterpret_cast<char*>(&version), sizeof(version));
        if (version != 1) {
            std::cerr << "Unsupported file version: " << version << std::endl;
            file.close();
            return false;
        }
        
        // Read number of strokes
        uint32_t numStrokes;
        file.read(reinterpret_cast<char*>(&numStrokes), sizeof(numStrokes));
        
        // Clear current strokes
        strokes.clear();
        
        // Read each stroke
        for (uint32_t i = 0; i < numStrokes; ++i) {
            auto stroke = std::make_shared<Stroke>();
            
            // Read color
            glm::vec3 color;
            file.read(reinterpret_cast<char*>(&color.r), sizeof(float));
            file.read(reinterpret_cast<char*>(&color.g), sizeof(float));
            file.read(reinterpret_cast<char*>(&color.b), sizeof(float));
            stroke->setColor(color);
            
            // Read base width
            float width;
            file.read(reinterpret_cast<char*>(&width), sizeof(float));
            stroke->setBaseWidth(width);
            
            // Read number of points
            uint32_t numPoints;
            file.read(reinterpret_cast<char*>(&numPoints), sizeof(numPoints));
            
            // Read each point
            for (uint32_t j = 0; j < numPoints; ++j) {
                StrokePoint point;
                file.read(reinterpret_cast<char*>(&point.position.x), sizeof(float));
                file.read(reinterpret_cast<char*>(&point.position.y), sizeof(float));
                file.read(reinterpret_cast<char*>(&point.pressure), sizeof(float));
                file.read(reinterpret_cast<char*>(&point.tiltX), sizeof(float));
                file.read(reinterpret_cast<char*>(&point.tiltY), sizeof(float));
                file.read(reinterpret_cast<char*>(&point.timestamp), sizeof(float));
                stroke->addPoint(point);
            }
            
            strokes.push_back(stroke);
        }
        
        file.close();
        
        // Initialize history after loading
        history.clear();
        std::vector<std::shared_ptr<Stroke>> initial;
        for (const auto& stroke : strokes) {
            auto strokeCopy = std::make_shared<Stroke>(*stroke);
            initial.push_back(strokeCopy);
        }
        history.push_back(initial);
        historyIndex = 0;
        
        std::cout << "Loaded " << numStrokes << " strokes from " << filepath << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading file: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

// Point-in-polygon test using ray casting algorithm
bool Canvas::pointInPolygon(const glm::vec2& point, const std::vector<glm::vec2>& polygon) const {
    if (polygon.size() < 3) return false;
    
    bool inside = false;
    size_t j = polygon.size() - 1;
    
    for (size_t i = 0; i < polygon.size(); i++) {
        if ((polygon[i].y > point.y) != (polygon[j].y > point.y) &&
            (point.x < (polygon[j].x - polygon[i].x) * (point.y - polygon[i].y) / 
                       (polygon[j].y - polygon[i].y) + polygon[i].x)) {
            inside = !inside;
        }
        j = i;
    }
    
    return inside;
}

void Canvas::selectStrokesInPolygon(const std::vector<glm::vec2>& lassoPoints) {
    selectedStrokes.clear();
    
    std::cout << "Lasso selection: checking " << strokes.size() << " total strokes" << std::endl;
    
    // Check each stroke
    for (size_t i = 0; i < strokes.size(); ++i) {
        const auto& stroke = strokes[i];
        const auto& points = stroke->getPoints();
        
        // If any point of the stroke is inside the lasso, select it
        for (const auto& point : points) {
            if (pointInPolygon(point.position, lassoPoints)) {
                selectedStrokes.insert(i);
                std::cout << "  → Stroke " << i << " selected (has " << points.size() << " points)" << std::endl;
                break; // Found at least one point inside, select this stroke
            }
        }
    }
    
    std::cout << "✓ Selected " << selectedStrokes.size() << " out of " << strokes.size() << " stroke(s)" << std::endl;
}

void Canvas::clearSelection() {
    selectedStrokes.clear();
}

void Canvas::moveSelectedStrokes(const glm::vec2& delta) {
    if (selectedStrokes.empty()) return;
    
    std::cout << "Moving " << selectedStrokes.size() << " strokes by delta(" 
              << delta.x << ", " << delta.y << ")" << std::endl;
    
    // Move all points in selected strokes
    for (size_t idx : selectedStrokes) {
        if (idx < strokes.size()) {
            strokes[idx]->movePoints(delta);
        }
    }
}

} // namespace VectorSketch
