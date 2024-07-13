#include "pch.h"
#include "tooltips.h"

Tooltips Tooltip;

void Tooltips::Draw() {
    if (!m_pFunc || !m_bShow) {
        return;
    }

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove
                             | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNavFocus
                             | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing;
    ImGui::SetNextWindowPos(ImVec2(15, 45));
    ImGui::SetNextWindowBgAlpha(0.35f);
    if (ImGui::Begin("##TooltipPanel", NULL, flags)) {
        m_pFunc();
        ImGui::End();
    }
}

void Tooltip_Browser() {
    ImGui::Text("Controls:");
    ImGui::Separator();
    ImGui::Text("Viewport switch: %s", viewportSwitchKey.GetNameString().c_str());
    ImGui::Spacing();
    ImGui::Text("Edit mode");
    ImGui::Spacing();
    ImGui::SameLine();
    ImGui::BulletText("Move objects");
    ImGui::Text("View mode");
    ImGui::Spacing();
    ImGui::SameLine();
    ImGui::BulletText("Rotate objects");
    ImGui::Spacing();
    ImGui::SameLine();
    ImGui::BulletText("Zoom in/out objects");
}