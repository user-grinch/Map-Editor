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

void Interface::SearchContextMenu(std::string& root, std::string& key, std::string& value)
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
        Viewport::COPY_MODEL::m_nModel = std::stoi(value);
        CHud::SetHelpMessage("Object Copied", false, false, false);
    };
}

void Interface::ProcessContextMenu()
{
    if (m_contextMenu.function != nullptr)
    {
        if (ImGui::BeginPopupContextWindow("TMenu"))
        {
            ImGui::Text(m_contextMenu.key.c_str());
            ImGui::Separator();
            
            m_contextMenu.function(m_contextMenu.rootKey, m_contextMenu.key, m_contextMenu.value);

            if (ImGui::MenuItem("Close"))
            {
                m_contextMenu.function = nullptr;
            }

            ImGui::EndPopup();
        }
    }
}

void Interface::ImportMenu()
{
    std::filesystem::path path = PLUGIN_PATH((char*)"/MapEditor/");
    if (std::filesystem::exists(path))
    {
        ImGui::Spacing();
        ImGui::Text("Notes,");
        ImGui::TextWrapped("Files are imported from '(game dir)/MapEditor' directory");
        ImGui::TextWrapped("Imported objects will be merged with current ones!");
        ImGui::TextWrapped("Use a limit adjuster if you're going to load a lot of objects!");
        ImGui::TextWrapped("You game may freeze while loading!");
        ImGui::Dummy(ImVec2(0, 20));
        static std::string selectedFileName = "";

        if (ImGui::Button("Import IPL", Utils::GetSize(2)))
        {
            FileMgr::ImportIPL(selectedFileName.c_str());
            m_bShowPopup = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear objects", Utils::GetSize(2)))
        {
            for (auto &pObj : ObjManager::m_pVecEntities)
            {
                pObj->Remove();
            }
            ObjManager::m_pSelected = nullptr;
            ObjManager::m_pVecEntities.clear();
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

void Interface::ExportMenu()
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
        m_bIsInputLocked = true;
    }
    ImGui::Spacing();
    if (ImGui::Button("Export IPL", Utils::GetSize()))
    {
        if (strcmp(buffer, "") == 0)
        {
            strcpy(buffer, "ProjectProps.ipl");
        }
        FileMgr::ExportIPL(buffer);
        m_bShowPopup = false;
    }
}

void Interface::WelcomeMenu()
{
    ImGui::Spacing();
    if (ImGui::Button("Discord", ImVec2(Utils::GetSize(3))))
	{
		ShellExecute(NULL, "open", DISCORD_INVITE, NULL, NULL, SW_SHOWNORMAL);
	}
    ImGui::SameLine();
    if (ImGui::Button("Controls", Utils::GetSize(3)))
	{
        m_popupTitle = "Controls";
        m_pPopupFunc = EditorControls;
	}
    ImGui::SameLine();
	if (ImGui::Button("About page", Utils::GetSize(3)))
	{
        m_popupTitle = "About";
        m_pPopupFunc = AboutEditorMenu;
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

void Interface::QuickObjectCreateMenu()
{
    static int modelId = DEFAULT_MODEL_ID;
    static std::string modelName = ObjManager::FindNameFromModel(modelId);

    ImGui::Text("Name: %s", modelName.c_str());
    if (ImGui::InputInt("Model", &modelId))
    {
        modelName = ObjManager::FindNameFromModel(modelId);
    }
    if (KeyPressed(VK_RETURN))
    {
        goto create_object;
    }

    ImGui::Spacing();
    if (ImGui::Button("Create", Utils::GetSize(2)))
    {
        create_object:
        int hObj;
        Command<Commands::REQUEST_MODEL>(modelId);
        Command<Commands::LOAD_ALL_MODELS_NOW>();
        Command<Commands::CREATE_OBJECT>(modelId, Viewport::m_vecWorldPos.x, 
        Viewport::m_vecWorldPos.y, Viewport::m_vecWorldPos.z, &hObj);
        Command<Commands::MARK_MODEL_AS_NO_LONGER_NEEDED>(modelId);
        
        CObject *pEntity = CPools::GetObject(hObj);
        auto &data = ObjManager::m_objData.Get(pEntity);
        data.m_modelName = modelName;

        ObjManager::m_pVecEntities.push_back(pEntity);
        ObjManager::m_pSelected = pEntity;

        Interface::m_bShowPopup = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Find models", Utils::GetSize(2)))
    {
        ShellExecute(NULL, "open", "https://dev.prineside.com/en/gtasa_samp_model_id/", NULL, NULL, SW_SHOWNORMAL);
    }
    ImGui::Spacing();
    if (ImGui::Button("Open object browser", Utils::GetSize()))
    {
        Viewport::m_eViewportMode = OBJECT_VIEW_MODE;
        Interface::m_bShowPopup = false;
        Interface::m_bShowPopup = false;
        m_bOpenObjectBrowser = true;
    }
}

void Interface::EditorControls()
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

            // -------------------------------------------------------------
            ImGui::EndTable();
        }
        ImGui::EndChild();
    }
}

void Interface::AboutEditorMenu()
{
    if (ImGui::Button("Discord server", ImVec2(Utils::GetSize(2))))
	{
		ShellExecute(NULL, "open", DISCORD_INVITE, NULL, NULL, SW_SHOWNORMAL);
	}
    ImGui::SameLine();
	if (ImGui::Button("Check update", Utils::GetSize(2)))
	{
		Editor::CheckForUpdate();
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

void Interface::DrawPopupMenu()
{
    if (!m_bShowPopup)
    {
        return;
    }

    ImGuiWindowFlags flags =  ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
    
    static ImVec2 prevSize;
    ImGui::SetNextWindowSizeConstraints(ImVec2(screen::GetScreenWidth()/4, screen::GetScreenHeight()/1.95), // manually tested
                    ImVec2(screen::GetScreenWidth()/4, screen::GetScreenHeight()/1.95));

    ImGui::SetNextWindowPos(ImVec2((Viewport::m_fViewportSize.x - prevSize.x)/2,
     (Viewport::m_fViewportSize.y - prevSize.y)/2), ImGuiCond_Always);
    if (ImGui::Begin(m_popupTitle.c_str(), &m_bShowPopup, flags))
    {
        if (m_pPopupFunc)
        {
            m_pPopupFunc();
        }
        prevSize = ImGui::GetWindowSize();
        ImGui::End();
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
        if(Viewport::m_eViewportMode == EDIT_MODE)
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
        ImGui::Text("Framerate: %0.1f", ImGui::GetIO().Framerate);
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
       
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Export"))
            {
                m_bShowPopup = true;
                m_popupTitle = "Export to IPL";
                m_pPopupFunc = ExportMenu;
            }
            if (ImGui::MenuItem("Import"))
            {
                m_bShowPopup = true;
                m_popupTitle = "Import from IPL";
                m_pPopupFunc = ImportMenu;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Options"))
        {
            static bool bNoPeds, bNoVehicles;
            
            if (ImGui::MenuItem("Auto snap to ground", NULL, &Interface::m_bAutoSnapToGround))
            {
                gConfig.SetValue("editor.autoSnap", Interface::m_bAutoSnapToGround);
            }
            if (ImGui::MenuItem("Axis lines", NULL, &ObjManager::m_bDrawAxisLines))
            {
                gConfig.SetValue("editor.drawAxisLines", ObjManager::m_bDrawAxisLines);
            }
            if (ImGui::MenuItem("Bounding box", NULL, &ObjManager::m_bDrawBoundingBox))
            {
                gConfig.SetValue("editor.drawBoundingBox", ObjManager::m_bDrawBoundingBox);
            }
            if (ImGui::MenuItem("Hover tooltip", NULL, &Viewport::m_bShowHoverMenu))
            {
                gConfig.SetValue("editor.showHoverMenu", Viewport::m_bShowHoverMenu);
            }
            if (ImGui::MenuItem("Info panel", NULL, &m_bShowInfoMenu))
            {
                gConfig.SetValue("editor.showInfoMenu", m_bShowInfoMenu);
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
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("About"))
        {
            if (ImGui::MenuItem("Welcome screen", NULL))
            {
                m_bShowPopup = true;
                m_popupTitle = "Map Editor";
                m_pPopupFunc = WelcomeMenu;
            }
            if (ImGui::MenuItem("Controls", NULL))
            {
                m_bShowPopup = true;
                m_popupTitle = "Map Editor Controls";
                m_pPopupFunc = EditorControls;
            }

            if (ImGui::MenuItem("About Map Editor", NULL))
            {
                m_bShowPopup = true;
                m_popupTitle = "About";
                m_pPopupFunc = AboutEditorMenu;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void Interface::PrintObjInfo(CEntity *pEntity)
{
	int model = pEntity->m_nModelIndex;
	ImGui::Text("Model: %d", model);

	switch(pEntity->m_nType)
	{
		case ENTITY_TYPE_OBJECT:
			ImGui::Text("Type: Object");
			break;
		case ENTITY_TYPE_BUILDING:
			ImGui::Text("Type: Building");
			break;
		default:
			ImGui::Text("Type: Unknown");
	}

	if (pEntity->m_nType == ENTITY_TYPE_OBJECT
	|| pEntity->m_nType == ENTITY_TYPE_BUILDING)
	{
		static int bmodel = 0;
		static std::string name = "";

		// lets not go over 20000 models each frame
		if (bmodel != model)
		{
			name = ObjManager::FindNameFromModel(model);
			bmodel = model;
		}

		ImGui::Text("Name: %s", name.c_str()); 
	}
}

void Interface::DrawInfoMenu()
{
    if (!m_bShowInfoMenu)
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
                        ImGui::Text("Object selection");
                        ImGui::Separator();
                        ImGui::Spacing();
                        if (m_bAutoSnapToGround)
                        {
                            ImGui::Text("Auto snap is enabled.");
                        }
                        ImGui::Spacing();

                        int hObj = CPools::GetObjectRef(ObjManager::m_pSelected);
                        CVector *objPos = &ObjManager::m_pSelected->GetPosition();
                        auto &data = ObjManager::m_objData.Get(ObjManager::m_pSelected);
                        PrintObjInfo(ObjManager::m_pSelected);
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
                        ImGui::Spacing();
                    }

                    // ---------------------------------------------------
                    // Camera
                    ImGui::Text("Camera");
                    ImGui::Separator();
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
                    if (ImGui::SliderFloat("Zoom", &Viewport::m_fFOV, 10.0f, 115.0f))
                    {
                        TheCamera.LerpFOV(TheCamera.FindCamFOV(), Viewport::m_fFOV, 250, true);
		                Command<Commands::CAMERA_PERSIST_FOV>(true);
                    }
                    ImGui::SliderInt("Move speed", &Viewport::m_nMul, 1, 10);
                    ImGui::Spacing();

                    // ---------------------------------------------------
                    // Weather
                    ImGui::Text("Weather");
                    ImGui::Separator();
                    
                    if (ImGui::Button("Foggy", Utils::GetSize(3)))
                    {
                        Call<0x438F80>();
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Overcast", Utils::GetSize(3)))
                    {
                        Call<0x438F60>();
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Rainy", Utils::GetSize(3)))
                    {
                        Call<0x438F70>();
                    }

                    if (ImGui::Button("Sandstorm", Utils::GetSize(3)))
                    {
                        Call<0x439590>();
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Thunderstorm", Utils::GetSize(3)))
                    {
                        Call<0x439570>();
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Very sunny", Utils::GetSize(3)))
                    {
                        Call<0x438F50>();
                    }
                    ImGui::Spacing();
                    
                    // ---------------------------------------------------
                    // Time
                    ImGui::Text("Time");
                    ImGui::Separator();
                    static bool bFreezeTime;
                    if (ImGui::Checkbox("Freeze time", &bFreezeTime))
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
                    ImGui::Spacing();
                    int hour = CClock::ms_nGameClockHours;
                    int minute = CClock::ms_nGameClockMinutes;

                    if (ImGui::InputInt("Hour", &hour))
                    {
                        if (hour < 0) hour = 23;
                        if (hour > 23) hour = 0;
                        CClock::ms_nGameClockHours = hour;
                    }

                    if (ImGui::InputInt("Minute", &minute))
                    {
                        if (minute < 0) minute = 59;
                        if (minute > 59) minute = 0;
                        CClock::ms_nGameClockMinutes = minute;
                    }
                    ImGui::EndChild();
                }
                ImGui::EndTabItem();
            }

            // ---------------------------------------------------
            // Created objects Tab
            if(ImGui::BeginTabItem("Objects"))
            {
                static bool bShowAnyway;
                static ImGuiTextFilter filter;

                ImGui::Spacing();
                if (ObjManager::m_pVecEntities.size() == 0)
                {
                    ImGui::TextWrapped("You haven't placed any objects yet!");
                }
                else
                {
                    if (ImGui::Button("Remove All", Utils::GetSize(bShowAnyway ? 2 : 1)))
                    {
                        for (auto &pObj : ObjManager::m_pVecEntities)
                        {
                            pObj->Remove();
                        }
                        ObjManager::m_pSelected = nullptr;
                        ObjManager::m_pVecEntities.clear();
                    }

                    if (ObjManager::m_pVecEntities.size() > 500)
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

                    if (ObjManager::m_pVecEntities.size() < 500 || bShowAnyway)
                    {
                        filter.Draw("Search");
                        ImGui::Spacing();
                        if (ImGui::BeginChild("Objects child"))
                        {
                            for (size_t i = 0; i < ObjManager::m_pVecEntities.size(); i++)
                            {
                                CObject *pObj = ObjManager::m_pVecEntities[i];
                                auto &data = ObjManager::m_objData.Get(pObj);

                                if (data.m_modelName == "")
                                {
                                    data.m_modelName = ObjManager::FindNameFromModel(pObj->m_nModelIndex);
                                }
                                char buf[32];
                                sprintf(buf, "%d. %s(%d)", i+1, data.m_modelName.c_str(), pObj->m_nModelIndex);

                                if (filter.PassFilter(buf) && ImGui::MenuItem(buf))
                                {
                                    Viewport::SetCameraPosn(pObj->GetPosition());
                                    ObjManager::m_pSelected = pObj;
                                }
                            }
                        
                            ImGui::EndChild();
                        }
                    }
                }            
                ImGui::EndTabItem();
            }
            //----------------------------------------------------
            // Locations
            if(ImGui::BeginTabItem("Locations"))
            {
                if (ImGui::CollapsingHeader("Add new"))
                {
                    static char m_nLocationBuffer[64], m_nInputBuffer[64];
                    ImGui::Spacing();
                    ImGui::InputTextWithHint("Location", "Groove Street", m_nLocationBuffer, IM_ARRAYSIZE(m_nLocationBuffer));
                    if (ImGui::IsItemActive())
                    {
                        m_bIsInputLocked = true;
                    }
                    ImGui::InputTextWithHint("Coordinates", "x, y, z", m_nInputBuffer, IM_ARRAYSIZE(m_nInputBuffer));
                    if (ImGui::IsItemActive())
                    {
                        m_bIsInputLocked = true;
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
                Widgets::DrawJSON(m_locData, 
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
                ProcessContextMenu();
                ImGui::EndTabItem();
            }
            //----------------------------------------------------
            // Browser
            if(ImGui::BeginTabItem("Browser", NULL, 
            m_bOpenObjectBrowser ? ImGuiTabItemFlags_SetSelected : NULL))
            {
                m_bOpenObjectBrowser = false;
                m_bObjectBrowserShown = true;
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
                ImGui::Text("Total IDEs loaded: %d", iplCount);
                ImGui::Text("Total models loaded: %d", ObjManager::totalIDELinesLoaded);
                ImGui::Spacing();
                ImGui::Checkbox("Auto rotate", &Viewport::m_bObjBrowserAutoRot);
                ImGui::SliderFloat("Render scale", &Viewport::m_nRenderScale, 0.0f, 5.0f);
                ImGui::Spacing();
                if (ImGui::Button("Copy render object", Utils::GetSize()))
                {
                    Viewport::COPY_MODEL::m_nModel = m_nBrowserSelectedModelId;
                    CHud::SetHelpMessage("Object Copied", false, false, false);
                }
                ImGui::Spacing();
                if(ImGui::BeginTabBar("Broweser Tab", ImGuiTabBarFlags_NoTooltip))
                {
                    if (ImGui::IsMouseClicked(1))
                    {
                        m_contextMenu.function = nullptr;
                    }

                    if(ImGui::BeginTabItem("IPL search"))
                    {
                        ImGui::Spacing();
                        IplFilter.Draw("Search");
                        if(ImGui::IsItemActive())
                        {
                            m_bIsInputLocked = true;
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
                                        m_nBrowserSelectedModelId = data.first;
                                    }
                                    if (ImGui::IsItemClicked(1))
                                    {
                                        Interface::m_contextMenu.function = SearchContextMenu;
                                        Interface::m_contextMenu.rootKey = "";
                                        Interface::m_contextMenu.key = data.second;
                                        Interface::m_contextMenu.value = std::to_string(data.first);
                                    }
                                }
                            }
                            ProcessContextMenu();
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
                            m_bIsInputLocked = true;
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
                                    m_nBrowserSelectedModelId = data.first;
                                }
                                if (ImGui::IsItemClicked(1))
                                {
                                    Interface::m_contextMenu.function = SearchContextMenu;
                                    Interface::m_contextMenu.rootKey = "";
                                    Interface::m_contextMenu.key = data.second;
                                    Interface::m_contextMenu.value = std::to_string(data.first);
                                }
                            }
                            ProcessContextMenu();
                            ImGui::EndChild();
                        }
                        ImGui::EndTabItem();
                    }
                    if(ImGui::BeginTabItem("Favourites"))
                    {
                        ImGui::Spacing();
                        Widgets::DrawJSON(m_favData, 
                        [](std::string& root, std::string& key, std::string& model)
                        {
                            m_nBrowserSelectedModelId = std::stoi(model);
                        },
                        [](std::string& root, std::string& key, std::string& value)
                        {   
                            if (ImGui::MenuItem("Copy"))
                            {
                                 Viewport::COPY_MODEL::m_nModel = std::stoi(value);
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
                m_bObjectBrowserShown = false;
            }
            // ---------------------------------------------------
            ImGui::EndTabBar();
        }
        ImGui::End();
    }
}

void Interface::UpdateFoundMenu()
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
    Widgets::CenterdText("Current version: " EDITOR_VERSION_NUMBER);
    Widgets::CenterdText("Latest version: " + Editor::m_LatestVersion);
    ImGui::Dummy(ImVec2(0,20));

    ImGui::TextWrapped("A newer version of Map Editor is available with,");
    ImGui::Text("1. New features\n2. Bug fixes\n3. Improvements");
    ImGui::Spacing();
    ImGui::TextWrapped("Click on the `Download page` button and follow the instructions there to update.");
}