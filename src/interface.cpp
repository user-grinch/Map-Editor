#include "pch.h"
#include "interface.h"
#include "utils/utils.h"
#include "viewport.h"
#include "editor.h"
#include "entitymgr.h"
#include "utils/widget.h"
#include "filemgr.h"
#include <CHud.h>
#include <CClock.h>
#include <CPopulation.h>
#include <filesystem>
#include <CStreaming.h>
#include <extensions/ScriptCommands.h>
#include <extensions/scripting/ScriptCommandNames.h>

#include "contextmenus.h"
#include "popups.h"
#include "tooltips.h"

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
            Popup.m_bShow = true;
            Popup.m_Title = "Map Editor";
            Popup.m_pFunc = Popup_Welcome;
            m_bWelcomeShown = true;
            gConfig.Set("editor.welcomeDisplayed", m_bWelcomeShown);
        }
        Interface.m_favData.UpdateSearchList(true);
    };
}

void InterfaceMgr::Process() {
    if (m_bShowGUI) {
        DrawMainMenuBar();
        Popup.Draw();
        DrawSidepanel();
        Tooltip.Draw();
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
                Popup.m_bShow = true;
                Popup.m_Title = "Export to IPL";
                Popup.m_pFunc = Popup_Export;
            }
            if (ImGui::MenuItem("Import")) {
                Popup.m_bShow = true;
                Popup.m_Title = "Import from IPL";
                Popup.m_pFunc = Popup_Import;
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
            if (ImGui::MenuItem("Control panel", NULL, &Tooltip.m_bShow)) {
                gConfig.Set("editor.drawControlPanel", Tooltip.m_bShow);
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
                Popup.m_bShow = true;
                Popup.m_Title = "Map Editor";
                Popup.m_pFunc = Popup_Welcome;
            }
            if (ImGui::MenuItem("Controls", NULL)) {
                Popup.m_bShow = true;
                Popup.m_Title = "Map Editor Controls";
                Popup.m_pFunc = Popup_Controls;
            }

            if (ImGui::MenuItem("About Map Editor", NULL)) {
                Popup.m_bShow = true;
                Popup.m_Title = "About";
                Popup.m_pFunc = Popup_AboutEditor;
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
            Tooltip.m_bShow = false; // reset
            if(ImGui::BeginTabItem("Editor")) {
                if (ImGui::BeginChild("Editor child")) {
                    ImGui::Dummy({0, 20});
                    // ---------------------------------------------------
                    // Object infoF
                    if (EntMgr.m_pSelected) {
                        if (ImGui::CollapsingHeader("Object selection", ImGuiTreeNodeFlags_DefaultOpen)) {
                            int hObj = CPools::GetObjectRef(EntMgr.m_pSelected);
                            CVector *objPos = &EntMgr.m_pSelected->GetPosition();
                            auto &data = EntMgr.m_Info.Get(EntMgr.m_pSelected);
                            int model = EntMgr.m_pSelected->m_nModelIndex;
                            if (EntMgr.m_pSelected->m_nType == ENTITY_TYPE_OBJECT
                                    || EntMgr.m_pSelected->m_nType == ENTITY_TYPE_BUILDING) {
                                static int bmodel = 0;
                                static std::string name = "";

                                // lets not go over 20000 models each frame
                                if (bmodel != model) {
                                    name = EntMgr.FindNameFromModel(model);
                                    bmodel = model;
                                }

                                ImGui::Spacing();
                                ImGui::SameLine();
                                ImGui::Text("Name: %s", name.c_str());
                            }

                            ImGui::Columns(2, NULL, false);
                            ImGui::Text("Model: %d", model);
                            ImGui::NextColumn();
                            switch(EntMgr.m_pSelected->m_nType) {
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
                            ImGui::Dummy({0, 15});
                            if (ImGui::Button("Goto", Utils::GetSize(2))) {
                                Action_MoveCamToObject(EntMgr.m_pSelected);
                            }
                            ImGui::SameLine();
                            if (ImGui::Button("Remove", Utils::GetSize(2))) {
                                Action_RemoveSelectedObject();
                            }
                            ImGui::Dummy({0, 10});
                            CVector rot = data.GetEuler();

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
                                data.SetEuler(rot);
                            }
                            if (ImGui::SliderFloat("Rot Y##Obj", &rot.y, 0.0f, 360.0f)) {
                                data.SetEuler(rot);
                            }
                            if (ImGui::SliderFloat("Rot Z##Obj", &rot.z, 0.0f, 360.0f)) {
                                data.SetEuler(rot);
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
                        if (ImGui::SliderFloat("Move speed", &Viewport.m_nMoveSpeedMul, 0.1f, 1.0f)) {
                            gConfig.Set("editor.moveSpeed", Viewport.m_nMoveSpeedMul);
                        }
                        ImGui::Spacing();
                        ImGui::Separator();
                    }

                    if (ImGui::CollapsingHeader("Procedural generation")) {
                        static bool snapToGround = true;
                        static int count = 10, model = 620;
                        static CVector low, high;

                        float width = Utils::GetContentRegionWidth();
                        ImGui::Checkbox("Snap to ground##PG", &snapToGround);
                        ImGui::Spacing();
                        ImGui::PushItemWidth(width / 1.0f);
                        ImGui::InputInt("Model", &model);
                        ImGui::InputInt("Total entities", &count);
                        ImGui::PopItemWidth();
                        ImGui::Dummy({0, 10});
                        
                        ImGui::Spacing();
                        ImGui::PushItemWidth(width / 1.3);
                        ImGui::Text("Top right back");
                        ImGui::InputFloat3("##Top right back", (float*)&high);
                        ImGui::Dummy({0, 2.5});
                        ImGui::Text("Bottom left front");
                        ImGui::InputFloat3("##Bottom left front", (float*)&low);
                        ImGui::PopItemWidth();
                        ImGui::Spacing();
                        if (ImGui::Button("Generate", Utils::GetSize())) {
                            Action_GenerateObjects(low, high, model, snapToGround, count);
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
                        if (EntMgr.m_pPlaced.size() == 0) {
                            Widget::TextCentered("You haven't placed any objects yet!");
                        } else {
                            if (ImGui::Button("Remove All", Utils::GetSize(1))) {
                                for (auto &pObj : EntMgr.m_pPlaced) {
                                    pObj->Remove();
                                }
                                EntMgr.m_pSelected = nullptr;
                                EntMgr.m_pPlaced.clear();
                            }
                        
                            ImGui::Spacing();
                            ImGui::SetNextItemWidth(Utils::GetContentRegionWidth());
                            Widget::Filter("##Search", filter, "Search");
                            if (ImGui::IsItemActive()) {
                                m_bInputLocked = true;
                            }
                            ImGui::Spacing();
                            ImGui::BeginChild("Objects child");
                            for (size_t i = 0; i < EntMgr.m_pPlaced.size(); i++) {
                                CObject *pObj = EntMgr.m_pPlaced[i];
                                auto &data = EntMgr.m_Info.Get(pObj);

                                if (data.m_sModelName == "") {
                                    data.m_sModelName = EntMgr.FindNameFromModel(pObj->m_nModelIndex);
                                }
                                char buf[32];
                                sprintf(buf, "%d. %s(%d)", i+1, data.m_sModelName.c_str(), pObj->m_nModelIndex);

                                if (filter.PassFilter(buf) && ImGui::MenuItem(buf)) {
                                    Action_MoveCamToObject(pObj);
                                }
                                if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                                    ContextMenu.m_bShow = true;
                                    ContextMenu.m_pFunc = ContextMenu_RegularMenu;
                                    ContextMenu.m_Key = data.m_sModelName;
                                    ContextMenu.m_Val = std::to_string((int)pObj);
                                }
                            }
                            ContextMenu.Draw();
                            ImGui::EndChild();
                        }
                        ImGui::EndTabItem();
                    }
                    if(ImGui::BeginTabItem("Favourites")) {
                        ImGui::Spacing();
                        Widget::DataListFav(m_favData, [](std::string& root, std::string& key, std::string& value) {
                            EntMgr.ClipBoard.m_nModel = std::stoi(value);
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
                                   Viewport.m_Renderer.m_bShowNextFrame ? ImGuiTabItemFlags_SetSelected : NULL)) {
                Viewport.m_Renderer.m_bShowNextFrame = false;
                Viewport.m_Renderer.m_bShown = true;      
                Tooltip.m_bShow = true;
                Tooltip.m_pFunc = Tooltip_Browser;

                static ImGuiTextFilter filter;
                static std::string selectedIDE = EntMgr.m_NameList.begin()->first;
                static std::map<EntityMgr::ModelID, EntityMgr::ModelName> *pSelectedIDE = &EntMgr.m_NameList[selectedIDE];
                
                ImGui::Spacing();
                ImGui::Text("Total IDEs loaded: %d", EntMgr.m_IdeList.size());
                ImGui::Text("Total entities loaded: %d", EntMgr.m_nTotalEntities);
                ImGui::Spacing();
                ImGui::Checkbox("Auto rotate", &Viewport.m_Renderer.m_bAutoRot);
                ImGui::SetNextItemWidth(Utils::GetContentRegionWidth()/2);
                ImGui::SliderFloat("Render scale", &Viewport.m_Renderer.m_fScale, 0.0f, 5.0f);
                ImGui::Spacing();
                if (ImGui::Button("Copy object", Utils::GetSize())) {
                    EntMgr.ClipBoard.m_nModel = Viewport.m_Renderer.GetSelected();
                    CHud::SetHelpMessage("Object Copied", false, false, false);
                }
                ImGui::Spacing();
                if(ImGui::BeginTabBar("Broweser Tab", ImGuiTabBarFlags_NoTooltip)) {
                    if(ImGui::BeginTabItem("IDE Search")) {
                        ImGui::Spacing();
                        ImGui::PushItemWidth((Utils::GetContentRegionWidth() - ImGui::GetStyle().ItemSpacing.x)/2);

                        if (Widget::ListBox("##IDEBox", EntMgr.m_IdeList, selectedIDE)) {
                             for (auto &data : EntMgr.m_NameList) {
                                if (data.first == selectedIDE) {
                                    pSelectedIDE = &data.second;
                                    break;
                                }
                            }
                        }
                        ImGui::SameLine();
                        Widget::Filter("##Filter", filter, "Search");
                        ImGui::PopItemWidth();
                        
                        if(ImGui::IsItemActive()) {
                            m_bInputLocked = true;
                        }
                        ImGui::Spacing();
                        ImGui::BeginChild("Browser");
                        for (auto &data : *pSelectedIDE) {
                            std::string text = std::to_string(data.first) + " - " +  data.second;
                            if (filter.PassFilter(text.c_str())) {
                                if (ImGui::MenuItem(text.c_str())) {
                                    Viewport.m_Renderer.SetSelected(data.first);
                                }
                                if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                                    ContextMenu.m_bShow = true;
                                    ContextMenu.m_pFunc = ContextMenu_RegularNoRemoveMenu;
                                    ContextMenu.m_Key = data.second;
                                    ContextMenu.m_Val = std::to_string(data.first);
                                }
                            }
                        }
                        ContextMenu.Draw();
                        ImGui::EndChild();
                        ImGui::EndTabItem();
                    }
                    if(ImGui::BeginTabItem("Full search")) {
                        static std::vector<std::pair<int, std::string>> searchResults;
                        ImGui::Spacing();
                        ImGui::SetNextItemWidth(Utils::GetContentRegionWidth());
                        Widget::Filter("##Filter", filter, "trashcan");
                        if(ImGui::IsItemActive()) {
                            m_bInputLocked = true;
                        }
                        if (KeyPressed(VK_RETURN)) {
                            goto full_search;
                        }

                        if (ImGui::Button("Search", Utils::GetSize())) {
full_search:
                            searchResults.clear();
                            for (auto &ide : EntMgr.m_NameList) {
                                for (auto &data : ide.second) {
                                    std::string text = std::to_string(data.first) + " - " +  data.second;

                                    if (filter.PassFilter(text.c_str())) {
                                        searchResults.push_back({data.first, data.second});
                                    }
                                }
                            }
                        }
                        ImGui::Spacing();
                        ImGui::BeginChild("FS Child");
                        for (auto &data : searchResults) {
                            std::string text = std::to_string(data.first) + " - " +  data.second;
                            if (ImGui::MenuItem(text.c_str())) {
                                Viewport.m_Renderer.SetSelected(data.first);
                            }
                            if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                                ContextMenu.m_bShow = true;
                                ContextMenu.m_pFunc = ContextMenu_RegularNoRemoveMenu;
                                ContextMenu.m_Key = data.second;
                                ContextMenu.m_Val = std::to_string(data.first);
                            }
                        }
                        ContextMenu.Draw();
                        ImGui::EndChild();
                        ImGui::EndTabItem();
                    }
                    if(ImGui::BeginTabItem("Favourites")) {
                        ImGui::Spacing();
                        Widget::DataListFav(m_favData,
                        [](std::string& root, std::string& key, std::string& model) {
                            Viewport.m_Renderer.SetSelected((size_t)std::stoi(model));
                        });
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
                ImGui::EndTabItem();
            } else {
                Viewport.m_Renderer.m_bShown = false;
            }
            // ---------------------------------------------------
            ImGui::EndTabBar();
        }
        ContextMenu.Draw();
        ImGui::End();
    }
    m_bCursorOnSidePanel = ImGui::IsItemHovered();
}