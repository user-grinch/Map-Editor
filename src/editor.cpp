#include "pch.h"
#include "editor.h"
#include "viewport.h"
#include "interface.h"
#include "utils.h"
#include "objmanager.h"
#include "hotkeys.h"
#include <CHud.h>
#include <CMenuManager.h>
#include "filemgr.h"

void Editor::Init()
{
    D3dHook::InjectHook(&DrawWindow);
    ApplyStyle();
    Updater::CheckUpdate();

    // Load config data
    Interface::m_bAutoSave = gConfig.GetValue("editor.autoSave", true);
    Interface::m_bAutoTpToLoc = gConfig.GetValue("editor.autoTpToLoc", false);
    Interface::m_bAutoSnapToGround = gConfig.GetValue("editor.autoSnap", true);
    Interface::m_bShowInfoMenu = gConfig.GetValue("editor.showInfoMenu", true);
    ObjManager::m_bDrawBoundingBox = gConfig.GetValue("editor.drawBoundingBox", true);
    ObjManager::m_bDrawAxisLines = gConfig.GetValue("editor.drawAxisLines", true);
    Viewport::m_bShowHoverMenu = gConfig.GetValue("editor.showHoverMenu", true);
    Interface::m_bWelcomeScreenDisplayed = gConfig.GetValue("editor.welcomeDisplayed", false);
    FontMgr::SetMultiplier(gConfig.GetValue("editor.fontMul", 1.0f));

    if (!Interface::m_bWelcomeScreenDisplayed)
    {
        Interface::m_bShowPopup = true;
        Interface::m_popupTitle = "Map Editor";
        Interface::m_pPopupFunc = Interface::WelcomeMenu;
        Interface::m_bWelcomeScreenDisplayed = true;
        gConfig.SetValue("editor.welcomeDisplayed", Interface::m_bWelcomeScreenDisplayed);
    }

    Events::processScriptsEvent += []()
    {
        if (Editor::m_bShowEditor)
        {
            Viewport::Process();
        }

        if (editorOpenKey.Pressed() && !Interface::m_bIsInputLocked)
        {
            Editor::m_bShowEditor = !Editor::m_bShowEditor;

            if (Editor::m_bShowEditor)
            {
                if (Interface::m_bAutoTpToLoc)
                {
                    CVector pos;
                    pos.x = gConfig.GetValue("editor.tp.X", -1.0f);
                    pos.y = gConfig.GetValue("editor.tp.Y", -1.0f);
                    pos.z = gConfig.GetValue("editor.tp.Z", -1.0f);

                    if (pos.x != -1.0f)
                    {
                        Viewport::SetCameraPosn(pos);
                    }
                }
            }
            else
            {
                Editor::Cleanup();
            }
        }
    };

    Events::shutdownRwEvent += []()
    {
       gConfig.WriteToDisk();
    };
};

void Editor::Cleanup()
{
    Viewport::m_eViewportMode = EDIT_MODE;
    Interface::m_bObjectBrowserShown = false;
    Interface::m_bOpenObjectBrowser = false;
    D3dHook::SetMouseState(false);
    gConfig.WriteToDisk();
    Viewport::Shutdown();
}

void Editor::DrawWindow()
{
    static bool bTriedtoHideCursor;
    if (!FrontEndMenuManager.m_bMenuActive)
    {
        if (Editor::m_bShowEditor)
        {
            if (Viewport::m_eViewportMode == EDIT_MODE)
            {
                D3dHook::SetMouseState(true);
                bTriedtoHideCursor = false;
            }
            Interface::m_bIsInputLocked = false;

            Interface::DrawMainMenuBar();
            Viewport::DrawOverlay();
            Viewport::ProcessSelectedObjectInputs();
            Interface::DrawPopupMenu();
            Interface::DrawInfoMenu();
            Viewport::DrawHoverMenu();

            if (Interface::m_bAutoSave)
            {
                static size_t timer = CTimer::m_snTimeInMilliseconds;
                size_t curTimer = CTimer::m_snTimeInMilliseconds;

                if (curTimer - timer > 60000)
                {
                    FileMgr::ExportIPL("auto_save.ipl");
                    timer = curTimer;
                }
            }
        }
    }
    else
    {
        if (!bTriedtoHideCursor)
        {
            D3dHook::SetMouseState(false);
            gConfig.WriteToDisk();
            bTriedtoHideCursor = true;
        }
    }
}

void Editor::ApplyStyle()
{
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
