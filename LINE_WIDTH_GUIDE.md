# Extended Line Width Guide

## Overview
The brush width range has been significantly extended to support drawing at extreme zoom levels, from ultra-fine details to bold strokes.

## Width Range
- **Minimum**: 0.01 pts (ultra-fine lines for high-zoom detailed work)
- **Maximum**: 200 pts (bold strokes visible at low zoom)
- **Previous range**: 0.1 - 40 pts

## UI Changes

### Logarithmic Slider
The slider now uses **logarithmic scale** for better control across the wide range:
- Fine movements at small values (0.01 - 1)
- Smooth transitions at medium values (1 - 40)
- Controlled jumps at large values (40 - 200)

### Preset Buttons
Organized into three categories:

**Fine** (for high zoom / detailed work):
- 0.05 pts
- 0.1 pts
- 0.5 pts
- 1.0 pts

**Normal** (standard drawing):
- 5 pts
- 10 pts
- 20 pts
- 40 pts

**Bold** (for low zoom / bold strokes):
- 60 pts
- 100 pts
- 150 pts
- 200 pts

### Width Display
The center of the tool wheel now shows adaptive precision:
- Values < 1: Two decimal places (e.g., "0.05")
- Values < 10: One decimal place (e.g., "5.2")
- Values ≥ 10: No decimal places (e.g., "150")

## How It Works with Zoom

The line width system is **geometry-based** using world-space coordinates:

1. **Width is baked into geometry**: Lines are rendered as triangle strips with the width built into the vertices
2. **Automatic zoom scaling**: When you zoom, the entire geometry (including width) scales naturally
3. **No separate width compensation needed**: The system handles extreme zoom automatically

### Use Cases

**High Zoom In (e.g., 10x - 50x)**:
- Use widths: 0.01 - 2 pts
- Draw fine details, technical drawings, intricate patterns
- Lines remain visible and precise

**Normal Zoom (1x - 5x)**:
- Use widths: 2 - 40 pts
- Standard sketching and drawing
- Default comfortable range

**Low Zoom Out (0.1x - 0.5x)**:
- Use widths: 40 - 200 pts
- Bold strokes for overview/composition
- Lines remain visible when zoomed far out

## Technical Notes

- Line width is stored in world-space units
- Triangle strip rendering ensures smooth scaling
- Round caps are included for professional appearance
- No performance impact from extended range
