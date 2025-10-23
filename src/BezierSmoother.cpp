#include "BezierSmoother.h"
#include <cmath>
#include <algorithm>

namespace VectorSketch {

std::vector<BezierSegment> BezierSmoother::smooth(const Stroke& stroke, float tension) {
    std::vector<BezierSegment> segments;
    const auto& points = stroke.getPoints();
    
    if (points.size() < 2) {
        return segments;
    }
    
    // For very short strokes, just create a simple line
    if (points.size() == 2) {
        BezierSegment seg;
        seg.p0 = points[0].position;
        seg.p1 = points[1].position;
        seg.c1 = glm::mix(seg.p0, seg.p1, 0.33f);
        seg.c2 = glm::mix(seg.p0, seg.p1, 0.67f);
        seg.widthStart = points[0].pressure * stroke.getBaseWidth();
        seg.widthEnd = points[1].pressure * stroke.getBaseWidth();
        segments.push_back(seg);
        return segments;
    }
    
    // Use Catmull-Rom to create smooth Bézier curves
    for (size_t i = 0; i < points.size() - 1; ++i) {
        BezierSegment seg;
        
        const glm::vec2& p0 = points[i].position;
        const glm::vec2& p1 = points[i + 1].position;
        
        seg.p0 = p0;
        seg.p1 = p1;
        
        // Calculate control points using Catmull-Rom approach
        glm::vec2 prevPos = (i > 0) ? points[i - 1].position : p0;
        glm::vec2 nextPos = (i + 2 < points.size()) ? points[i + 2].position : p1;
        
        // Tangent at p0
        glm::vec2 tangent0 = (p1 - prevPos) * tension;
        // Tangent at p1
        glm::vec2 tangent1 = (nextPos - p0) * tension;
        
        // Convert to Bézier control points
        seg.c1 = p0 + tangent0 / 3.0f;
        seg.c2 = p1 - tangent1 / 3.0f;
        
        // Width based on pressure
        seg.widthStart = points[i].pressure * stroke.getBaseWidth();
        seg.widthEnd = points[i + 1].pressure * stroke.getBaseWidth();
        
        segments.push_back(seg);
    }
    
    return segments;
}

glm::vec2 BezierSmoother::evaluateCubic(const BezierSegment& segment, float t) {
    float t2 = t * t;
    float t3 = t2 * t;
    float mt = 1.0f - t;
    float mt2 = mt * mt;
    float mt3 = mt2 * mt;
    
    return segment.p0 * mt3 + 
           segment.c1 * 3.0f * mt2 * t + 
           segment.c2 * 3.0f * mt * t2 + 
           segment.p1 * t3;
}

std::vector<glm::vec2> BezierSmoother::tesselate(const std::vector<BezierSegment>& segments, 
                                                  int pointsPerSegment) {
    std::vector<glm::vec2> result;
    
    for (const auto& segment : segments) {
        for (int i = 0; i < pointsPerSegment; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(pointsPerSegment - 1);
            result.push_back(evaluateCubic(segment, t));
        }
    }
    
    return result;
}

} // namespace VectorSketch
