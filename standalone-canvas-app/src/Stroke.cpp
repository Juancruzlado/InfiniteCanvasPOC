#include "Stroke.h"

namespace VectorSketch {

void Stroke::addPoint(const StrokePoint& point) {
    points.push_back(point);
}

void Stroke::clear() {
    points.clear();
}

void Stroke::movePoints(const glm::vec2& delta) {
    for (auto& point : points) {
        point.position += delta;
    }
}

} // namespace VectorSketch
