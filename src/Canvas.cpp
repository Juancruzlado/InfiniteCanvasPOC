#include "Canvas.h"

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
    }
    currentStroke = nullptr;
}

void Canvas::clear() {
    strokes.clear();
    currentStroke = nullptr;
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
