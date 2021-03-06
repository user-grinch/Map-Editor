#include "pch.h"
#include "interface.h"
#include "utils.h"
#include "viewport.h"
#include "editor.h"
#include "objmanager.h"
#include "hotkeys.h"
#include "widgets.h"
#include "filemgr.h"
#include <CHud.h>
#include <CClock.h>
#include <CPopulation.h>
#include <filesystem>
#include <CStreaming.h>

void ContextMenu_Search(std::string& root, std::string& key, std::string& value)
{
    if (ImGui::MenuItem("Add to favourites"))
    {
        int model = std::stoi(value);
        std::string keyName = value + " - " + ObjManager::FindNameFromModel(model);
        Interface::m_favData.m_pJson->m_Data["All"][keyName] = std::to_string(model);
        Interface::m_favData.m_pJson->WriteToDisk();
        CHud::SetHelpMessage("Added to favourites", false, false, false);
    };

    if (ImGui::MenuItem("Copy"))
    {
        ObjManager::ClipBoard::m_nModel = std::stoi(value);
        CHud::SetHelpMessage("Object Copied", false, false, false);
    };
}


void ImportPopup()
{
    std::filesystem::path path = PLUGIN_PATH((char*)"/MapEditor/");
    if (std::filesystem::exists(path))
    {
        ImGui::Spacing();
        ImGui::Text("Info,");
        ImGui::TextWrapped("- Place ipl files in 'MapEditor' directory");
        ImGui::TextWrapped("- Use limit adjuster if you're gonna load a lot of objects!");
        ImGui::Spacing();
        ImGui::TextWrapped("You game may freeze while loading!");
        ImGui::Dummy(ImVec2(0, 20));
        static std::string selectedFileName = "";

        static bool logImports;
        ImGui::Checkbox("Log imports", &logImports);
        Widgets::ShowTooltip("Logs imports line by line in MapEditor.log.\nEnable this if the game crashes while importing\nand you want to know the error line.\n\nNote: Has performance impact!");

        if (ImGui::Button("Import IPL", Utils::GetSize(2)))
        {
            FileMgr::ImportIPL(selectedFileName.c_str(), logImports);
            Interface::m_PopupMenu.m_bShow = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear placed objects", Utils::GetSize(2)))
        {
            for (auto &pObj : ObjManager::m_pPlacedObjs)
            {
                pObj->Remove();
            }
            ObjManager::m_pSelected = nullptr;
            ObjManager::m_pPlacedObjs.clear();
            CHud::SetHelpMessage("Current objects cleared", false, false, false);
        }
        ImGui::Spacing();

        if(ImGui::BeginChild("ImportMenu"))
        {
            for (const auto & entry : std::filesystem::directory_iterator(path))
            {
                if (entry.path().filename().string().ends_with(".ipl"))
                {
                    std::string fileName = entry.path().filename().string();

                    if (ImGui::MenuItem(fileName.c_str(), NULL, selectedFileName == fileName))
                    {
                        selectedFileName = fileName;
                    }
                }
            }

            ImGui::EndChild();
        }
    }
    else
    {
        ImGui::Text("Map Editor folder not found!");
    }
}

void ExportPopup()
{
    ImGui::Spacing();
    ImGui::Text("Notes,");
    ImGui::TextWrapped("Files are exported to '(game dir)/MapEditor' directory.");
    ImGui::TextWrapped("Exisitng files with the same name will be replaced!");
    ImGui::Dummy(ImVec2(0, 20));

    static char buffer[32];
    ImGui::Spacing();
    ImGui::InputTextWithHint("File name##Buffer", "ProjectProps.ipl", buffer, ARRAYSIZE(buffer));
    if (ImGui::IsItemActive())
    {
        Interface::m_bInputLocked = true;
    }
    ImGui::Spacing();
    if (ImGui::Button("Export IPL", Utils::GetSize()))
    {
        if (strcmp(buffer, "") == 0)
        {
            strcpy(buffer, "ProjectProps.ipl");
        }
        FileMgr::ExportIPL(buffer);
        Interface::m_PopupMenu.m_bShow = false;
    }
}


void UpdateFoundPopup()
{
    if (ImGui::Button("Discord server", ImVec2(Utils::GetSize(2))))
    {
        ShellExecute(NULL, "open", DISCORD_INVITE, NULL, NULL, SW_SHOWNORMAL);
    }
    ImGui::SameLine();
    if (ImGui::Button("Downlod page", Utils::GetSize(2)))
    {
        ShellExecute(NULL, "open", "https://github.com/user-grinch/Map-Editor/", NULL, NULL, SW_SHOWNORMAL);
    }
    ImGui::Spacing();
    Widgets::CenterdText("Current version: " EDITOR_VERSION);
    Widgets::CenterdText("Latest version: " + Updater::GetUpdateVersion());
    ImGui::Dummy(ImVec2(0,20));

    ImGui::TextWrapped("A newer version of Map Editor is available with,");
    ImGui::Text("1. New features\n2. Bug fixes\n3. Improvements");
    ImGui::Spacing();
    ImGui::TextWrapped("Click on the `Download page` button and follow the instructions there to update.");
}

void AboutEditorPopup()
{
    if (ImGui::Button("Discord server", ImVec2(Utils::GetSize(2))))
    {
        ShellExecute(NULL, "open", DISCORD_INVITE, NULL, NULL, SW_SHOWNORMAL);
    }
    ImGui::SameLine();
    if (ImGui::Button("Check update", Utils::GetSize(2)))
    {
        Updater::CheckUpdate();
    }
    ImGui::Spacing();
    ImGui::Columns(2, NULL, false);
    ImGui::Text(EDITOR_NAME);
    ImGui::Text("Version: %s", EDITOR_VERSION);
    ImGui::Dummy(ImVec2(0.0f, 20.0f));
    ImGui::Text("Credits:");
    ImGui::Text("1. Plugin SDK");
    ImGui::Text("3. ImGui");
    ImGui::NextColumn();
    ImGui::Text("Author: Grinch_");
    ImGui::Text("Build: %s", BUILD_NUMBER);
    ImGui::Dummy(ImVec2(0.0f, 20.0f));
    ImGui::Text("-");
    ImGui::Text("2. MTA");
    ImGui::Columns(1);
    ImGui::Dummy(ImVec2(0.0f, 20.0f));
    ImGui::TextWrapped("You are not allowed to reupload this modification without explicit permission from the author! You have to link to the official source.");
    ImGui::Dummy(ImVec2(0.0f, 30.0f));
    Widgets::CenterdText("Copyright Grinch_ 2021-2022. All rights reserved");
}

void ControlsPopup()
{
    if (ImGui::BeginChild("Controls"))
    {
        if (ImGui::BeginTable("Controls", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
        {
            // ---------------------------------------------------------------
            // Camera Section
            ImGui::TableNextColumn();
            ImGui::Text("Camera");
            ImGui::TableNextColumn();
            ImGui::Text("-");

            ImGui::TableNextColumn();
            ImGui::Text("Movement");
            ImGui::TableNextColumn();
            ImGui::Text("W A S D");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Speed multiplier");
            ImGui::TableNextColumn();
            ImGui::Text("LShift");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Rotation (view mode)");
            ImGui::TableNextColumn();
            ImGui::Text("Mouse");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Zoom (view mode)");
            ImGui::TableNextColumn();
            ImGui::Text("Mouse wheel");

            // -------------------------------------------------------------
            // Object Section
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("");
            ImGui::TableNextColumn();
            ImGui::Text("");
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Object controls");
            ImGui::TableNextColumn();
            ImGui::Text("-");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Move up/ down (edit mode)");
            ImGui::TableNextColumn();
            ImGui::Text("Mouse wheel");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Rotate left/right (edit mode)");
            ImGui::TableNextColumn();
            ImGui::Text("LCtrl + Mouse wheel");

            // -------------------------------------------------------------
            // Browser Section
            ImGui::TableNextColumn();
            ImGui::Text("");
            ImGui::TableNextColumn();
            ImGui::Text("");
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Browser controls");
            ImGui::TableNextColumn();
            ImGui::Text("-");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Rotate (view mode)");
            ImGui::TableNextColumn();
            ImGui::Text("Mouse");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Zoom (view mode)");
            ImGui::TableNextColumn();
            ImGui::Text("Mouse wheel");

            // -------------------------------------------------------------
            // Toggles
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("");
            ImGui::TableNextColumn();
            ImGui::Text("");
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Toggles");
            ImGui::TableNextColumn();
            ImGui::Text("-");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Open/Close Map Editor");
            ImGui::TableNextColumn();
            ImGui::Text(editorOpenKey.GetNameString().c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Toggle user interface");
            ImGui::TableNextColumn();
            ImGui::Text(toggleUIKey.GetNameString().c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Switch modes");
            ImGui::TableNextColumn();
            ImGui::Text(viewportSwitchKey.GetNameString().c_str());

            // -------------------------------------------------------------
            // Shortcuts
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("");
            ImGui::TableNextColumn();
            ImGui::Text("");
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Shortcuts");
            ImGui::TableNextColumn();
            ImGui::Text("-");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("New object");
            ImGui::TableNextColumn();
            ImGui::Text(newObjKey.GetNameString().c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Copy object");
            ImGui::TableNextColumn();
            ImGui::Text(copyKey.GetNameString().c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Paste object");
            ImGui::TableNextColumn();
            ImGui::Text(pasteKey.GetNameString().c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Delete object");
            ImGui::TableNextColumn();
            ImGui::Text(deleteKey.GetNameString().c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Snap object");
            ImGui::TableNextColumn();
            ImGui::Text(snapKey.GetNameString().c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Copy hovered obj name");
            ImGui::TableNextColumn();
            ImGui::Text(copyHoveredObjName.GetNameString().c_str());

            // -------------------------------------------------------------
            ImGui::EndTable();
        }
        ImGui::EndChild();
    }
}

void WelcomePopup()
{
    ImGui::Spacing();
    if (ImGui::Button("Discord", ImVec2(Utils::GetSize(3))))
    {
        ShellExecute(NULL, "open", DISCORD_INVITE, NULL, NULL, SW_SHOWNORMAL);
    }
    ImGui::SameLine();
    if (ImGui::Button("Controls", Utils::GetSize(3)))
    {
        Interface::m_PopupMenu.m_Title = "Controls";
        Interface::m_PopupMenu.m_pFunc = ControlsPopup;
    }
    ImGui::SameLine();
    if (ImGui::Button("About page", Utils::GetSize(3)))
    {
        Interface::m_PopupMenu.m_Title = "About";
        Interface::m_PopupMenu.m_pFunc = AboutEditorPopup;
    }
    ImGui::Dummy(ImVec2(0, 20));
    if (ImGui::BeginChild("WelcomeScreen"))
    {
        Widgets::CenterdText("Welcome to Map Editor");
        Widgets::CenterdText(std::string("v") + EDITOR_VERSION);
        Widgets::CenterdText(std::string("(") + BUILD_NUMBER + ")");
        ImGui::Dummy(ImVec2(0, 10));

        ImGui::TextWrapped("Click on the `Controls` button to get started. If you have suggestions let me know on Discord.");
        ImGui::Dummy(ImVec2(0, 10));
        ImGui::TextWrapped("Before you go ahead and spam me with bug reports, keep in mind this is a alpha release, not even beta! Do some reasearch if the bug was already reported or not. Try on a FRESH GAME first. I'm NOT fixing bugs with X camera mod or Y graphics mod.");
        ImGui::Dummy(ImVec2(0, 10));
        ImGui::TextWrapped("Partial modloader support has been implemented. Create a folder called 'MapEditor' in modloader folder and put your map mods there.");
        ImGui::Dummy(ImVec2(0, 30));
        ImGui::Text("Please note,");
        ImGui::TextWrapped("1. You are NOT allowed to reupload this modifiction.");
        ImGui::TextWrapped("2. If you're posting it somewhere, link to the official source.");
        ImGui::TextWrapped("3. This can be ignored by getting permission from the author.");
        ImGui::Dummy(ImVec2(0, 10));
        Widgets::CenterdText("Copyright Grinch_ 2021-2022. All rights reserved");
        ImGui::EndChild();
    }
}


void Interface::Init()
{
    m_bAutoSave = gConfig.GetValue("editor.autoSave", true);
    m_bAutoTpToLoc = gConfig.GetValue("editor.autoTpToLoc", false);
    m_bAutoSnapToGround = gConfig.GetValue("editor.autoSnap", true);
     m_bDrawAxisLines = gConfig.GetValue("editor.drawAxisLines", true);
    m_bDrawBoundingBox = gConfig.GetValue("editor.drawBoundingBox", true);
    m_bShowFPS = gConfig.GetValue("editor.showFPS", false);
    m_bShowHoverMenu = gConfig.GetValue("editor.showHoverMenu", true);
    m_bShowSidepanel = gConfig.GetValue("editor.showInfoMenu", true);
    m_bWelcomeShown = gConfig.GetValue("editor.welcomeDisplayed", false);
    
    if (!m_bWelcomeShown)
    {
        m_PopupMenu.m_bShow = true;
        m_PopupMenu.m_Title = "Map Editor";
        m_PopupMenu.m_pFunc = WelcomePopup;
        m_bWelcomeShown = true;
        gConfig.SetValue("editor.welcomeDisplayed", m_bWelcomeShown);
    }
}

void Interface::Process()
{
    if (m_bShowGUI)
    {
        DrawMainMenuBar();
        DrawPopupMenu();
        DrawSidepanel();
    }
}

void Interface::DrawContextMenu()
{
    if (m_ContextMenu.m_bShow)
    {
        if (ImGui::BeginPopupContextWindow("ContextMenu"))
        {
            if (m_ContextMenu.m_Key != "")
            {
                ImGui::Text(m_ContextMenu.m_Key.c_str());
                ImGui::Separator();
            }

            m_ContextMenu.m_pFunc(m_ContextMenu.m_Root, m_ContextMenu.m_Key, m_ContextMenu.m_Val);

            if (ImGui::MenuItem("Close"))
            {
                m_ContextMenu.m_bShow = false;
            }

            ImGui::EndPopup();
        }
    }
}

void Interface::DrawPopupMenu()
{
    if (Updater::IsUpdateAvailable())
    {
        m_PopupMenu.m_bShow = true;
        m_PopupMenu.m_Title = "Update available!";
        m_PopupMenu.m_pFunc = UpdateFoundPopup;
    }

    if (!m_PopupMenu.m_bShow)
    {
        return;
    }

    ImGuiWindowFlags flags =  ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;

    static ImVec2 prevSize;
    ImGui::SetNextWindowSizeConstraints(ImVec2(screen::GetScreenWidth()/4, screen::GetScreenHeight()/1.95), // manually tested
                                        ImVec2(screen::GetScreenWidth()/4, screen::GetScreenHeight()/1.95));

    ImVec2 size = Viewport::GetSize();
    ImGui::SetNextWindowPos(ImVec2((size.x - prevSize.x)/2, (size.y - prevSize.y)/2), ImGuiCond_Always);
    if (ImGui::Begin(m_PopupMenu.m_Title.c_str(), &m_PopupMenu.m_bShow, flags))
    {
        if (m_PopupMenu.m_pFunc)
        {
            m_PopupMenu.m_pFunc();
        }
        prevSize = ImGui::GetWindowSize();
        ImGui::End();
    }

    // Reset state on window close
    if (Updater::IsUpdateAvailable() && !m_PopupMenu.m_pFunc)
    {
        Updater::ResetUpdaterState();
    }
}

void Interface::DrawMainMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        ImGui::Text(EDITOR_TITLE" by Grinch_");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        if(Viewport::m_eState == eViewportState::Edit)
        {
            ImGui::Text("Edit Mode");
        }
        else
        {
            ImGui::Text("View Mode");
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (Interface::m_bShowFPS)
        {
            ImGui::Text("Framerate: %0.1f", ImGui::GetIO().Framerate);
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }

        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Export"))
            {
                m_PopupMenu.m_bShow = true;
                m_PopupMenu.m_Title = "Export to IPL";
                m_PopupMenu.m_pFunc = ExportPopup;
            }
            if (ImGui::MenuItem("Import"))
            {
                m_PopupMenu.m_bShow = true;
                m_PopupMenu.m_Title = "Import from IPL";
                m_PopupMenu.m_pFunc = ImportPopup;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Options"))
        {
            static bool bNoPeds, bNoVehicles;

            if (ImGui::MenuItem("Auto save every minute", NULL, &Interface::m_bAutoSave))
            {
                gConfig.SetValue("editor.autoSave", Interface::m_bAutoSave);
            }
            if (ImGui::MenuItem("Auto snap to ground", NULL, &Interface::m_bAutoSnapToGround))
            {
                gConfig.SetValue("editor.autoSnap", Interface::m_bAutoSnapToGround);
            }
            if (ImGui::MenuItem("Auto teleport to location", NULL, &Interface::m_bAutoTpToLoc))
            {
                gConfig.SetValue("editor.autoTpToLoc", Interface::m_bAutoTpToLoc);
            }
            static bool bFreezeTime;
            if (ImGui::MenuItem("Freeze time", NULL, &bFreezeTime))
            {
                if (bFreezeTime)
                {
                    patch::SetRaw(0x52CF10, (char*)"\xEB\xEF", 2);
                }
                else
                {
                    patch::SetRaw(0x52CF10, (char*)"\x56\x8B", 2);
                }
            }
            if (ImGui::MenuItem("No pedstrain", NULL, &bNoPeds))
            {
                if (bNoPeds)
                {
                    CPopulation::PedDensityMultiplier = 0.0f;
                }
                else
                {
                    CPopulation::PedDensityMultiplier = 1.0f;
                }
            }
            if (ImGui::MenuItem("No vehicles", NULL, &bNoVehicles))
            {
                if (bNoVehicles)
                {
                    patch::SetFloat(0x8A5B20, 0.0f);
                }
                else
                {
                    patch::SetFloat(0x8A5B20, 1.0f);
                }
            }
            if (ImGui::BeginMenu("Weather"))
            {
                if (ImGui::MenuItem("Foggy"))
                {
                    Call<0x438F80>();
                }
                if (ImGui::MenuItem("Overcast"))
                {
                    Call<0x438F60>();
                }
                if (ImGui::MenuItem("Rainy"))
                {
                    Call<0x438F70>();
                }
                if (ImGui::MenuItem("Sandstorm"))
                {
                    Call<0x439590>();
                }
                if (ImGui::MenuItem("Thunderstorm"))
                {
                    Call<0x439570>();
                }
                if (ImGui::MenuItem("Very sunny"))
                {
                    Call<0x438F50>();
                }
                ImGui::EndMenu();
            }
            ImGui::Dummy(ImVec2(0, 10));
            static float mul = 1;
            static bool sliderClicked = false;
            ImGui::Text("Font multip");
            ImGui::SameLine();
            ImGui::Spacing();
            ImGui::SameLine();
            if (ImGui::SliderFloat("##FontMul", &mul, 1, 5))
            {
                sliderClicked = true;
            }
            if (ImGui::IsMouseReleased(0) && sliderClicked)
            {
                gConfig.SetValue("editor.fontMul", mul);
                FontMgr::SetMultiplier(mul);
                sliderClicked = false;
            }
            ImGui::Spacing();
            int hour = CClock::ms_nGameClockHours;
            int minute = CClock::ms_nGameClockMinutes;
            ImGui::Text("Game Hour");
            ImGui::SameLine();
            ImGui::Spacing();
            ImGui::SameLine();
            if (ImGui::InputInt("##GameHOur", &hour))
            {
                if (hour < 0) hour = 23;
                if (hour > 23) hour = 0;
                CClock::ms_nGameClockHours = hour;
            }
            ImGui::Text("Game Minu");
            ImGui::SameLine();
            ImGui::Spacing();
            ImGui::SameLine();
            if (ImGui::InputInt("##GameMinute", &minute))
            {
                if (minute < 0) minute = 59;
                if (minute > 59) minute = 0;
                CClock::ms_nGameClockMinutes = minute;
            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Axis lines", NULL, &m_bDrawAxisLines))
            {
                gConfig.SetValue("editor.drawAxisLines", m_bDrawAxisLines);
            }
            if (ImGui::MenuItem("Bounding box", NULL, &m_bDrawBoundingBox))
            {
                gConfig.SetValue("editor.drawBoundingBox", m_bDrawBoundingBox);
            }
            if (ImGui::MenuItem("Framerate", NULL, &m_bShowFPS))
            {
                gConfig.SetValue("editor.showFPS", m_bShowFPS);
            }
            if (ImGui::MenuItem("Hover tooltip", NULL, &m_bShowHoverMenu))
            {
                gConfig.SetValue("editor.showHoverMenu", m_bShowHoverMenu);
            }
            if (ImGui::MenuItem("Info panel", NULL, &m_bShowSidepanel))
            {
                gConfig.SetValue("editor.showInfoMenu", m_bShowSidepanel);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("About"))
        {
            if (ImGui::MenuItem("Welcome screen", NULL))
            {
                m_PopupMenu.m_bShow = true;
                m_PopupMenu.m_Title = "Map Editor";
                m_PopupMenu.m_pFunc = WelcomePopup;
            }
            if (ImGui::MenuItem("Controls", NULL))
            {
                m_PopupMenu.m_bShow = true;
                m_PopupMenu.m_Title = "Map Editor Controls";
                m_PopupMenu.m_pFunc = ControlsPopup;
            }

            if (ImGui::MenuItem("About Map Editor", NULL))
            {
                m_PopupMenu.m_bShow = true;
                m_PopupMenu.m_Title = "About";
                m_PopupMenu.m_pFunc = AboutEditorPopup;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void Interface::DrawSidepanel()
{
    if (!m_bShowSidepanel)
    {
        return;
    }

    // ---------------------------------------------------
    // do calcualtes for pos & size
    float width = screen::GetScreenWidth();
    float menuWidth = width/5.0f;
    float frameHeight = ImGui::GetFrameHeight();
    ImGui::SetNextWindowPos(ImVec2(width-menuWidth+1.0f, frameHeight));
    ImGui::SetNextWindowSize(ImVec2(menuWidth, screen::GetScreenHeight()-frameHeight));
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar + ImGuiWindowFlags_NoMove
                             + ImGuiWindowFlags_NoCollapse + ImGuiWindowFlags_NoResize;

    // ---------------------------------------------------

    if(ImGui::Begin("Info Menu", NULL, flags))
    {
        if(ImGui::BeginTabBar("Info Tab", ImGuiTabBarFlags_NoTooltip))
        {
            // ---------------------------------------------------
            // Editor Tab
            if(ImGui::BeginTabItem("Editor"))
            {
                if (ImGui::BeginChild("Editor child"))
                {
                    // ---------------------------------------------------
                    // Object info
                    if (ObjManager::m_pSelected)
                    {
                        if (ImGui::CollapsingHeader("Object selection", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            int hObj = CPools::GetObjectRef(ObjManager::m_pSelected);
                            CVector *objPos = &ObjManager::m_pSelected->GetPosition();
                            auto &data = ObjManager::m_objData.Get(ObjManager::m_pSelected);
                            int model = ObjManager::m_pSelected->m_nModelIndex;
                            if (ObjManager::m_pSelected->m_nType == ENTITY_TYPE_OBJECT
                                    || ObjManager::m_pSelected->m_nType == ENTITY_TYPE_BUILDING)
                            {
                                static int bmodel = 0;
                                static std::string name = "";

                                // lets not go over 20000 models each frame
                                if (bmodel != model)
                                {
                                    name = ObjManager::FindNameFromModel(model);
                                    bmodel = model;
                                }

                                ImGui::Spacing();
                                ImGui::SameLine();
                                ImGui::Text("Name: %s", name.c_str());
                            }

                            ImGui::Columns(2, NULL, false);
                            ImGui::Text("Model: %d", model);
                            ImGui::NextColumn();
                            switch(ObjManager::m_pSelected->m_nType)
                            {
                            case ENTITY_TYPE_OBJECT:
                                ImGui::Text("Type: Dynamic");
                                break;
                            case ENTITY_TYPE_BUILDING:
                                ImGui::Text("Type: Static");
                                break;
                            default:
                                ImGui::Text("Type: Unknown");
                            }
                            ImGui::Columns(1);

                            ImGui::Spacing();
                            CVector rot = data.GetRotation();

                            if (ImGui::InputFloat("Pos X##Obj", &objPos->x))
                            {
                                Command<Commands::SET_OBJECT_COORDINATES>(hObj, objPos->x, objPos->y, objPos->z);
                            }
                            if (ImGui::InputFloat("Pos Y##Obj", &objPos->y))
                            {
                                Command<Commands::SET_OBJECT_COORDINATES>(hObj, objPos->x, objPos->y, objPos->z);
                            }
                            if (ImGui::InputFloat("Pos Z##Obj", &objPos->z))
                            {
                                Command<Commands::SET_OBJECT_COORDINATES>(hObj, objPos->x, objPos->y, objPos->z);
                            }

                            ImGui::Spacing();
                            static int rotToggle = 0;
                            ImGui::Columns(2, NULL, false);
                            ImGui::RadioButton("Slider", &rotToggle, 0);
                            ImGui::NextColumn();
                            ImGui::RadioButton("Input", &rotToggle, 1);
                            ImGui::Columns(1);


                            if (rotToggle == 0)
                            {
                                if (ImGui::SliderFloat("Rot X##Obj", &rot.x, 0.0f, 360.0f))
                                {
                                    data.SetRotation(rot);
                                }
                                if (ImGui::SliderFloat("Rot Y##Obj", &rot.y, 0.0f, 360.0f))
                                {
                                    data.SetRotation(rot);
                                }
                                if (ImGui::SliderFloat("Rot Z##Obj", &rot.z, 0.0f, 360.0f))
                                {
                                    data.SetRotation(rot);
                                }
                            }
                            else
                            {
                                if (ImGui::InputFloat("Rot X##Obj", &rot.x))
                                {
                                    rot.x = rot.x > 360.0f ? rot.x - 360.0f : rot.x;
                                    rot.x = rot.x < 0.0f ? rot.x + 360.0f : rot.x;
                                    data.SetRotation(rot);
                                }

                                if (ImGui::InputFloat("Rot Y##Obj", &rot.y))
                                {
                                    rot.y = rot.y > 360.0f ? rot.y - 360.0f : rot.y;
                                    rot.y = rot.y < 0.0f ? rot.y + 360.0f : rot.y;
                                    data.SetRotation(rot);
                                }

                                if (ImGui::InputFloat("Rot Z##Obj", &rot.z))
                                {
                                    rot.z = rot.z > 360.0f ? rot.z - 360.0f : rot.z;
                                    rot.z = rot.z < 0.0f ? rot.z + 360.0f : rot.z;
                                    data.SetRotation(rot);
                                }
                            }

                            ImGui::Spacing();
                            ImGui::Separator();
                        }

                        if (ImGui::CollapsingHeader("Random rotations"))
                        {
                            ImGui::Checkbox("Enable",  &Interface::m_bRandomRot);
                            Widgets::ShowTooltip("Places objects with random rotations in given range");

                            ImGui::Spacing();
                            ImGui::InputFloat2("Rot X##RR", &Interface::m_RandomRotX[0]);
                            ImGui::InputFloat2("Rot Y##RR", &Interface::m_RandomRotY[0]);
                            ImGui::InputFloat2("Rot Z##RR", &Interface::m_RandomRotZ[0]);

                            ImGui::Spacing();
                            ImGui::Separator();
                        }
                    }

                    // ---------------------------------------------------
                    // Camera
                    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        CVector pos = TheCamera.GetPosition();
                        if (ImGui::InputFloat("Pos X##Cam", &pos.x))
                        {
                            Viewport::SetCameraPosn(pos);
                        }
                        if (ImGui::InputFloat("Pos Y##Cam", &pos.y))
                        {
                            Viewport::SetCameraPosn(pos);
                        }
                        if (ImGui::InputFloat("Pos Z##Cam", &pos.z))
                        {
                            Viewport::SetCameraPosn(pos);
                        }
                        ImGui::Spacing();
                        if (ImGui::SliderFloat("Zoom", &Viewport::Viewport::m_fFOV, 10.0f, 115.0f))
                        {
                            TheCamera.LerpFOV(TheCamera.FindCamFOV(), Viewport::Viewport::m_fFOV, 250, true);
                            Command<Commands::CAMERA_PERSIST_FOV>(true);
                        }
                        if (ImGui::SliderInt("Move speed", &Viewport::m_nMoveSpeed, 1, 10))
                        {
                            gConfig.SetValue("editor.moveSpeed", Viewport::m_nMoveSpeed);
                        }
                        ImGui::Spacing();
                        ImGui::Separator();
                    }

                    ImGui::EndChild();
                }
                ImGui::EndTabItem();
            }

            // ---------------------------------------------------
            // Created objects Tab
            if(ImGui::BeginTabItem("Objects"))
            {
                ImGui::Spacing();
                if (ImGui::BeginTabBar("OBJBAR"))
                {
                    if(ImGui::BeginTabItem("All"))
                    {
                        static bool bShowAnyway;
                        static ImGuiTextFilter filter;

                        ImGui::Spacing();
                        if (ObjManager::m_pPlacedObjs.size() == 0)
                        {
                            ImGui::TextWrapped("You haven't placed any objects yet!");
                        }
                        else
                        {
                            if (ImGui::Button("Remove All", Utils::GetSize(bShowAnyway ? 2 : 1)))
                            {
                                for (auto &pObj : ObjManager::m_pPlacedObjs)
                                {
                                    pObj->Remove();
                                }
                                ObjManager::m_pSelected = nullptr;
                                ObjManager::m_pPlacedObjs.clear();
                            }

                            if (ObjManager::m_pPlacedObjs.size() > 500)
                            {
                                if (bShowAnyway)
                                {
                                    ImGui::SameLine();
                                    if (ImGui::Button("Hide list", Utils::GetSize(2)))
                                    {
                                        bShowAnyway = false;
                                    }
                                }
                                else
                                {
                                    ImGui::Spacing();
                                    ImGui::TextWrapped("You've placed more than 500 objects. The list has been hidden to avoid performance issues.");
                                    ImGui::Spacing();
                                    if (ImGui::Button("Show anyway", Utils::GetSize()))
                                    {
                                        bShowAnyway = true;
                                    }
                                }
                            }
                            ImGui::Spacing();

                            if (ObjManager::m_pPlacedObjs.size() < 500 || bShowAnyway)
                            {
                                filter.Draw("Search");
                                if (ImGui::IsItemActive())
                                {
                                    m_bInputLocked = true;
                                }
                                ImGui::Spacing();
                                if (ImGui::BeginChild("Objects child"))
                                {
                                    for (size_t i = 0; i < ObjManager::m_pPlacedObjs.size(); i++)
                                    {
                                        CObject *pObj = ObjManager::m_pPlacedObjs[i];
                                        auto &data = ObjManager::m_objData.Get(pObj);

                                        if (data.m_modelName == "")
                                        {
                                            data.m_modelName = ObjManager::FindNameFromModel(pObj->m_nModelIndex);
                                        }
                                        char buf[32];
                                        sprintf(buf, "%d. %s(%d)", i+1, data.m_modelName.c_str(), pObj->m_nModelIndex);

                                        if (filter.PassFilter(buf) && ImGui::MenuItem(buf))
                                        {
                                            // Setting the camera pos to bounding box
                                            CMatrix *matrix = pObj->GetMatrix();
                                            CColModel *pColModel = pObj->GetColModel();
                                            CVector min = pColModel->m_boundBox.m_vecMin;
                                            CVector max = pColModel->m_boundBox.m_vecMax;

                                            CVector workVec = min;
                                            workVec.x = max.x;
                                            workVec.z = max.z;
                                            CVector vec = *matrix * workVec;

                                            // TODO: Rotate the camera to face the object

                                            Viewport::SetCameraPosn(vec);
                                            ObjManager::m_pSelected = pObj;
                                        }
                                    }

                                    ImGui::EndChild();
                                }
                            }
                        }
                        ImGui::EndTabItem();
                    }
                    if(ImGui::BeginTabItem("Favourites"))
                    {
                        ImGui::Spacing();
                        Widgets::DrawJSON(m_favData, m_ContextMenu, 
                                          [](std::string& root, std::string& key, std::string& value)
                        {
                            ObjManager::ClipBoard::m_nModel = std::stoi(value);
                            CHud::SetHelpMessage("Object Copied", false, false, false);
                        },
                        [](std::string& root, std::string& key, std::string& value)
                        {
                            if (ImGui::MenuItem("Copy"))
                            {
                                ObjManager::ClipBoard::m_nModel = std::stoi(value);
                                CHud::SetHelpMessage("Object Copied", false, false, false);
                            }
                            if (ImGui::MenuItem("Remove"))
                            {
                                m_favData.m_pJson->m_Data["All"].erase(key);
                                m_favData.m_pJson->WriteToDisk();
                            };
                        });
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
                ImGui::EndTabItem();
            }
            //----------------------------------------------------
            // Locations
            if(ImGui::BeginTabItem("Locations"))
            {
                if (ImGui::Button("Set auto teleport location", Utils::GetSize()))
                {
                    CVector pos = TheCamera.GetPosition();
                    gConfig.SetValue("editor.tp.X", pos.x);
                    gConfig.SetValue("editor.tp.Y", pos.y);
                    gConfig.SetValue("editor.tp.Z", pos.z);
                    CHud::SetHelpMessage("Teleport location set", false, false, false);
                }
                ImGui::Spacing();
                if (ImGui::CollapsingHeader("Add new"))
                {
                    static char m_nLocationBuffer[64], m_nInputBuffer[64];
                    ImGui::Spacing();
                    ImGui::InputTextWithHint("Location", "Groove Street", m_nLocationBuffer, IM_ARRAYSIZE(m_nLocationBuffer));
                    if (ImGui::IsItemActive())
                    {
                        m_bInputLocked = true;
                    }
                    ImGui::InputTextWithHint("Coordinates", "x, y, z", m_nInputBuffer, IM_ARRAYSIZE(m_nInputBuffer));
                    if (ImGui::IsItemActive())
                    {
                        m_bInputLocked = true;
                    }
                    ImGui::Spacing();
                    if (ImGui::Button("Insert current coord", Utils::GetSize(2)))
                    {
                        CVector pos = FindPlayerPed()->GetPosition();
                        sprintf(m_nInputBuffer, "%f, %f, %f", pos.x, pos.y, pos.z);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Add location", Utils::GetSize(2)))
                    {
                        m_locData.m_pJson->m_Data["Custom"][m_nLocationBuffer] = ("0, " + std::string(m_nInputBuffer));
                        m_locData.m_pJson->WriteToDisk();
                    }
                    ImGui::Spacing();
                }
                Widgets::DrawJSON(m_locData, m_ContextMenu,
                                  [](std::string& root, std::string& key, std::string& loc)
                {
                    try
                    {
                        int dimension = 0;
                        CVector pos;
                        sscanf(loc.c_str(), "%d,%f,%f,%f", &dimension, &pos.x, &pos.y, &pos.z);
                        FindPlayerPed()->m_nAreaCode = dimension;
                        Command<Commands::SET_AREA_VISIBLE>(dimension);
                        Viewport::SetCameraPosn(pos);
                    }
                    catch (...)
                    {
                        CHud::SetHelpMessage("Invalid location", false, false, false);
                    }
                },
                [](std::string& root, std::string& key, std::string& value)
                {
                    if (ImGui::MenuItem("Remove"))
                    {
                        if (root == "Custom")
                        {
                            m_locData.m_pJson->m_Data["Custom"].erase(key);
                            CHud::SetHelpMessage("Location removed", false, false, false);
                            m_locData.m_pJson->WriteToDisk();
                        }
                        else
                        {
                            CHud::SetHelpMessage("You can only remove custom location", false, false, false);
                        }
                    }
                });
                ImGui::EndTabItem();
            }
            //----------------------------------------------------
            // Browser
            if(ImGui::BeginTabItem("Browser", NULL,
                                   Viewport::Browser::m_bShowNextFrame ? ImGuiTabItemFlags_SetSelected : NULL))
            {
                Viewport::Browser::m_bShowNextFrame = false;
                Viewport::Browser::m_bShown = true;
                static ImGuiTextFilter IplFilter;
                static ImGuiTextFilter totalFilter;
                static std::vector<std::string> iplList;
                static std::vector<std::pair<int, std::string>> *pData;

                int iplCount = iplList.size();
                if (iplCount < 1)
                {
                    for (auto &data : ObjManager::m_vecModelNames)
                    {
                        iplList.push_back(data.first);
                    }
                    pData = &ObjManager::m_vecModelNames[0].second;
                }
                static std::string selected = iplList[0];
                ImGui::Spacing();
                ImGui::Text("Total IPLs loaded: %d", iplCount);
                ImGui::Text("Total models loaded: %d", ObjManager::m_nTotalIDELine);
                ImGui::Spacing();
                ImGui::Checkbox("Auto rotate", &Viewport::Browser::m_bAutoRot);
                ImGui::SliderFloat("Render scale", &Viewport::Browser::m_fScale, 0.0f, 5.0f);
                ImGui::Spacing();
                if (ImGui::Button("Copy render object", Utils::GetSize()))
                {
                    ObjManager::ClipBoard::m_nModel = Viewport::Browser::GetSelected();
                    CHud::SetHelpMessage("Object Copied", false, false, false);
                }
                ImGui::Spacing();
                if(ImGui::BeginTabBar("Broweser Tab", ImGuiTabBarFlags_NoTooltip))
                {
                    if(ImGui::BeginTabItem("IPL search"))
                    {
                        ImGui::Spacing();
                        IplFilter.Draw("Search");
                        if(ImGui::IsItemActive())
                        {
                            m_bInputLocked = true;
                        }

                        if (Widgets::ListBoxStr("IDE", iplList, selected))
                        {
                            for (auto &data : ObjManager::m_vecModelNames)
                            {
                                if (data.first == selected)
                                {
                                    pData = &data.second;
                                    break;
                                }
                            }
                        }
                        ImGui::Spacing();
                        if(ImGui::BeginChild("Browser"))
                        {
                            for (auto &data : *pData)
                            {
                                std::string text = std::to_string(data.first) + " - " +  data.second;
                                if (IplFilter.PassFilter(text.c_str()))
                                {
                                    if (ImGui::MenuItem(text.c_str()))
                                    {
                                        Viewport::Browser::SetSelected(data.first);
                                    }
                                    if (ImGui::IsItemClicked(1))
                                    {
                                        m_ContextMenu.m_pFunc = ContextMenu_Search;
                                        m_ContextMenu.m_Key = data.second;
                                        m_ContextMenu.m_Val = std::to_string(data.first);
                                    }
                                }
                            }
                            DrawContextMenu();
                            ImGui::EndChild();
                        }
                        ImGui::EndTabItem();
                    }
                    if(ImGui::BeginTabItem("Full search"))
                    {
                        static std::vector<std::pair<int, std::string>> searchResults;
                        ImGui::Spacing();
                        ImGui::TextWrapped("Full search is intensive. May lag your game");
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
                        totalFilter.Draw("##Search");
                        if(ImGui::IsItemActive())
                        {
                            m_bInputLocked = true;
                        }
                        if (KeyPressed(VK_RETURN))
                        {
                            goto full_search;
                        }

                        if (ImGui::Button("Search", Utils::GetSize()))
                        {
full_search:
                            searchResults.clear();
                            for (auto &ipl : ObjManager::m_vecModelNames)
                            {
                                for (auto &data : ipl.second)
                                {
                                    std::string text = std::to_string(data.first) + " - " +  data.second;

                                    if (totalFilter.PassFilter(text.c_str()))
                                    {
                                        searchResults.push_back({data.first, data.second});
                                    }
                                }
                            }
                        }
                        ImGui::Spacing();
                        if(ImGui::BeginChild("FS Child"))
                        {
                            for (auto &data : searchResults)
                            {
                                std::string text = std::to_string(data.first) + " - " +  data.second;
                                if (ImGui::MenuItem(text.c_str()))
                                {
                                    Viewport::Browser::SetSelected(data.first);
                                }
                                if (ImGui::IsItemClicked(1))
                                {
                                    m_ContextMenu.m_pFunc = ContextMenu_Search;
                                    m_ContextMenu.m_Key = data.second;
                                    m_ContextMenu.m_Val = std::to_string(data.first);
                                }
                            }
                            DrawContextMenu();
                            ImGui::EndChild();
                        }
                        ImGui::EndTabItem();
                    }
                    if(ImGui::BeginTabItem("Favourites"))
                    {
                        ImGui::Spacing();
                        Widgets::DrawJSON(m_favData, m_ContextMenu,
                                          [](std::string& root, std::string& key, std::string& model)
                        {
                            Viewport::Viewport::Browser::SetSelected((size_t)std::stoi(model));
                        },
                        [](std::string& root, std::string& key, std::string& value)
                        {
                            if (ImGui::MenuItem("Copy"))
                            {
                                ObjManager::ClipBoard::m_nModel = std::stoi(value);
                                CHud::SetHelpMessage("Object Copied", false, false, false);
                            }
                            if (ImGui::MenuItem("Remove"))
                            {
                                m_favData.m_pJson->m_Data["All"].erase(key);
                                m_favData.m_pJson->WriteToDisk();
                            };
                        });
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
                ImGui::EndTabItem();
            }
            else
            {
                Viewport::Browser::m_bShown = false;
            }
            // ---------------------------------------------------
            ImGui::EndTabBar();
        }
        ImGui::End();
    }
    DrawContextMenu();
}

