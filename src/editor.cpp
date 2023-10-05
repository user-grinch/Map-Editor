#include "pch.h"
#include "editor.h"
#include "viewport.h"
#include "interface.h"
#include <CHud.h>
#include <CMenuManager.h>
#include "filemgr.h"
#include "objectmgr.h"

bool EditorMgr::IsOpen() {
    return m_bOpened;
}

EditorMgr Editor;
EditorMgr::EditorMgr() {

    uint gameVersion = GetGameVersion();
    if (gameVersion != GAME_10US_HOODLUM && gameVersion == GAME_10US_COMPACT)
    {
        MessageBox(HWND_DESKTOP, "Unknown game version. GTA SA v1.0 US is required.", "CheatMenu", MB_ICONERROR);
        return;
    }
   
    Events::initRwEvent.before += [this]() {
        D3dHook::Init(fArgNoneWrapper(Editor.Process));
        ApplyStyle();
    };

    Events::initGameEvent += []() {
        if (!std::filesystem::is_directory(PLUGIN_PATH((char*)FILE_NAME))) {
            std::string msg = std::format("{} folder not found. You need to put both '{}.asi' & '{}' folder in the same directory", FILE_NAME, FILE_NAME, FILE_NAME);
            Log::Print<eLogLevel::Error>(msg.c_str());
            MessageBox(NULL, msg.c_str(), FILE_NAME, MB_ICONERROR);
            return;
        }

        if (!GetModuleHandle("SilentPatchSA.asi")) {
            Log::Print<eLogLevel::Error>("SilentPatch not found. Please install it from here https://gtaforums.com/topic/669045-silentpatch/");
            int msgID = MessageBox(NULL, "SilentPatch not found. Do you want to install Silent Patch? (Game restart required)", FILE_NAME, MB_OKCANCEL | MB_DEFBUTTON1);

            if (msgID == IDOK) {
                OPEN_LINK("https://gtaforums.com/topic/669045-silentpatch/");
            };
            return;
        }

        Log::Print<eLogLevel::None>("Starting " EDITOR_TITLE " (" __DATE__ ")\nAuthor: Grinch_\nDiscord: "
                                    DISCORD_INVITE "\nPatreon: " PATREON_LINK "\nMore Info: " GITHUB_LINK "\n");

        // date time
        SYSTEMTIME st;
        GetSystemTime(&st);
        Log::Print<eLogLevel::None>("Date: {}-{}-{} Time: {}:{}\n", st.wYear, st.wMonth, st.wDay,
                                    st.wHour, st.wMinute);

        bool modloader = GetModuleHandle("modloader.asi");
        const char *path = PLUGIN_PATH((char*)"");
        Log::Print<eLogLevel::None>("Install location: {}", modloader && strstr(path, "modloader") ? "Modloader" : "Game directory");
        Log::Print<eLogLevel::None>("FLA installed: {}", GetModuleHandle("$fastman92limitAdjuster.asi") ? "True" : "False");
        Log::Print<eLogLevel::None>("Modloader installed: {}", modloader ? "True" : "False");
        Log::Print<eLogLevel::None>("OLA installed: {}", GetModuleHandle("III.VC.SA.LimitAdjuster.asi") ? "True" : "False");
        Log::Print<eLogLevel::None>("Renderhook installed: {}", GetModuleHandle("_gtaRenderHook.asi") ? "True" : "False");
        Log::Print<eLogLevel::None>("");

        // Checking for updates once a day
        if (gConfig.Get("Menu.LastUpdateChecked", 0) != st.wDay) {
            Updater::CheckUpdate();
            gConfig.Set("Menu.LastUpdateChecked", st.wDay);
        }

        if (!std::filesystem::exists(PLUGIN_PATH((char*)FILE_NAME))) {
            Log::Print<eLogLevel::Error>("Failed to find MapEditor directory!");
            return;
        }
    };

    Events::processScriptsEvent += [this]() {
        if (toggleUIKey.Pressed()) {
            Interface.m_bShowGUI = !Interface.m_bShowGUI;
        }

        if (copyHoveredObjName.Pressed()) {
            std::string name = ObjMgr.FindNameFromModel(Viewport.m_HoveredEntity->m_nModelIndex);
            ImGui::SetClipboardText(name.c_str());
            CHud::SetHelpMessage("Copied to clipboard", false, false, false);
        }

        if (editorOpenKey.Pressed() && !Interface.m_bInputLocked) {
            m_bOpened = !m_bOpened;

            if (m_bOpened) {
                if (Interface.m_bAutoTpToLoc) {
                    CVector pos;
                    pos.x = gConfig.Get("editor.tp.X", -1.0f);
                    pos.y = gConfig.Get("editor.tp.Y", -1.0f);
                    pos.z = gConfig.Get("editor.tp.Z", -1.0f);

                    if (pos.x != -1.0f) {
                        Viewport.SetCameraPosn(pos);
                    }
                }
            } else {
                Cleanup();
            }
        }
    };

    Events::shutdownRwEvent += []() {
        gConfig.Save();
    };
};

void EditorMgr::Cleanup() {
    D3dHook::SetMouseState(false);
    Viewport.Cleanup();
    gConfig.Save();
}

void EditorMgr::Process() {
    static bool bTriedtoHideCursor;
    if (!FrontEndMenuManager.m_bMenuActive) {
        if (m_bOpened) {
            if (Viewport.m_eState == eViewportState::Edit) {
                D3dHook::SetMouseState(true);
                bTriedtoHideCursor = false;
            }
            Interface.m_bInputLocked = false;

            if (Interface.m_bAutoSave && ObjMgr.m_pPlacedObjs.size() > 0) {
                static size_t timer = CTimer::m_snTimeInMilliseconds;
                size_t curTimer = CTimer::m_snTimeInMilliseconds;

                if (curTimer - timer > 60000) {
                    FileMgr::ExportIPL("auto_save.ipl");
                    timer = curTimer;
                }
            }
            Interface.Process();
            Viewport.Process();
        }
    } else {
        if (!bTriedtoHideCursor) {
            D3dHook::SetMouseState(false);
            gConfig.Save();
            bTriedtoHideCursor = true;
        }
    }
}

void EditorMgr::ApplyStyle() {
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
