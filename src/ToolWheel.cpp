#include "ToolWheel.h"
#include "imgui.h"
#include <cmath>
#include <string>

namespace VectorSketch {

ToolWheel::ToolWheel() 
    : currentTool(ToolType::BRUSH),
      brushWidth(5.2f),
      currentColor(0.0f, 0.0f, 0.0f),  // Black by default
      mouseOverUI(false),
      wheelVisible(true) {
}

void ToolWheel::render(int windowWidth, int windowHeight) {
    mouseOverUI = false;
    
    ImDrawList* draw_list = ImGui::GetForegroundDrawList();
    
    // Wheel position (top-left corner)
    ImVec2 center = ImVec2(100, 100);
    
    // Three concentric circles
    float outer_radius = 70.0f;      // Tool selector ring
    float middle_radius = 45.0f;     // Width display ring
    float inner_radius = 25.0f;      // Color selector circle
    
    // Check if mouse is over the wheel
    ImVec2 mouse_pos = ImGui::GetMousePos();
    float dist_to_center = sqrtf(
        (mouse_pos.x - center.x) * (mouse_pos.x - center.x) + 
        (mouse_pos.y - center.y) * (mouse_pos.y - center.y)
    );
    bool mouse_over_wheel = dist_to_center < outer_radius;
    mouseOverUI = mouse_over_wheel;
    
    // ===== Draw the three circles =====
    
    // Outer ring (light gray) - Tool selector background
    draw_list->AddCircleFilled(center, outer_radius, IM_COL32(220, 220, 220, 255), 64);
    
    // Middle ring (white) - Width display
    draw_list->AddCircleFilled(center, middle_radius, IM_COL32(255, 255, 255, 255), 64);
    
    // Inner circle (black) - Color selector
    draw_list->AddCircleFilled(center, inner_radius, IM_COL32(0, 0, 0, 255), 64);
    
    // ===== Tool Segments (outer ring) =====
    const int arc_segments = 32;
    
    // Brush Tool Segment (top)
    float brush_angle_start = -M_PI / 2.0f - M_PI / 8.0f;
    float brush_angle_end = -M_PI / 2.0f + M_PI / 8.0f;
    
    // Eraser Tool Segment (bottom)
    float eraser_angle_start = M_PI / 2.0f - M_PI / 8.0f;
    float eraser_angle_end = M_PI / 2.0f + M_PI / 8.0f;
    
    // Check which tool segment is being clicked
    if (mouse_over_wheel && dist_to_center > middle_radius && ImGui::IsMouseClicked(0)) {
        // Calculate angle of mouse relative to center
        float dx = mouse_pos.x - center.x;
        float dy = mouse_pos.y - center.y;
        float mouse_angle = atan2f(dy, dx);
        
        // Check if clicking on brush segment (top)
        if (mouse_angle >= brush_angle_start && mouse_angle <= brush_angle_end) {
            currentTool = ToolType::BRUSH;
        }
        // Check if clicking on eraser segment (bottom)
        else if (mouse_angle >= eraser_angle_start && mouse_angle <= eraser_angle_end) {
            currentTool = ToolType::ERASER;
        }
    }
    
    // Draw Brush segment
    ImVec2 prev_outer, prev_inner;
    ImU32 brush_color = (currentTool == ToolType::BRUSH) ? IM_COL32(40, 40, 40, 255) : IM_COL32(100, 100, 100, 255);
    
    for (int i = 0; i <= arc_segments; i++) {
        float t = (float)i / (float)arc_segments;
        float angle = brush_angle_start + t * (brush_angle_end - brush_angle_start);
        
        float x_outer = center.x + cosf(angle) * outer_radius;
        float y_outer = center.y + sinf(angle) * outer_radius;
        float x_inner = center.x + cosf(angle) * middle_radius;
        float y_inner = center.y + sinf(angle) * middle_radius;
        
        if (i > 0) {
            draw_list->AddQuadFilled(
                prev_inner, prev_outer,
                ImVec2(x_outer, y_outer), ImVec2(x_inner, y_inner),
                brush_color
            );
        }
        
        prev_outer = ImVec2(x_outer, y_outer);
        prev_inner = ImVec2(x_inner, y_inner);
    }
    
    // Draw Eraser segment
    ImU32 eraser_color = (currentTool == ToolType::ERASER) ? IM_COL32(240, 100, 100, 255) : IM_COL32(150, 150, 150, 255);
    
    for (int i = 0; i <= arc_segments; i++) {
        float t = (float)i / (float)arc_segments;
        float angle = eraser_angle_start + t * (eraser_angle_end - eraser_angle_start);
        
        float x_outer = center.x + cosf(angle) * outer_radius;
        float y_outer = center.y + sinf(angle) * outer_radius;
        float x_inner = center.x + cosf(angle) * middle_radius;
        float y_inner = center.y + sinf(angle) * middle_radius;
        
        if (i > 0) {
            draw_list->AddQuadFilled(
                prev_inner, prev_outer,
                ImVec2(x_outer, y_outer), ImVec2(x_inner, y_inner),
                eraser_color
            );
        }
        
        prev_outer = ImVec2(x_outer, y_outer);
        prev_inner = ImVec2(x_inner, y_inner);
    }
    
    // Brush icon (wavy line symbol) in outer ring
    float brush_icon_angle = -M_PI / 2.0f;
    float icon_radius = (outer_radius + middle_radius) / 2.0f;
    ImVec2 brush_icon_center = ImVec2(
        center.x + cosf(brush_icon_angle) * icon_radius,
        center.y + sinf(brush_icon_angle) * icon_radius
    );
    
    // Draw simple brush icon (circle)
    draw_list->AddCircleFilled(brush_icon_center, 3.0f, IM_COL32(255, 255, 255, 255), 16);
    
    // Eraser icon in outer ring
    float eraser_icon_angle = M_PI / 2.0f;
    ImVec2 eraser_icon_center = ImVec2(
        center.x + cosf(eraser_icon_angle) * icon_radius,
        center.y + sinf(eraser_icon_angle) * icon_radius
    );
    
    // Draw eraser icon (small square)
    float eraser_icon_size = 5.0f;
    draw_list->AddRectFilled(
        ImVec2(eraser_icon_center.x - eraser_icon_size/2, eraser_icon_center.y - eraser_icon_size/2),
        ImVec2(eraser_icon_center.x + eraser_icon_size/2, eraser_icon_center.y + eraser_icon_size/2),
        IM_COL32(255, 255, 255, 255)
    );
    
    // ===== Middle ring: Brush width display =====
    
    // Format width with appropriate precision based on size
    char width_text[32];
    if (brushWidth < 1.0f) {
        snprintf(width_text, sizeof(width_text), "%.2f", brushWidth);
    } else if (brushWidth < 10.0f) {
        snprintf(width_text, sizeof(width_text), "%.1f", brushWidth);
    } else {
        snprintf(width_text, sizeof(width_text), "%.0f", brushWidth);
    }
    
    // Calculate text size
    ImVec2 text_size = ImGui::CalcTextSize(width_text);
    ImVec2 text_pos = ImVec2(center.x - text_size.x * 0.5f, center.y - text_size.y * 0.5f);
    
    // Check if clicking on the middle ring to open slider
    bool clicking_width_ring = false;
    if (dist_to_center > inner_radius && dist_to_center < middle_radius && ImGui::IsMouseClicked(0)) {
        clicking_width_ring = true;
    }
    
    // Check if clicking on the inner circle to open color picker
    bool clicking_color_circle = false;
    if (dist_to_center <= inner_radius && ImGui::IsMouseClicked(0)) {
        clicking_color_circle = true;
    }
    
    // Draw the width text in middle ring (black text on white background)
    draw_list->AddText(text_pos, IM_COL32(0, 0, 0, 255), width_text);
    
    // ===== Slider Popup (appears when clicking the middle ring) =====
    
    static bool show_slider = false;
    static bool slider_was_just_opened = false;
    
    if (clicking_width_ring) {
        show_slider = !show_slider;
        slider_was_just_opened = true;
    }
    
    if (show_slider) {
        mouseOverUI = true;
        
        // Slider popup position (to the right of the wheel)
        ImVec2 slider_pos = ImVec2(center.x + outer_radius + 20, center.y - 60);
        
        ImGui::SetNextWindowPos(slider_pos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(280, 180), ImGuiCond_Always);
        
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | 
                                ImGuiWindowFlags_NoResize |
                                ImGuiWindowFlags_NoMove |
                                ImGuiWindowFlags_NoScrollbar;
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.95f, 0.95f, 0.95f, 0.98f));
        
        if (ImGui::Begin("##BrushSlider", &show_slider, flags)) {
            ImGui::Spacing();
            
            // Title
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
            ImGui::Text("TAMAÑO");
            
            ImGui::Spacing();
            ImGui::Spacing();
            
            // Slider with extended range for extreme zoom levels
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
            ImGui::PushItemWidth(260);
            
            // Logarithmic slider for better control across wide range
            if (ImGui::SliderFloat("##width", &brushWidth, 0.01f, 200.0f, "%.2f pts", ImGuiSliderFlags_Logarithmic)) {
                // Slider changed
            }
            
            ImGui::PopItemWidth();
            
            ImGui::Spacing();
            ImGui::Spacing();
            
            // Preset buttons below - organized by size
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
            ImGui::Text("Fine:");
            ImGui::SameLine();
            if (ImGui::Button("0.05")) brushWidth = 0.05f;
            ImGui::SameLine();
            if (ImGui::Button("0.1")) brushWidth = 0.1f;
            ImGui::SameLine();
            if (ImGui::Button("0.5")) brushWidth = 0.5f;
            ImGui::SameLine();
            if (ImGui::Button("1")) brushWidth = 1.0f;
            
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
            ImGui::Text("Normal:");
            ImGui::SameLine();
            if (ImGui::Button("5")) brushWidth = 5.0f;
            ImGui::SameLine();
            if (ImGui::Button("10")) brushWidth = 10.0f;
            ImGui::SameLine();
            if (ImGui::Button("20")) brushWidth = 20.0f;
            ImGui::SameLine();
            if (ImGui::Button("40")) brushWidth = 40.0f;
            
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
            ImGui::Text("Bold:");
            ImGui::SameLine();
            if (ImGui::Button("60")) brushWidth = 60.0f;
            ImGui::SameLine();
            if (ImGui::Button("100")) brushWidth = 100.0f;
            ImGui::SameLine();
            if (ImGui::Button("150")) brushWidth = 150.0f;
            ImGui::SameLine();
            if (ImGui::Button("200")) brushWidth = 200.0f;
        }
        ImGui::End();
        
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        
        // Click outside to close (but not on the frame it was just opened)
        if (ImGui::IsMouseClicked(0) && !slider_was_just_opened) {
            // Check if click is outside both the slider and the wheel
            ImVec2 click_pos = ImGui::GetMousePos();
            
            // Check if click is outside slider window
            bool outside_slider = (click_pos.x < slider_pos.x || 
                                   click_pos.x > slider_pos.x + 280 ||
                                   click_pos.y < slider_pos.y || 
                                   click_pos.y > slider_pos.y + 180);
            
            // Check if click is outside wheel
            bool outside_wheel = dist_to_center > outer_radius;
            
            if (outside_slider && outside_wheel) {
                show_slider = false;
            }
        }
        
        // Reset the flag after this frame
        slider_was_just_opened = false;
    }
    
    // ===== Color Picker Popup (appears when clicking the inner circle) =====
    
    static bool show_color_picker = false;
    static bool color_picker_was_just_opened = false;
    
    // Use member variable currentColor, convert to array for ImGui
    float temp_color[3] = {currentColor.r, currentColor.g, currentColor.b};
    
    if (clicking_color_circle) {
        show_color_picker = !show_color_picker;
        color_picker_was_just_opened = true;
    }
    
    if (show_color_picker) {
        mouseOverUI = true;
        
        // Color picker popup position (to the right of the wheel)
        ImVec2 picker_pos = ImVec2(center.x + outer_radius + 20, center.y - 100);
        
        ImGui::SetNextWindowPos(picker_pos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(300, 320), ImGuiCond_Always);
        
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | 
                                ImGuiWindowFlags_NoResize |
                                ImGuiWindowFlags_NoMove |
                                ImGuiWindowFlags_NoScrollbar;
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.95f, 0.95f, 0.95f, 0.98f));
        
        if (ImGui::Begin("##ColorPicker", &show_color_picker, flags)) {
            ImGui::Spacing();
            
            // Title
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
            ImGui::Text("COLOR");
            
            ImGui::Spacing();
            ImGui::Spacing();
            
            // Color picker wheel
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
            if (ImGui::ColorPicker3("##picker", temp_color, 
                               ImGuiColorEditFlags_PickerHueWheel | 
                               ImGuiColorEditFlags_NoSidePreview |
                               ImGuiColorEditFlags_NoInputs |
                               ImGuiColorEditFlags_NoAlpha)) {
                // Update currentColor when picker changes
                currentColor = glm::vec3(temp_color[0], temp_color[1], temp_color[2]);
            }
            
            ImGui::Spacing();
            
            // Common color presets
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
            ImGui::Text("Presets:");
            
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
            if (ImGui::ColorButton("Black", ImVec4(0.0f, 0.0f, 0.0f, 1.0f), 0, ImVec2(30, 30))) {
                currentColor = glm::vec3(0.0f, 0.0f, 0.0f);
            }
            ImGui::SameLine();
            if (ImGui::ColorButton("Red", ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 0, ImVec2(30, 30))) {
                currentColor = glm::vec3(1.0f, 0.0f, 0.0f);
            }
            ImGui::SameLine();
            if (ImGui::ColorButton("Blue", ImVec4(0.0f, 0.0f, 1.0f, 1.0f), 0, ImVec2(30, 30))) {
                currentColor = glm::vec3(0.0f, 0.0f, 1.0f);
            }
            ImGui::SameLine();
            if (ImGui::ColorButton("Green", ImVec4(0.0f, 1.0f, 0.0f, 1.0f), 0, ImVec2(30, 30))) {
                currentColor = glm::vec3(0.0f, 1.0f, 0.0f);
            }
            ImGui::SameLine();
            if (ImGui::ColorButton("Yellow", ImVec4(1.0f, 1.0f, 0.0f, 1.0f), 0, ImVec2(30, 30))) {
                currentColor = glm::vec3(1.0f, 1.0f, 0.0f);
            }
            ImGui::SameLine();
            if (ImGui::ColorButton("Magenta", ImVec4(1.0f, 0.0f, 1.0f, 1.0f), 0, ImVec2(30, 30))) {
                currentColor = glm::vec3(1.0f, 0.0f, 1.0f);
            }
        }
        ImGui::End();
        
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        
        // Update the center circle color to show current selection
        draw_list->AddCircleFilled(center, inner_radius, 
                                   IM_COL32((int)(currentColor.r * 255), 
                                           (int)(currentColor.g * 255), 
                                           (int)(currentColor.b * 255), 255), 64);
        
        // Click outside to close
        if (ImGui::IsMouseClicked(0) && !color_picker_was_just_opened) {
            ImVec2 click_pos = ImGui::GetMousePos();
            
            bool outside_picker = (click_pos.x < picker_pos.x || 
                                  click_pos.x > picker_pos.x + 300 ||
                                  click_pos.y < picker_pos.y || 
                                  click_pos.y > picker_pos.y + 320);
            
            bool outside_wheel = dist_to_center > outer_radius;
            
            if (outside_picker && outside_wheel) {
                show_color_picker = false;
            }
        }
        
        color_picker_was_just_opened = false;
    }
    
    // Outline for the whole wheel
    draw_list->AddCircle(center, outer_radius, IM_COL32(150, 150, 150, 255), 64, 2.0f);
}

} // namespace VectorSketch
