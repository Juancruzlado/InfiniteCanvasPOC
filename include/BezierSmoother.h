#pragma once

#include "Stroke.h"
#include <vector>
#include <glm/glm.hpp>

namespace VectorSketch {

// Control point for a cubic Bézier curve
struct BezierSegment {
    glm::vec2 p0;      // Start point
    glm::vec2 c1;      // First control point
    glm::vec2 c2;      // Second control point
    glm::vec2 p1;      // End point
    float widthStart;  // Width at start
    float widthEnd;    // Width at end
};

// Smooths stroke points into Bézier curves
class BezierSmoother {
public:
    // Convert raw stroke points into smooth Bézier segments
    static std::vector<BezierSegment> smooth(const Stroke& stroke, float tension = 0.5f);
    
    // Evaluate a Bézier curve at parameter t (0 to 1)
    static glm::vec2 evaluateCubic(const BezierSegment& segment, float t);
    
    // Tesselate Bézier segments into line segments for rendering
    static std::vector<glm::vec2> tesselate(const std::vector<BezierSegment>& segments, 
                                            int pointsPerSegment = 20);
};

} // namespace VectorSketch
