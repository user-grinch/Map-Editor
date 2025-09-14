#include "pch.h"
#include "editor.h"
#include "viewport.h"
#include "interface.h"
#include <CHud.h>
#include <CMenuManager.h>
#include "filemgr.h"
#include "entitymgr.h"

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
            std::string name = EntMgr.FindNameFromModel(Viewport.m_HoveredEntity->m_nModelIndex);
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

            if (Interface.m_bAutoSave && EntMgr.m_pPlaced.size() > 0) {
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
		ImGuiStyle& style = ImGui::GetStyle();
	
	style.Alpha = 1.0f;
	style.DisabledAlpha = 1.0f;
	style.WindowPadding = ImVec2(8.100000381469727f, 8.100000381469727f);
	style.WindowRounding = 2.0f;
	style.WindowBorderSize = 0.0f;
	style.WindowMinSize = ImVec2(20.0f, 20.0f);
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_None;
	style.ChildRounding = 2.0f;
	style.ChildBorderSize = 0.0f;
	style.PopupRounding = 2.0f;
	style.PopupBorderSize = 0.0f;
	style.FramePadding = ImVec2(5.0f, 5.0f);
	style.FrameRounding = 2.0f;
	style.FrameBorderSize = 0.0f;
	style.ItemSpacing = ImVec2(12.0f, 6.0f);
	style.ItemInnerSpacing = ImVec2(6.0f, 3.0f);
	style.CellPadding = ImVec2(8.100000381469727f, 8.100000381469727f);
	style.IndentSpacing = 20.0f;
	style.ColumnsMinSpacing = 6.0f;
	style.ScrollbarSize = 12.0f;
	style.ScrollbarRounding = 2.0f;
	style.GrabMinSize = 12.0f;
	style.GrabRounding = 2.0f;
	style.TabRounding = 2.0f;
	style.TabBorderSize = 0.0f;
	style.TabMinWidthForCloseButton = 0.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);
	
	style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.2745098173618317f, 0.3176470696926117f, 0.4509803950786591f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.5372549295425415f, 0.5529412031173706f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 1.0f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 1.0f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.5215686559677124f, 0.6000000238418579f, 0.7019608020782471f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.03921568766236305f, 0.9803921580314636f, 0.9803921580314636f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.0f, 0.2901960909366608f, 0.5960784554481506f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.9960784316062927f, 0.4745098054409027f, 0.6980392336845398f, 1.0f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0.501960813999176f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0.501960813999176f);
}