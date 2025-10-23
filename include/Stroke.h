#pragma once

#include "StrokePoint.h"
#include <vector>
#include <glm/glm.hpp>

namespace VectorSketch {

// Represents a complete stroke with sampled points
class Stroke {
public:
    Stroke() = default;
    
    void addPoint(const StrokePoint& point);
    void clear();
    
    const std::vector<StrokePoint>& getPoints() const { return points; }
    bool isEmpty() const { return points.empty(); }
    size_t getPointCount() const { return points.size(); }
    
    // Get stroke color
    glm::vec3 getColor() const { return color; }
    void setColor(const glm::vec3& c) { color = c; }
    
    // Get base width
    float getBaseWidth() const { return baseWidth; }
    void setBaseWidth(float w) { baseWidth = w; }
    
private:
    std::vector<StrokePoint> points;
    glm::vec3 color{0.0f, 0.0f, 0.0f}; // Black by default
    float baseWidth = 2.0f; // Base stroke width in pixels
};

} // namespace VectorSketch
