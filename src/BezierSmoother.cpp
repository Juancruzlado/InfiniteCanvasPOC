#include "BezierSmoother.h"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace VectorSketch {

std::vector<BezierSegment> BezierSmoother::smooth(const Stroke& stroke, float tension) {
    std::vector<BezierSegment> segments;
    const auto& points = stroke.getPoints();
    
    if (points.empty()) {
        return segments;
    }
    
    // Special case: single point (dot)
    if (points.size() == 1) {
        BezierSegment seg;
        seg.p0 = points[0].position;
        seg.p1 = points[0].position;  // Same point (degenerate segment)
        seg.c1 = points[0].position;
        seg.c2 = points[0].position;
        seg.widthStart = points[0].pressure * stroke.getBaseWidth();
        seg.widthEnd = points[0].pressure * stroke.getBaseWidth();
        segments.push_back(seg);
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

std::vector<glm::vec2> BezierSmoother::generateTriangleStrip(const std::vector<BezierSegment>& segments,
                                                              float baseWidth,
                                                              int pointsPerSegment) {
    std::vector<glm::vec2> vertices;
    
    if (segments.empty()) {
        return vertices;
    }
    
    // Check if this is a single point (degenerate segment where p0 == p1)
    if (segments.size() == 1) {
        float distance = glm::length(segments[0].p1 - segments[0].p0);
        std::cout << "Single segment, distance: " << distance << std::endl;
        
        if (distance < 0.001f) {
            // Draw a circle for a single click
            std::cout << "Drawing dot (circle)" << std::endl;
            glm::vec2 center = segments[0].p0;
            float halfWidth = baseWidth * 0.5f;
            const int circleSegments = 32;
            
            for (int i = 0; i <= circleSegments; ++i) {
                float angle = (static_cast<float>(i) / static_cast<float>(circleSegments)) * 2.0f * M_PI;
                glm::vec2 point = center + glm::vec2(cosf(angle), sinf(angle)) * halfWidth;
                vertices.push_back(center);  // Center point
                vertices.push_back(point);   // Edge point
            }
            
            return vertices;
        }
    }
    
    std::cout << "Drawing line with " << segments.size() << " segments (with round caps)" << std::endl;
    
    // First, tesselate to get centerline points
    std::vector<glm::vec2> centerPoints;
    for (const auto& segment : segments) {
        for (int i = 0; i < pointsPerSegment; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(pointsPerSegment - 1);
            centerPoints.push_back(evaluateCubic(segment, t));
        }
    }
    
    if (centerPoints.empty()) {
        return vertices;
    }
    
    float halfWidth = baseWidth * 0.5f;
    
    // Special case: very few points (shouldn't happen now but keep as fallback)
    if (centerPoints.size() == 1) {
        // Draw a circle for a single click
        glm::vec2 center = centerPoints[0];
        const int circleSegments = 32;
        
        for (int i = 0; i <= circleSegments; ++i) {
            float angle = (static_cast<float>(i) / static_cast<float>(circleSegments)) * 2.0f * M_PI;
            glm::vec2 point = center + glm::vec2(cosf(angle), sinf(angle)) * halfWidth;
            vertices.push_back(center);  // Center point
            vertices.push_back(point);   // Edge point
        }
        
        return vertices;
    }
    
    // Reserve space for triangle strip (2 vertices per center point + round caps)
    vertices.reserve(centerPoints.size() * 2 + 64);
    
    // Calculate tangents and normals for all points
    std::vector<glm::vec2> normals;
    normals.reserve(centerPoints.size());
    
    for (size_t i = 0; i < centerPoints.size(); ++i) {
        glm::vec2 tangent;
        
        // Calculate tangent direction
        if (i == 0) {
            tangent = centerPoints[i + 1] - centerPoints[i];
        } else if (i == centerPoints.size() - 1) {
            tangent = centerPoints[i] - centerPoints[i - 1];
        } else {
            tangent = centerPoints[i + 1] - centerPoints[i - 1];
        }
        
        // Normalize tangent
        float length = glm::length(tangent);
        if (length > 0.0001f) {
            tangent /= length;
        } else {
            tangent = glm::vec2(1.0f, 0.0f);
        }
        
        // Calculate perpendicular normal (rotate tangent 90 degrees)
        glm::vec2 normal(-tangent.y, tangent.x);
        normals.push_back(normal);
    }
    
    // Add round cap at start as a circle (triangle fan)
    const int capSegments = 16;
    glm::vec2 startCenter = centerPoints[0];
    
    for (int i = 0; i <= capSegments; ++i) {
        float angle = 2.0f * M_PI * static_cast<float>(i) / static_cast<float>(capSegments);
        glm::vec2 point = startCenter + glm::vec2(cosf(angle), sinf(angle)) * halfWidth;
        vertices.push_back(startCenter);
        vertices.push_back(point);
    }
    
    // Add degenerate triangles to connect cap to body
    glm::vec2 startLeft = startCenter - normals[0] * halfWidth;
    vertices.push_back(startLeft);
    vertices.push_back(startLeft);
    
    // Body of the stroke
    for (size_t i = 0; i < centerPoints.size(); ++i) {
        glm::vec2 center = centerPoints[i];
        glm::vec2 normal = normals[i];
        
        glm::vec2 left = center - normal * halfWidth;
        glm::vec2 right = center + normal * halfWidth;
        
        vertices.push_back(left);
        vertices.push_back(right);
    }
    
    // Add degenerate triangles to connect body to end cap
    glm::vec2 endCenter = centerPoints[centerPoints.size() - 1];
    glm::vec2 endRight = endCenter + normals[normals.size() - 1] * halfWidth;
    vertices.push_back(endRight);
    vertices.push_back(endRight);
    vertices.push_back(endCenter);
    
    // Add round cap at end as a circle (triangle fan)
    for (int i = 0; i <= capSegments; ++i) {
        float angle = 2.0f * M_PI * static_cast<float>(i) / static_cast<float>(capSegments);
        glm::vec2 point = endCenter + glm::vec2(cosf(angle), sinf(angle)) * halfWidth;
        vertices.push_back(endCenter);
        vertices.push_back(point);
    }
    
    std::cout << "Total vertices with caps: " << vertices.size() 
              << " (centerPoints: " << centerPoints.size() << ", capSegments: " << capSegments << ")" << std::endl;
    
    return vertices;
}

} // namespace VectorSketch
