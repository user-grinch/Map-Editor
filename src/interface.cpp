#include "pch.h"
#include "interface.h"
#include "utils/utils.h"
#include "viewport.h"
#include "editor.h"
#include "objectmgr.h"
#include "utils/widget.h"
#include "filemgr.h"
#include <CHud.h>
#include <CClock.h>
#include <CPopulation.h>
#include <filesystem>
#include <CStreaming.h>

void ContextMenu_Search(std::string& root, std::string& key, std::string& value) {
    if (ImGui::MenuItem("Add to favourites")) {
        int model = std::stoi(value);
        std::string keyName = value + " - " + ObjMgr.FindNameFromModel(model);
        Interface.m_favData.m_pData->Set(std::format("Favourites.{}", keyName).c_str(), model);
        Interface.m_favData.m_pData->Save();
        Interface.m_favData.UpdateSearchList();
        CHud::SetHelpMessage("Added to favourites", false, false, false);
    };

    if (ImGui::MenuItem("Copy")) {
        ObjMgr.ClipBoard.m_nModel = std::stoi(value);
        CHud::SetHelpMessage("Object Copied", false, false, false);
    };
}


void ImportPopup() {
    std::filesystem::path path = PLUGIN_PATH((char*)"/"FILE_NAME"/");
    if (std::filesystem::exists(path)) {
        ImGui::Spacing();
        ImGui::Text("Info,");
        ImGui::TextWrapped("- Place IPLs in 'MapEditorSA' directory");
        ImGui::TextWrapped("- Use limit adjuster if you're gonna load a lot of objects!");
        ImGui::Spacing();
        ImGui::TextWrapped("You game may freeze while loading!");
        ImGui::Dummy(ImVec2(0, 20));
        static std::string selectedFileName = "";

        static bool logImports;
        ImGui::Checkbox("Log imports", &logImports);
        Widget::Tooltip("Logs imports line by line in MapEditorSA.log.\nEnable this if the game crashes while importing\nand you want to know the error line.\n\nNote: Has performance impact!");

        if (ImGui::Button("Import IPL", Utils::GetSize(2))) {
            FileMgr::ImportIPL(selectedFileName.c_str(), logImports);
            Interface.m_PopupMenu.m_bShow = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear placed objects", Utils::GetSize(2))) {
            for (auto &pObj : ObjMgr.m_pPlacedObjs) {
                pObj->Remove();
            }
            ObjMgr.m_pSelected = nullptr;
            ObjMgr.m_pPlacedObjs.clear();
            CHud::SetHelpMessage("Current objects cleared", false, false, false);
        }
        ImGui::Spacing();

        if(ImGui::BeginChild("ImportMenu")) {
            for (const auto & entry : std::filesystem::directory_iterator(path)) {
                if (entry.path().filename().string().ends_with(".ipl")) {
                    std::string fileName = entry.path().filename().string();

                    if (ImGui::MenuItem(fileName.c_str(), NULL, selectedFileName == fileName)) {
                        selectedFileName = fileName;
                    }
                }
            }

            ImGui::EndChild();
        }
    } else {
        ImGui::Text("Map Editor folder not found!");
    }
}

void ExportPopup() {
    static char buffer[32];
    ImGui::Spacing();
    ImGui::InputTextWithHint("File name##Buffer", "ProjectProps.ipl", buffer, ARRAYSIZE(buffer));
    if (ImGui::IsItemActive()) {
        Interface.m_bInputLocked = true;
    }
    ImGui::Spacing();
    if (ImGui::Button("Export IPL", Utils::GetSize())) {
        if (strcmp(buffer, "") == 0) {
            strcpy(buffer, "Untitled.ipl");
        }

        std::string fullPath = std::string(PLUGIN_PATH((char*)FILE_NAME"/")) + buffer;
        if (std::filesystem::exists(fullPath)) {
            auto temp = std::move(Interface.m_PopupMenu);
            Interface.m_PopupMenu.m_Title = "Replace Confirmation Dialog";
            Interface.m_PopupMenu.m_pFunc = [temp](){
                ImGui::Spacing();
                ImGui::TextWrapped("Another file with the same name already exists. Would you like to replace it?");
                ImGui::Dummy(ImVec2(0, 20));
                if (ImGui::Button("Yes", Widget::CalcSize(2))) {
                    FileMgr::ExportIPL(buffer);
                    Interface.m_PopupMenu.m_bShow = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("No", Widget::CalcSize(2))) {
                    Interface.m_PopupMenu = std::move(temp);
                }
            };
        } else {
            FileMgr::ExportIPL(buffer);
            Interface.m_PopupMenu.m_bShow = false;
        }
        
    }
}

void UpdateFoundPopup() {
    if (ImGui::Button("Discord server", ImVec2(Utils::GetSize(2)))) {
        ShellExecute(NULL, "open", DISCORD_INVITE, NULL, NULL, SW_SHOWNORMAL);
    }
    ImGui::SameLine();
    if (ImGui::Button("Downlod page", Utils::GetSize(2))) {
        ShellExecute(NULL, "open", "https://github.com/user-grinch/Map-Editor/", NULL, NULL, SW_SHOWNORMAL);
    }
    ImGui::Spacing();
    Widget::TextCentered("Current version: " EDITOR_VERSION);
    Widget::TextCentered("Latest version: " + Updater::GetUpdateVersion());
    ImGui::Dummy(ImVec2(0,20));

    ImGui::TextWrapped("A newer version of Map Editor is available with,");
    ImGui::Text("1. New features\n2. Bug fixes\n3. Improvements");
    ImGui::Spacing();
    ImGui::TextWrapped("Click on the `Download page` button and follow the instructions there to update.");
}

void AboutEditorPopup() {
    if (ImGui::Button("Discord server", ImVec2(Utils::GetSize(2)))) {
        ShellExecute(NULL, "open", DISCORD_INVITE, NULL, NULL, SW_SHOWNORMAL);
    }
    ImGui::SameLine();
    if (ImGui::Button("Check update", Utils::GetSize(2))) {
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
    ImGui::Text("Build: %s", __DATE__);
    ImGui::Dummy(ImVec2(0.0f, 20.0f));
    ImGui::Text("-");
    ImGui::Text("2. MTA");
    ImGui::Columns(1);
    ImGui::Dummy(ImVec2(0.0f, 20.0f));
    ImGui::TextWrapped("You are not allowed to reupload this modification without explicit permission from the author! You have to link to the official source.");
    ImGui::Dummy(ImVec2(0.0f, 30.0f));
    Widget::TextCentered("Copyright Grinch_ 2021-2022");
    Widget::TextCentered("All rights reserved");
}

void ControlsPopup() {
    if (ImGui::BeginChild("Controls")) {
        if (ImGui::BeginTable("Controls", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
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

void WelcomePopup() {
    ImGui::Spacing();
    if (ImGui::Button("Discord", ImVec2(Utils::GetSize(3)))) {
        ShellExecute(NULL, "open", DISCORD_INVITE, NULL, NULL, SW_SHOWNORMAL);
    }
    ImGui::SameLine();
    if (ImGui::Button("GitHub", Utils::GetSize(3))) {
        ShellExecute(NULL, "open", GITHUB_LINK, NULL, NULL, SW_SHOWNORMAL);
    }
    ImGui::SameLine();
    if (ImGui::Button("Patreon", ImVec2(Utils::GetSize(3)))) {
        ShellExecute(NULL, "open", PATREON_LINK, NULL, NULL, SW_SHOWNORMAL);
    }
    if (ImGui::Button("Editor controls", Utils::GetSize(2))) {
        Interface.m_PopupMenu.m_Title = "Controls";
        Interface.m_PopupMenu.m_pFunc = ControlsPopup;
    }
    ImGui::SameLine();
    if (ImGui::Button("About page", Utils::GetSize(2))) {
        Interface.m_PopupMenu.m_Title = "About";
        Interface.m_PopupMenu.m_pFunc = AboutEditorPopup;
    }
    
    ImGui::Dummy(ImVec2(0, 20));
    if (ImGui::BeginChild("WelcomeScreen")) {
        Widget::TextCentered("Welcome to Map Editor");
        Widget::TextCentered(std::string("v") + EDITOR_VERSION);
        Widget::TextCentered(std::string("(") + __DATE__ + ")");
        ImGui::Dummy(ImVec2(0, 10));

        ImGui::TextWrapped("Click on the `Controls` button to get started. If you have suggestions let me know on Discord.");
        ImGui::Dummy(ImVec2(0, 10));
        ImGui::TextWrapped("Partial modloader support has been implemented. Create a folder called 'MapEditor' in modloader folder and put your map mods there.");
        ImGui::Dummy(ImVec2(0, 30));
        Widget::TextCentered("Consider supporting on Patreon!");
        ImGui::Dummy(ImVec2(0, 10));
        Widget::TextCentered("Copyright Grinch_ 2021-2023");
        Widget::TextCentered("All rights reserved");
        ImGui::EndChild();
    }
}

InterfaceMgr Interface;
InterfaceMgr::InterfaceMgr() {
    Events::initGameEvent += [this](){
        m_bAutoSave = gConfig.Get("editor.autoSave", true);
        m_bAutoTpToLoc = gConfig.Get("editor.autoTpToLoc", false);
        m_bAutoSnapToGround = gConfig.Get("editor.autoSnap", true);
        m_bDrawAxisLines = gConfig.Get("editor.drawAxisLines", false);
        m_bDrawBoundingBox = gConfig.Get("editor.drawBoundingBox", false);
        m_bShowFPS = gConfig.Get("editor.showFPS", false);
        m_bShowHoverMenu = gConfig.Get("editor.showHoverMenu", true);
        m_bShowSidepanel = gConfig.Get("editor.showInfoMenu", true);
        m_bWelcomeShown = gConfig.Get("editor.welcomeDisplayed", false);

        if (!m_bWelcomeShown) {
            m_PopupMenu.m_bShow = true;
            m_PopupMenu.m_Title = "Map Editor";
            m_PopupMenu.m_pFunc = WelcomePopup;
            m_bWelcomeShown = true;
            gConfig.Set("editor.welcomeDisplayed", m_bWelcomeShown);
        }
    };
}

void InterfaceMgr::Process() {
    if (m_bShowGUI) {
        DrawMainMenuBar();
        DrawPopupMenu();
        DrawSidepanel();
    }
}

void InterfaceMgr::DrawContextMenu() {
    if (m_ContextMenu.m_bShow) {
        if (ImGui::BeginPopupContextWindow("ContextMenu")) {
            if (m_ContextMenu.m_Key != "") {
                ImGui::Text(m_ContextMenu.m_Key.c_str());
                ImGui::Separator();
            }

            m_ContextMenu.m_pFunc(m_ContextMenu.m_Root, m_ContextMenu.m_Key, m_ContextMenu.m_Val);

            if (ImGui::MenuItem("Close")) {
                m_ContextMenu.m_bShow = false;
            }

            ImGui::EndPopup();
        }
    }
}

void InterfaceMgr::DrawPopupMenu() {
    if (Updater::IsUpdateAvailable()) {
        m_PopupMenu.m_bShow = true;
        m_PopupMenu.m_Title = "Update available!";
        m_PopupMenu.m_pFunc = UpdateFoundPopup;
    }

    if (!m_PopupMenu.m_bShow) {
        return;
    }

    ImGuiWindowFlags flags =  ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;

    static ImVec2 prevSize;
    ImGui::SetNextWindowSizeConstraints(ImVec2(screen::GetScreenWidth()/4, screen::GetScreenHeight()/1.95), // manually tested
                                        ImVec2(screen::GetScreenWidth()/4, screen::GetScreenHeight()/1.95));

    ImVec2 size = Viewport.GetSize();
    ImGui::SetNextWindowPos(ImVec2((size.x - prevSize.x)/2, (size.y - prevSize.y)/2), ImGuiCond_Always);
    if (ImGui::Begin(m_PopupMenu.m_Title.c_str(), &m_PopupMenu.m_bShow, flags)) {
        if (m_PopupMenu.m_pFunc) {
            m_PopupMenu.m_pFunc();
        }
        prevSize = ImGui::GetWindowSize();
        ImGui::End();
    }

    // Reset state on window close
    if (Updater::IsUpdateAvailable() && !m_PopupMenu.m_pFunc) {
        Updater::ResetUpdaterState();
    }
}

void InterfaceMgr::DrawMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        ImGui::Text(EDITOR_TITLE" by Grinch_");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        if(Viewport.m_eState == eViewportState::Edit) {
            ImGui::Text("Edit Mode");
        } else {
            ImGui::Text("View Mode");
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (m_bShowFPS) {
            ImGui::Text("Framerate: %0.1f", ImGui::GetIO().Framerate);
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }

        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Export")) {
                m_PopupMenu.m_bShow = true;
                m_PopupMenu.m_Title = "Export to IPL";
                m_PopupMenu.m_pFunc = ExportPopup;
            }
            if (ImGui::MenuItem("Import")) {
                m_PopupMenu.m_bShow = true;
                m_PopupMenu.m_Title = "Import from IPL";
                m_PopupMenu.m_pFunc = ImportPopup;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Options")) {
            static bool bNoPeds, bNoVehicles;

            if (ImGui::MenuItem("Auto save every minute", NULL, &m_bAutoSave)) {
                gConfig.Set("editor.autoSave", m_bAutoSave);
            }
            if (ImGui::MenuItem("Auto snap to ground", NULL, &m_bAutoSnapToGround)) {
                gConfig.Set("editor.autoSnap", m_bAutoSnapToGround);
            }
            if (ImGui::MenuItem("Auto teleport to location", NULL, &m_bAutoTpToLoc)) {
                gConfig.Set("editor.autoTpToLoc", m_bAutoTpToLoc);
            }
            static bool bFreezeTime;
            if (ImGui::MenuItem("Freeze time", NULL, &bFreezeTime)) {
                if (bFreezeTime) {
                    patch::SetRaw(0x52CF10, (char*)"\xEB\xEF", 2);
                } else {
                    patch::SetRaw(0x52CF10, (char*)"\x56\x8B", 2);
                }
            }
            if (ImGui::MenuItem("No pedstrain", NULL, &bNoPeds)) {
                if (bNoPeds) {
                    CPopulation::PedDensityMultiplier = 0.0f;
                } else {
                    CPopulation::PedDensityMultiplier = 1.0f;
                }
            }
            if (ImGui::MenuItem("No vehicles", NULL, &bNoVehicles)) {
                if (bNoVehicles) {
                    patch::SetFloat(0x8A5B20, 0.0f);
                } else {
                    patch::SetFloat(0x8A5B20, 1.0f);
                }
            }
            if (ImGui::BeginMenu("Weather")) {
                if (ImGui::MenuItem("Foggy")) {
                    Call<0x438F80>();
                }
                if (ImGui::MenuItem("Overcast")) {
                    Call<0x438F60>();
                }
                if (ImGui::MenuItem("Rainy")) {
                    Call<0x438F70>();
                }
                if (ImGui::MenuItem("Sandstorm")) {
                    Call<0x439590>();
                }
                if (ImGui::MenuItem("Thunderstorm")) {
                    Call<0x439570>();
                }
                if (ImGui::MenuItem("Very sunny")) {
                    Call<0x438F50>();
                }
                ImGui::EndMenu();
            }
            ImGui::Dummy(ImVec2(0, 10));
            ImGui::Spacing();
            int hour = CClock::ms_nGameClockHours;
            int minute = CClock::ms_nGameClockMinutes;
            ImGui::Text("Game Hour");
            ImGui::SameLine();
            ImGui::Spacing();
            ImGui::SameLine();
            if (ImGui::InputInt("##GameHOur", &hour)) {
                if (hour < 0) hour = 23;
                if (hour > 23) hour = 0;
                CClock::ms_nGameClockHours = hour;
            }
            ImGui::Text("Game Minu");
            ImGui::SameLine();
            ImGui::Spacing();
            ImGui::SameLine();
            if (ImGui::InputInt("##GameMinute", &minute)) {
                if (minute < 0) minute = 59;
                if (minute > 59) minute = 0;
                CClock::ms_nGameClockMinutes = minute;
            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Axis lines", NULL, &m_bDrawAxisLines)) {
                gConfig.Set("editor.drawAxisLines", m_bDrawAxisLines);
            }
            if (ImGui::MenuItem("Bounding box", NULL, &m_bDrawBoundingBox)) {
                gConfig.Set("editor.drawBoundingBox", m_bDrawBoundingBox);
            }
            if (ImGui::MenuItem("Framerate", NULL, &m_bShowFPS)) {
                gConfig.Set("editor.showFPS", m_bShowFPS);
            }
            if (ImGui::MenuItem("Hover tooltip", NULL, &m_bShowHoverMenu)) {
                gConfig.Set("editor.showHoverMenu", m_bShowHoverMenu);
            }
            if (ImGui::MenuItem("Info panel", NULL, &m_bShowSidepanel)) {
                gConfig.Set("editor.showInfoMenu", m_bShowSidepanel);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("About")) {
            if (ImGui::MenuItem("Welcome screen", NULL)) {
                m_PopupMenu.m_bShow = true;
                m_PopupMenu.m_Title = "Map Editor";
                m_PopupMenu.m_pFunc = WelcomePopup;
            }
            if (ImGui::MenuItem("Controls", NULL)) {
                m_PopupMenu.m_bShow = true;
                m_PopupMenu.m_Title = "Map Editor Controls";
                m_PopupMenu.m_pFunc = ControlsPopup;
            }

            if (ImGui::MenuItem("About Map Editor", NULL)) {
                m_PopupMenu.m_bShow = true;
                m_PopupMenu.m_Title = "About";
                m_PopupMenu.m_pFunc = AboutEditorPopup;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void InterfaceMgr::DrawSidepanel() {
    if (!m_bShowSidepanel) {
        return;
    }

    // ---------------------------------------------------
    // do calcualtes for pos & size
    float width = screen::GetScreenWidth();
    float menuWidth = width/4.8f;
    float frameHeight = ImGui::GetFrameHeight();
    ImGui::SetNextWindowPos(ImVec2(width-menuWidth+1.0f, frameHeight));
    ImGui::SetNextWindowSize(ImVec2(menuWidth, screen::GetScreenHeight()-frameHeight));
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar + ImGuiWindowFlags_NoMove
                             + ImGuiWindowFlags_NoCollapse + ImGuiWindowFlags_NoResize;

    // ---------------------------------------------------

    if(ImGui::Begin("Info Menu", NULL, flags)) {
        if(ImGui::BeginTabBar("Info Tab", ImGuiTabBarFlags_NoTooltip)) {
            // ---------------------------------------------------
            // Editor Tab
            if(ImGui::BeginTabItem("Editor")) {
                if (ImGui::BeginChild("Editor child")) {
                    // ---------------------------------------------------
                    // Object info
                    if (ObjMgr.m_pSelected) {
                        if (ImGui::CollapsingHeader("Object selection", ImGuiTreeNodeFlags_DefaultOpen)) {
                            int hObj = CPools::GetObjectRef(ObjMgr.m_pSelected);
                            CVector *objPos = &ObjMgr.m_pSelected->GetPosition();
                            auto &data = ObjMgr.m_objData.Get(ObjMgr.m_pSelected);
                            int model = ObjMgr.m_pSelected->m_nModelIndex;
                            if (ObjMgr.m_pSelected->m_nType == ENTITY_TYPE_OBJECT
                                    || ObjMgr.m_pSelected->m_nType == ENTITY_TYPE_BUILDING) {
                                static int bmodel = 0;
                                static std::string name = "";

                                // lets not go over 20000 models each frame
                                if (bmodel != model) {
                                    name = ObjMgr.FindNameFromModel(model);
                                    bmodel = model;
                                }

                                ImGui::Spacing();
                                ImGui::SameLine();
                                ImGui::Text("Name: %s", name.c_str());
                            }

                            ImGui::Columns(2, NULL, false);
                            ImGui::Text("Model: %d", model);
                            ImGui::NextColumn();
                            switch(ObjMgr.m_pSelected->m_nType) {
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

                            if (ImGui::InputFloat("Pos X##Obj", &objPos->x)) {
                                Command<Commands::SET_OBJECT_COORDINATES>(hObj, objPos->x, objPos->y, objPos->z);
                            }
                            if (ImGui::InputFloat("Pos Y##Obj", &objPos->y)) {
                                Command<Commands::SET_OBJECT_COORDINATES>(hObj, objPos->x, objPos->y, objPos->z);
                            }
                            if (ImGui::InputFloat("Pos Z##Obj", &objPos->z)) {
                                Command<Commands::SET_OBJECT_COORDINATES>(hObj, objPos->x, objPos->y, objPos->z);
                            }

                            ImGui::Spacing();

                            if (ImGui::SliderFloat("Rot X##Obj", &rot.x, 0.0f, 360.0f)) {
                                data.SetRotation(rot);
                            }
                            if (ImGui::SliderFloat("Rot Y##Obj", &rot.y, 0.0f, 360.0f)) {
                                data.SetRotation(rot);
                            }
                            if (ImGui::SliderFloat("Rot Z##Obj", &rot.z, 0.0f, 360.0f)) {
                                data.SetRotation(rot);
                            }

                            ImGui::Spacing();
                            ImGui::Separator();
                        }

                        if (ImGui::CollapsingHeader("Random rotations")) {
                            ImGui::Checkbox("Enable",  &m_bRandomRot);
                            Widget::Tooltip("Places objects with random rotations in given range");

                            ImGui::Spacing();
                            ImGui::InputFloat2("Rot X##RR", &m_RandomRotX[0]);
                            ImGui::InputFloat2("Rot Y##RR", &m_RandomRotY[0]);
                            ImGui::InputFloat2("Rot Z##RR", &m_RandomRotZ[0]);

                            ImGui::Spacing();
                            ImGui::Separator();
                        }
                    }

                    // ---------------------------------------------------
                    // Camera
                    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
                        CVector pos = TheCamera.GetPosition();
                        if (ImGui::InputFloat("Pos X##Cam", &pos.x)) {
                            Viewport.SetCameraPosn(pos);
                        }
                        if (ImGui::InputFloat("Pos Y##Cam", &pos.y)) {
                            Viewport.SetCameraPosn(pos);
                        }
                        if (ImGui::InputFloat("Pos Z##Cam", &pos.z)) {
                            Viewport.SetCameraPosn(pos);
                        }
                        ImGui::Spacing();
                        if (ImGui::SliderFloat("Zoom", &Viewport.m_fFOV, 10.0f, 115.0f)) {
                            TheCamera.LerpFOV(TheCamera.FindCamFOV(), Viewport.m_fFOV, 250, true);
                            Command<Commands::CAMERA_PERSIST_FOV>(true);
                        }
                        if (ImGui::SliderFloat("Move speed", &Viewport.m_nMoveSpeed, 0.1f, 1.0f)) {
                            gConfig.Set("editor.moveSpeed", Viewport.m_nMoveSpeed);
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
            if(ImGui::BeginTabItem("Objects")) {
                ImGui::Spacing();
                if (ImGui::BeginTabBar("OBJBAR")) {
                    if(ImGui::BeginTabItem("Placed")) {
                        static ImGuiTextFilter filter;
                        ImGui::Spacing();
                        if (ObjMgr.m_pPlacedObjs.size() == 0) {
                            Widget::TextCentered("You haven't placed any objects yet!");
                        } else {
                            if (ImGui::Button("Remove All", Utils::GetSize(1))) {
                                for (auto &pObj : ObjMgr.m_pPlacedObjs) {
                                    pObj->Remove();
                                }
                                ObjMgr.m_pSelected = nullptr;
                                ObjMgr.m_pPlacedObjs.clear();
                            }
                        
                            ImGui::Spacing();
                            ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
                            Widget::Filter("##Search", filter, "Search");
                            if (ImGui::IsItemActive()) {
                                m_bInputLocked = true;
                            }
                            ImGui::Spacing();
                            if (ImGui::BeginChild("Objects child")) {
                                for (size_t i = 0; i < ObjMgr.m_pPlacedObjs.size(); i++) {
                                    CObject *pObj = ObjMgr.m_pPlacedObjs[i];
                                    auto &data = ObjMgr.m_objData.Get(pObj);

                                    if (data.m_modelName == "") {
                                        data.m_modelName = ObjMgr.FindNameFromModel(pObj->m_nModelIndex);
                                    }
                                    char buf[32];
                                    sprintf(buf, "%d. %s(%d)", i+1, data.m_modelName.c_str(), pObj->m_nModelIndex);

                                    if (filter.PassFilter(buf) && ImGui::MenuItem(buf)) {
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

                                        Viewport.SetCameraPosn(vec);
                                        ObjMgr.m_pSelected = pObj;
                                    }
                                }

                                ImGui::EndChild();
                            }
                        }
                        ImGui::EndTabItem();
                    }
                    if(ImGui::BeginTabItem("Favourites")) {
                        ImGui::Spacing();
                        Widget::DataListFav(m_favData, [](std::string& root, std::string& key, std::string& value) {
                            ObjMgr.ClipBoard.m_nModel = std::stoi(value);
                            CHud::SetHelpMessage("Object Copied", false, false, false);
                        });
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
                ImGui::EndTabItem();
            }
            //----------------------------------------------------
            // Locations
            if(ImGui::BeginTabItem("Location")) {
                ImGui::Spacing();
                if (ImGui::Button("Set auto teleport location", Utils::GetSize())) {
                    CVector pos = TheCamera.GetPosition();
                    gConfig.Set("editor.tp.X", pos.x);
                    gConfig.Set("editor.tp.Y", pos.y);
                    gConfig.Set("editor.tp.Z", pos.z);
                    CHud::SetHelpMessage("Teleport location set", false, false, false);
                }
                ImGui::Spacing();
                Widget::DataList(m_locData,
                [](std::string& root, std::string& key, std::string& loc) {
                    try {
                        int dimension = 0;
                        CVector pos;
                        sscanf(loc.c_str(), "%d,%f,%f,%f", &dimension, &pos.x, &pos.y, &pos.z);
                        FindPlayerPed()->m_nAreaCode = dimension;
                        Command<Commands::SET_AREA_VISIBLE>(dimension);
                        Viewport.SetCameraPosn(pos);
                    } catch (...) {
                        CHud::SetHelpMessage("Invalid location", false, false, false);
                    }
                }, [this](){
                    static char m_nLocationBuffer[64], m_nInputBuffer[64];
                    ImGui::Spacing();
                    ImGui::InputTextWithHint("Location", "Groove Street", m_nLocationBuffer, IM_ARRAYSIZE(m_nLocationBuffer));
                    if (ImGui::IsItemActive()) {
                        m_bInputLocked = true;
                    }
                    ImGui::InputTextWithHint("Coordinates", "x, y, z", m_nInputBuffer, IM_ARRAYSIZE(m_nInputBuffer));
                    if (ImGui::IsItemActive()) {
                        m_bInputLocked = true;
                    }
                    ImGui::Spacing();
                    if (ImGui::Button("Insert coord", Utils::GetSize(2))) {
                        CVector pos = FindPlayerPed()->GetPosition();
                        sprintf(m_nInputBuffer, "%f, %f, %f", pos.x, pos.y, pos.z);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Add location", Utils::GetSize(2))) {
                        m_locData.m_pData->Set(std::format("Custom.{}", m_nLocationBuffer).c_str(), ("0, " + std::string(m_nInputBuffer)));
                        m_locData.m_pData->Save();
                    }
                });
                ImGui::EndTabItem();
            }
            //----------------------------------------------------
            // Browser
            if(ImGui::BeginTabItem("Browser", NULL,
                                   Viewport.Browser.m_bShowNextFrame ? ImGuiTabItemFlags_SetSelected : NULL)) {
                Viewport.Browser.m_bShowNextFrame = false;
                Viewport.Browser.m_bShown = true;
                static ImGuiTextFilter IplFilter;
                static ImGuiTextFilter totalFilter;
                static std::vector<std::string> iplList;
                static std::vector<std::pair<int, std::string>> *pData;

                int iplCount = iplList.size();
                if (iplCount < 1) {
                    for (auto &data : ObjMgr.m_vecModelNames) {
                        iplList.push_back(data.first);
                    }
                    pData = &ObjMgr.m_vecModelNames[0].second;
                }
                static std::string selected = iplList[0];
                ImGui::Spacing();
                ImGui::Text("Total IPLs loaded: %d", iplCount);
                ImGui::Text("Total models loaded: %d", ObjMgr.m_nTotalIDELine);
                ImGui::Spacing();
                ImGui::Checkbox("Auto rotate", &Viewport.Browser.m_bAutoRot);
                ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth()/2);
                ImGui::SliderFloat("Render scale", &Viewport.Browser.m_fScale, 0.0f, 5.0f);
                ImGui::Spacing();
                if (ImGui::Button("Copy object", Utils::GetSize())) {
                    ObjMgr.ClipBoard.m_nModel = Viewport.Browser.GetSelected();
                    CHud::SetHelpMessage("Object Copied", false, false, false);
                }
                ImGui::Spacing();
                if(ImGui::BeginTabBar("Broweser Tab", ImGuiTabBarFlags_NoTooltip)) {
                    if(ImGui::BeginTabItem("IDE Search")) {
                        ImGui::Spacing();
                        ImGui::PushItemWidth((ImGui::GetWindowContentRegionWidth() - ImGui::GetStyle().ItemSpacing.x)/2);

                        if (Widget::ListBox("##IDEBox", iplList, selected)) {
                             for (auto &data : ObjMgr.m_vecModelNames) {
                                if (data.first == selected) {
                                    pData = &data.second;
                                    break;
                                }
                            }
                        }
                        ImGui::SameLine();
                        Widget::Filter("##Filter", IplFilter, "Search");
                        ImGui::PopItemWidth();
                        
                        if(ImGui::IsItemActive()) {
                            m_bInputLocked = true;
                        }
                        ImGui::Spacing();
                        if(ImGui::BeginChild("Browser")) {
                            for (auto &data : *pData) {
                                std::string text = std::to_string(data.first) + " - " +  data.second;
                                if (IplFilter.PassFilter(text.c_str())) {
                                    if (ImGui::MenuItem(text.c_str())) {
                                        Viewport.Browser.SetSelected(data.first);
                                    }
                                    if (ImGui::IsItemClicked(1)) {
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
                    if(ImGui::BeginTabItem("Full search")) {
                        static std::vector<std::pair<int, std::string>> searchResults;
                        ImGui::Spacing();
                        ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
                        Widget::Filter("##Filter", totalFilter, "trashcan");
                        if(ImGui::IsItemActive()) {
                            m_bInputLocked = true;
                        }
                        if (KeyPressed(VK_RETURN)) {
                            goto full_search;
                        }

                        if (ImGui::Button("Search", Utils::GetSize())) {
full_search:
                            searchResults.clear();
                            for (auto &ipl : ObjMgr.m_vecModelNames) {
                                for (auto &data : ipl.second) {
                                    std::string text = std::to_string(data.first) + " - " +  data.second;

                                    if (totalFilter.PassFilter(text.c_str())) {
                                        searchResults.push_back({data.first, data.second});
                                    }
                                }
                            }
                        }
                        ImGui::Spacing();
                        if(ImGui::BeginChild("FS Child")) {
                            for (auto &data : searchResults) {
                                std::string text = std::to_string(data.first) + " - " +  data.second;
                                if (ImGui::MenuItem(text.c_str())) {
                                    Viewport.Browser.SetSelected(data.first);
                                }
                                if (ImGui::IsItemClicked(1)) {
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
                    if(ImGui::BeginTabItem("Favourites")) {
                        ImGui::Spacing();
                        Widget::DataListFav(m_favData,
                        [](std::string& root, std::string& key, std::string& model) {
                            Viewport.Browser.SetSelected((size_t)std::stoi(model));
                        });
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
                ImGui::EndTabItem();
            } else {
                Viewport.Browser.m_bShown = false;
            }
            // ---------------------------------------------------
            ImGui::EndTabBar();
        }
        ImGui::End();
    }
    DrawContextMenu();
}

