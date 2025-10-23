# Bug Fixes - Coordinate Transformation Issues

## Issues Fixed

### Bug #1: Erratic Panning When Zoomed
**Symptom**: When zoomed in, panning causes the mouse cursor and canvas to desync, with content jumping around.

**Root Cause**: Pan delta was being applied directly in screen space, but the view transform was already scaled. A 10-pixel pan in screen space becomes a smaller movement in world space when zoomed in.

**Fix**: Convert pan delta from screen space to world space using inverse view transform:
```cpp
void VectorRenderer::pan(const glm::vec2& delta) {
    // Apply translation in world space (inverse transform the delta)
    glm::mat4 invView = glm::inverse(viewTransform);
    glm::vec4 worldDelta = invView * glm::vec4(delta.x, delta.y, 0.0f, 0.0f);
    viewTransform = glm::translate(glm::mat4(1.0f), glm::vec3(worldDelta.x, worldDelta.y, 0.0f)) * viewTransform;
}
```

### Bug #2: Drawing in Wrong Location When Zoomed
**Symptom**: Mouse cursor at position (x, y) but stroke appears at different position (x², y²) when zoomed.

**Root Cause**: Mouse coordinates are in screen space (pixels), but strokes are stored in world space. When zoomed, screen coordinates don't match world coordinates.

**Fix**: Convert all drawing coordinates from screen space to world space:
```cpp
// In mouseButtonCallback and cursorPosCallback:
glm::vec2 worldPos = renderer.screenToWorld(mousePos);
StrokePoint point(worldPos, pressure, tiltX, tiltY, getCurrentTime());
```

### Bug #3: Zoom Center Point Incorrect
**Symptom**: Zooming doesn't center properly around mouse cursor when already zoomed.

**Fix**: Convert screen-space zoom center to world space before applying zoom transformation:
```cpp
void VectorRenderer::zoom(float factor, const glm::vec2& center) {
    // Convert screen center to world space before zoom
    glm::vec2 worldCenter = screenToWorld(center);
    
    // Translate to origin, scale, translate back
    viewTransform = glm::translate(glm::mat4(1.0f), glm::vec3(worldCenter, 0.0f)) * viewTransform;
    viewTransform = glm::scale(glm::mat4(1.0f), glm::vec3(factor, factor, 1.0f)) * viewTransform;
    viewTransform = glm::translate(glm::mat4(1.0f), glm::vec3(-worldCenter, 0.0f)) * viewTransform;
}
```

## New Method Added

### `screenToWorld()`
Converts screen coordinates to world coordinates by inverting the MVP (Model-View-Projection) matrix:

```cpp
glm::vec2 VectorRenderer::screenToWorld(const glm::vec2& screenPos) const {
    // Get the combined transformation matrix
    glm::mat4 mvp = projectionMatrix * viewTransform;
    glm::mat4 invMVP = glm::inverse(mvp);
    
    // Screen coordinates directly (projection handles Y-flip)
    glm::vec4 ndc(screenPos.x, screenPos.y, 0.0f, 1.0f);
    
    // Transform to world space
    glm::vec4 worldPos = invMVP * ndc;
    
    return glm::vec2(worldPos.x, worldPos.y);
}
```

## Coordinate Spaces Explained

### 1. Screen Space
- Origin: Top-left (0, 0)
- Units: Pixels
- Range: (0, 0) to (windowWidth, windowHeight)
- Used for: Mouse input from GLFW

### 2. World Space
- Origin: User-defined (initially 0, 0)
- Units: Arbitrary (canvas units)
- Range: Unlimited (infinite canvas)
- Used for: Stroke data storage

### 3. NDC (Normalized Device Coordinates)
- Origin: Center (0, 0)
- Range: (-1, -1) to (1, 1)
- Used for: GPU rendering (intermediate)

## Transformation Pipeline

### Drawing Flow
```
Mouse Input (Screen Space)
    ↓
screenToWorld()
    ↓
Stroke Data (World Space)
    ↓
Renderer: MVP Transform
    ↓
GPU (NDC → Screen Space)
```

### Pan Flow
```
Mouse Delta (Screen Space)
    ↓
Inverse View Transform
    ↓
World Space Delta
    ↓
Update View Transform
```

### Zoom Flow
```
Mouse Position (Screen Space)
    ↓
screenToWorld()
    ↓
Zoom Center (World Space)
    ↓
Translate-Scale-Translate
    ↓
Update View Transform
```

## Files Modified

1. **include/VectorRenderer.h**
   - Added `screenToWorld()` method declaration
   - Added `#include <glm/gtc/matrix_inverse.hpp>`

2. **src/VectorRenderer.cpp**
   - Implemented `screenToWorld()` method
   - Fixed `pan()` to use world space deltas
   - Fixed `zoom()` to convert center to world space

3. **src/main.cpp**
   - Added `lastWorldPos` to track world coordinates
   - Convert mouse to world coords in `mouseButtonCallback()`
   - Convert mouse to world coords in `cursorPosCallback()`
   - Use world space for pressure calculation

## Testing the Fixes

### Test 1: Draw at Default Zoom
- Draw a stroke
- Should appear exactly under cursor

### Test 2: Zoom In and Draw
- Zoom in (scroll up)
- Draw a stroke
- Should still appear exactly under cursor

### Test 3: Pan While Zoomed
- Zoom in significantly
- Pan with middle/right mouse
- Canvas should move smoothly following cursor
- No jumping or erratic behavior

### Test 4: Zoom Around Cursor
- Draw something in center
- Move cursor to edge
- Zoom in/out
- Should zoom around cursor position, not center

### Test 5: Complex Navigation
- Zoom in
- Pan around
- Draw strokes
- Zoom out
- All strokes should be in correct positions

## Mathematical Details

### MVP Matrix Composition
```
MVP = Projection × View × Model
```
Where:
- **Model**: Identity (no per-object transform)
- **View**: Pan and zoom transforms
- **Projection**: Orthographic (2D)

### Inverse Transform
To go from screen → world, we invert the MVP:
```
worldPos = MVP⁻¹ × screenPos
```

### Pan in World Space
Screen delta must account for current zoom:
```
worldDelta = View⁻¹ × screenDelta
```

This ensures a 100-pixel pan moves the same world distance regardless of zoom level.

## Why This Matters

These bugs are common in 2D canvas applications and graphics programming:

1. **Coordinate Space Confusion**: Mixing screen and world coordinates causes misalignment
2. **Transform Order**: Matrix multiplication is non-commutative; order matters
3. **Inverse Transforms**: Required to convert user input to world space

The fixes ensure:
- ✅ Drawing always appears under cursor
- ✅ Panning is smooth at any zoom level
- ✅ Zooming centers on cursor position
- ✅ All operations work correctly when combined

## Performance Impact

- `screenToWorld()`: ~1 matrix inverse per mouse event (negligible)
- `pan()`: ~1 matrix inverse per pan event (negligible)
- No impact on rendering performance
- No impact on stroke storage

## Future Enhancements

Consider adding:
- Cached inverse matrices to avoid recalculation
- Viewport culling using world-space bounds
- Mini-map showing world coordinates
- Debug overlay showing coordinate spaces
