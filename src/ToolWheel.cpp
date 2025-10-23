#include "ToolWheel.h"
#include "imgui.h"
#include <cmath>
#include <string>

namespace VectorSketch {

ToolWheel::ToolWheel() 
    : currentTool(ToolType::BRUSH),
      brushWidth(5.2f),
      mouseOverUI(false),
      wheelVisible(true) {
}

void ToolWheel::render(int windowWidth, int windowHeight) {
    mouseOverUI = false;
    
    ImDrawList* draw_list = ImGui::GetForegroundDrawList();
    
    // Wheel position (top-left corner)
    ImVec2 center = ImVec2(100, 100);
    float outer_radius = 70.0f;
    float inner_radius = 30.0f;
    float center_circle_radius = 25.0f;
    
    // Check if mouse is over the wheel
    ImVec2 mouse_pos = ImGui::GetMousePos();
    float dist_to_center = sqrtf(
        (mouse_pos.x - center.x) * (mouse_pos.x - center.x) + 
        (mouse_pos.y - center.y) * (mouse_pos.y - center.y)
    );
    bool mouse_over_wheel = dist_to_center < outer_radius;
    mouseOverUI = mouse_over_wheel;
    
    // ===== Draw the wheel =====
    
    // Outer ring (light gray background)
    draw_list->AddCircleFilled(center, outer_radius, IM_COL32(220, 220, 220, 255), 64);
    
    // Inner circle (darker gray)
    draw_list->AddCircleFilled(center, center_circle_radius, IM_COL32(180, 180, 180, 255), 64);
    
    // ===== Brush Tool Segment (top, black) =====
    float brush_angle_start = -M_PI / 2.0f - M_PI / 8.0f;
    float brush_angle_end = -M_PI / 2.0f + M_PI / 8.0f;
    
    // Draw brush segment as filled arc
    const int segments = 32;
    ImVec2 prev_outer, prev_inner;
    
    for (int i = 0; i <= segments; i++) {
        float t = (float)i / (float)segments;
        float angle = brush_angle_start + t * (brush_angle_end - brush_angle_start);
        
        float x_outer = center.x + cosf(angle) * outer_radius;
        float y_outer = center.y + sinf(angle) * outer_radius;
        float x_inner = center.x + cosf(angle) * inner_radius;
        float y_inner = center.y + sinf(angle) * inner_radius;
        
        if (i > 0) {
            // Draw quad between this point and previous
            draw_list->AddQuadFilled(
                prev_inner, prev_outer,
                ImVec2(x_outer, y_outer), ImVec2(x_inner, y_inner),
                IM_COL32(40, 40, 40, 255)  // Black for brush
            );
        }
        
        prev_outer = ImVec2(x_outer, y_outer);
        prev_inner = ImVec2(x_inner, y_inner);
    }
    
    // Brush icon (wavy line symbol)
    float icon_angle = -M_PI / 2.0f;
    ImVec2 icon_center = ImVec2(
        center.x + cosf(icon_angle) * (outer_radius - 20.0f),
        center.y + sinf(icon_angle) * (outer_radius - 20.0f)
    );
    
    // Draw simple brush icon (wavy line)
    draw_list->AddCircleFilled(icon_center, 3.0f, IM_COL32(255, 255, 255, 255), 16);
    
    // ===== Center: Brush width display =====
    
    // Format width as "5.2 pts"
    char width_text[32];
    snprintf(width_text, sizeof(width_text), "%.1f", brushWidth);
    
    // Calculate text size
    ImVec2 text_size = ImGui::CalcTextSize(width_text);
    ImVec2 text_pos = ImVec2(center.x - text_size.x * 0.5f, center.y - text_size.y * 0.5f);
    
    // Check if clicking on the number
    bool clicking_number = false;
    if (dist_to_center < center_circle_radius && ImGui::IsMouseClicked(0)) {
        clicking_number = true;
    }
    
    // Draw the number
    draw_list->AddText(text_pos, IM_COL32(255, 255, 255, 255), width_text);
    
    // ===== Slider Popup (appears when clicking the number) =====
    
    static bool show_slider = false;
    if (clicking_number) {
        show_slider = !show_slider;
    }
    
    if (show_slider) {
        mouseOverUI = true;
        
        // Slider popup position (to the right of the wheel)
        ImVec2 slider_pos = ImVec2(center.x + outer_radius + 20, center.y - 60);
        
        ImGui::SetNextWindowPos(slider_pos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(280, 120), ImGuiCond_Always);
        
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
            
            // Slider with preset marks
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
            ImGui::PushItemWidth(260);
            
            if (ImGui::SliderFloat("##width", &brushWidth, 0.5f, 27.0f, "%.1f pts")) {
                // Slider changed
            }
            
            ImGui::PopItemWidth();
            
            // Preset buttons below
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
            if (ImGui::Button("3 pts")) brushWidth = 3.0f;
            ImGui::SameLine();
            if (ImGui::Button("5.2 pts")) brushWidth = 5.2f;
            ImGui::SameLine();
            if (ImGui::Button("16 pts")) brushWidth = 16.0f;
            ImGui::SameLine();
            if (ImGui::Button("27 pts")) brushWidth = 27.0f;
        }
        ImGui::End();
        
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        
        // Click outside to close
        if (ImGui::IsMouseClicked(0)) {
            ImVec2 click_pos = ImGui::GetMousePos();
            float dist_to_slider = 
                fabsf(click_pos.x - (slider_pos.x + 140)) + 
                fabsf(click_pos.y - (slider_pos.y + 60));
            
            if (dist_to_slider > 200 && dist_to_center > center_circle_radius) {
                show_slider = false;
            }
        }
    }
    
    // Outline for the whole wheel
    draw_list->AddCircle(center, outer_radius, IM_COL32(150, 150, 150, 255), 64, 2.0f);
}

} // namespace VectorSketch
