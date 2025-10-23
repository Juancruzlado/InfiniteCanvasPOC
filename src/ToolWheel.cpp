#include "ToolWheel.h"
#include "imgui.h"
#include <cmath>

namespace VectorSketch {

ToolWheel::ToolWheel() 
    : currentTool(ToolType::BRUSH),
      brushWidth(3.0f),
      mouseOverUI(false),
      wheelVisible(true) {
}

void ToolWheel::render(int windowWidth, int windowHeight) {
    mouseOverUI = false;
    
    // Set up ImGui window in top-left corner
    ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(200, 250), ImGuiCond_Always);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | 
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoCollapse;
    
    if (ImGui::Begin("Tool Wheel", &wheelVisible, window_flags)) {
        mouseOverUI = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem | 
                                             ImGuiHoveredFlags_ChildWindows);
        
        // Title
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Tools");
        ImGui::Separator();
        ImGui::Spacing();
        
        // Draw circular tool wheel
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 window_pos = ImGui::GetCursorScreenPos();
        ImVec2 center = ImVec2(window_pos.x + 90, window_pos.y + 60);
        float radius = 50.0f;
        
        // Draw outer circle
        draw_list->AddCircleFilled(center, radius, IM_COL32(240, 240, 240, 255), 32);
        draw_list->AddCircle(center, radius, IM_COL32(150, 150, 150, 255), 32, 2.0f);
        
        // Draw inner circle
        draw_list->AddCircleFilled(center, 15.0f, IM_COL32(200, 200, 200, 255), 32);
        
        // Brush tool section (top segment, black like in Concepts)
        float angle_start = -M_PI / 2.0f - M_PI / 6.0f;  // Start from top-left
        float angle_end = -M_PI / 2.0f + M_PI / 6.0f;    // End at top-right
        
        // Draw brush segment
        const int num_segments = 16;
        ImVec2 brush_center = center;
        
        for (int i = 0; i <= num_segments; i++) {
            float t = (float)i / (float)num_segments;
            float angle = angle_start + t * (angle_end - angle_start);
            
            float x1 = brush_center.x + cosf(angle) * 15.0f;
            float y1 = brush_center.y + sinf(angle) * 15.0f;
            float x2 = brush_center.x + cosf(angle) * radius;
            float y2 = brush_center.y + sinf(angle) * radius;
            
            if (i < num_segments) {
                float next_angle = angle_start + ((float)(i+1) / (float)num_segments) * (angle_end - angle_start);
                float x3 = brush_center.x + cosf(next_angle) * radius;
                float y3 = brush_center.y + sinf(next_angle) * radius;
                float x4 = brush_center.x + cosf(next_angle) * 15.0f;
                float y4 = brush_center.y + sinf(next_angle) * 15.0f;
                
                // Black color for brush tool (selected)
                ImU32 color = (currentTool == ToolType::BRUSH) ? 
                             IM_COL32(40, 40, 40, 255) : IM_COL32(180, 180, 180, 255);
                
                draw_list->AddQuadFilled(
                    ImVec2(x1, y1), ImVec2(x2, y2), 
                    ImVec2(x3, y3), ImVec2(x4, y4), 
                    color
                );
            }
        }
        
        // Brush icon in the segment
        ImVec2 icon_pos = ImVec2(
            brush_center.x + cosf(-M_PI/2.0f) * 32.0f - 6,
            brush_center.y + sinf(-M_PI/2.0f) * 32.0f - 6
        );
        
        // Simple brush icon (circle)
        draw_list->AddCircleFilled(icon_pos, 6.0f, IM_COL32(255, 255, 255, 255), 16);
        
        // Reserve space for the wheel
        ImGui::Dummy(ImVec2(180, 130));
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Brush width slider
        ImGui::Text("Brush Width");
        ImGui::PushItemWidth(160);
        ImGui::SliderFloat("##brushwidth", &brushWidth, 0.5f, 20.0f, "%.1f px");
        ImGui::PopItemWidth();
        
        // Visual preview of brush size
        ImGui::Spacing();
        ImVec2 preview_pos = ImGui::GetCursorScreenPos();
        ImVec2 preview_center = ImVec2(preview_pos.x + 90, preview_pos.y + 15);
        
        draw_list->AddCircleFilled(preview_center, brushWidth / 2.0f, 
                                   IM_COL32(0, 0, 0, 255), 32);
        
        ImGui::Dummy(ImVec2(180, 30));
    }
    ImGui::End();
}

} // namespace VectorSketch
