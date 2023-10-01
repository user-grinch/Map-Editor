#include "pch.h"
#include "editor.h"
#include "viewport.h"
#include "interface.h"
#include "utils/utils.h"
#include "objmanager.h"
#include <CHud.h>
#include <CMenuManager.h>
#include "filemgr.h"

bool Editor::IsOpen() {
    return m_bOpened;
}

void Editor::Init() {
    D3dHook::Init(&Draw);
    ApplyStyle();
    Updater::CheckUpdate();

    // Load config data
    Viewport::m_nMoveSpeed = gConfig.Get("editor.moveSpeed", 1.0f);
    Interface::Init();

    Events::processScriptsEvent += []() {
        if (toggleUIKey.Pressed()) {
            Interface::Interface::m_bShowGUI = !Interface::Interface::m_bShowGUI;
        }

        if (copyHoveredObjName.Pressed()) {
            std::string name = ObjManager::FindNameFromModel(Viewport::m_HoveredEntity->m_nModelIndex);
            ImGui::SetClipboardText(name.c_str());
            CHud::SetHelpMessage("Copied to clipboard", false, false, false);
        }

        if (editorOpenKey.Pressed() && !Interface::m_bInputLocked) {
            m_bOpened = !m_bOpened;

            if (m_bOpened) {
                if (Interface::m_bAutoTpToLoc) {
                    CVector pos;
                    pos.x = gConfig.Get("editor.tp.X", -1.0f);
                    pos.y = gConfig.Get("editor.tp.Y", -1.0f);
                    pos.z = gConfig.Get("editor.tp.Z", -1.0f);

                    if (pos.x != -1.0f) {
                        Viewport::SetCameraPosn(pos);
                    }
                }
            } else {
                Editor::Cleanup();
            }
        }
    };

    Events::shutdownRwEvent += []() {
        gConfig.Save();
    };
};

void Editor::Cleanup() {
    D3dHook::SetMouseState(false);
    gConfig.Save();
    Viewport::Cleanup();
}

void Editor::Draw() {
    static bool bTriedtoHideCursor;
    if (!FrontEndMenuManager.m_bMenuActive) {
        if (m_bOpened) {
            if (Viewport::m_eState == eViewportState::Edit) {
                D3dHook::SetMouseState(true);
                bTriedtoHideCursor = false;
            }
            Interface::m_bInputLocked = false;

            if (Interface::m_bAutoSave && ObjManager::m_pPlacedObjs.size() > 0) {
                static size_t timer = CTimer::m_snTimeInMilliseconds;
                size_t curTimer = CTimer::m_snTimeInMilliseconds;

                if (curTimer - timer > 60000) {
                    FileMgr::ExportIPL("auto_save.ipl");
                    timer = curTimer;
                }
            }
            Interface::Process();
            Viewport::Process();
        }
    } else {
        if (!bTriedtoHideCursor) {
            D3dHook::SetMouseState(false);
            gConfig.Save();
            bTriedtoHideCursor = true;
        }
    }
}

void Editor::ApplyStyle() {
    ImGuiStyle* style = &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    style->WindowPadding = ImVec2(8, 8);
    style->WindowRounding = 5.0f;
    style->FramePadding = ImVec2(8, 8);
    style->FrameRounding = 5.0f;
    style->PopupRounding = 5.0f;
    style->ItemSpacing = ImVec2(7, 7);
    style->ItemInnerSpacing = ImVec2(7, 7);
    style->IndentSpacing = 25.0f;
    style->ScrollbarSize = 12.0f;
    style->ScrollbarRounding = 10.0f;
    style->GrabMinSize = 5.0f;
    style->GrabRounding = 3.0f;

    style->ChildBorderSize = 0;
    style->WindowBorderSize = 0;
    style->FrameBorderSize = 0;
    style->TabBorderSize = 0;
    style->PopupBorderSize = 0;
    style->WindowTitleAlign = ImVec2(0.5f, 0.5f);

    style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
    style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.35f, 0.33f, 0.3f, 1.00f);
    style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.06f, 0.95f);
    style->Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 0.95f);
    style->Colors[ImGuiCol_Border] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
    style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    style->Colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.15f, 0.95f);
    style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
    style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    style->Colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.12f, 0.12f, 0.94f);
    style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
    style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.15f, 0.15f, 0.95f);
    style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.15f, 0.15f, 0.15f, 0.95f);
    style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.5f, 0.5f, 0.5f, 0.3f);
    style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.7f, 0.7f, 0.7f, 0.3f);
    style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.9f, 0.9f, 0.9f, 0.3f);
    style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_Separator] = ImVec4(0.15f, 0.15f, 0.15f, 0.95f);
    style->Colors[ImGuiCol_Button] = ImVec4(0.15f, 0.15f, 0.15f, 0.95f);
    style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
    style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    style->Colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.15f, 0.95f);
    style->Colors[ImGuiCol_TabHovered] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
    style->Colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    style->Colors[ImGuiCol_Header] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
    style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.12f, 0.12f, 0.12f, 0.00f);
    style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
    style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.06f, 0.05f, 0.06f, 0.95f);
    style->Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.6f);
}
