#pragma once

#include <glm/glm.hpp>

namespace VectorSketch {

// Represents a single point in a stroke with pressure and tilt data
struct StrokePoint {
    glm::vec2 position;    // X, Y coordinates
    float pressure;         // 0.0 to 1.0
    float tiltX;           // Tilt angle in X (-1.0 to 1.0)
    float tiltY;           // Tilt angle in Y (-1.0 to 1.0)
    float timestamp;       // Time when point was captured
    
    StrokePoint(glm::vec2 pos = glm::vec2(0.0f), 
                float press = 1.0f, 
                float tx = 0.0f, 
                float ty = 0.0f,
                float time = 0.0f)
        : position(pos), pressure(press), tiltX(tx), tiltY(ty), timestamp(time) {}
};

} // namespace VectorSketch
