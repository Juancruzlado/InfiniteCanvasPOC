#include "Stroke.h"

namespace VectorSketch {

void Stroke::addPoint(const StrokePoint& point) {
    points.push_back(point);
}

void Stroke::clear() {
    points.clear();
}

} // namespace VectorSketch
