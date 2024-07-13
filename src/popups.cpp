#include "pch.h"
#include "utils/widget.h"
#include "filemgr.h"
#include "utils/utils.h"
#include "objectmgr.h"
#include "interface.h"
#include "viewport.h"
#include "popups.h"

Popups Popup;

void Popup_Import() {
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
            Popup.m_bShow = false;
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

void Popup_Export() {
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
            auto temp = std::move(Popup);
            Popup.m_Title = "Replace Confirmation Dialog";
            Popup.m_pFunc = [temp](){
                ImGui::Spacing();
                ImGui::TextWrapped("Another file with the same name already exists. Would you like to replace it?");
                ImGui::Dummy(ImVec2(0, 20));
                if (ImGui::Button("Yes", Widget::CalcSize(2))) {
                    FileMgr::ExportIPL(buffer);
                    Popup.m_bShow = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("No", Widget::CalcSize(2))) {
                    Popup = std::move(temp);
                }
            };
        } else {
            FileMgr::ExportIPL(buffer);
            Popup.m_bShow = false;
        }
        
    }
}

void Popup_UpdateFound() {
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

void Popup_Controls();

void Popup_AboutEditor() {
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
        Popup.m_Title = "Controls";
        Popup.m_pFunc = Popup_Controls;
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
    Widget::TextCentered("Copyright Grinch_ 2021-2024");
    Widget::TextCentered("All rights reserved");
}

void Popup_Controls() {
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

void Popup_Welcome() {
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
        Popup.m_Title = "Controls";
        Popup.m_pFunc = Popup_Controls;
    }
    ImGui::SameLine();
    if (ImGui::Button("About page", Utils::GetSize(2))) {
        Popup.m_Title = "About";
        Popup.m_pFunc = Popup_AboutEditor;
    }
    
    ImGui::Dummy(ImVec2(0, 20));
    if (ImGui::BeginChild("WelcomeScreen")) {
        Widget::TextCentered("Welcome to Map Editor");
        Widget::TextCentered(std::string("v") + EDITOR_VERSION);
        Widget::TextCentered(std::string("(") + __DATE__ + ")");
        ImGui::Dummy(ImVec2(0, 10));

        ImGui::TextWrapped("Click on the `Controls` button to get started. If you have suggestions let me know on Discord.");
        ImGui::Dummy(ImVec2(0, 10));
        Widget::TextCentered("Consider supporting on Patreon!");
        ImGui::Spacing();
        Widget::TextCentered("Copyright Grinch_ 2021-2024");
        Widget::TextCentered("All rights reserved");
        ImGui::EndChild();
    }
}

void Popups::Draw() {
    if (Updater::IsUpdateAvailable()) {
        Popup.m_bShow = true;
        Popup.m_Title = "Update available!";
        Popup.m_pFunc = Popup_UpdateFound;
    }

    if (!Popup.m_bShow) {
        return;
    }

    ImGuiWindowFlags flags =  ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;

    static ImVec2 prevSize;
    ImGui::SetNextWindowSizeConstraints(ImVec2(screen::GetScreenWidth()/4, screen::GetScreenHeight()/1.95), // manually tested
                                        ImVec2(screen::GetScreenWidth()/4, screen::GetScreenHeight()/1.95));

    ImVec2 size = Viewport.GetSize();
    ImGui::SetNextWindowPos(ImVec2((size.x - prevSize.x)/2, (size.y - prevSize.y)/2), ImGuiCond_Always);
    if (ImGui::Begin(Popup.m_Title.c_str(), &Popup.m_bShow, flags)) {
        if (Popup.m_pFunc) {
            Popup.m_pFunc();
        }
        prevSize = ImGui::GetWindowSize();
        ImGui::End();
    }

    // Reset state on window close
    if (Updater::IsUpdateAvailable() && !Popup.m_pFunc) {
        Updater::ResetUpdaterState();
    }
}