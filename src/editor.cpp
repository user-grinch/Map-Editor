#include "pch.h"
#include "editor.h"
#include "viewport.h"
#include "interface.h"
#include "utils.h"
#include "objmanager.h"
#include "hotkeys.h"
#include <CHud.h>
#include <CMenuManager.h>

void Editor::Init()
{
	ObjManager::Init();
}

Editor::Editor()
{
	ApplyStyle();
	pCallbackFunc = std::bind(&DrawWindow);

	// Load config data
	Interface::m_bAutoSnapToGround = gConfig.GetValue("editor.autoSnap", true); 
	Interface::m_bShowInfoMenu = gConfig.GetValue("editor.showInfoMenu", true); 
	ObjManager::m_bDrawBoundingBox = gConfig.GetValue("editor.drawBoundingBox", true);
    ObjManager::m_bDrawAxisLines = gConfig.GetValue("editor.drawAxisLines", true);
	Viewport::m_bShowHoverMenu = gConfig.GetValue("editor.showHoverMenu", true);
	Interface::m_bWelcomeScreenDisplayed = gConfig.GetValue("editor.welcomeDisplayed", false); 

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

			if (!Editor::m_bShowEditor)
			{
				Viewport::m_eViewportMode = EDIT_MODE;
				Interface::m_bObjectBrowserShown = false;
				Interface::m_bOpenObjectBrowser = false;
				m_bShowMouse = false;
				gConfig.WriteToDisk();
				Viewport::Shutdown();
			}
		}
	};
};

void Editor::DrawWindow()
{
	static bool bTriedtoHideCursor;
	if (!FrontEndMenuManager.m_bMenuActive)
	{
		if (Editor::m_bShowEditor)
		{
			if (Viewport::m_eViewportMode == EDIT_MODE)
			{
				m_bShowMouse = true;
				bTriedtoHideCursor = false;
			}
    		Interface::m_bIsInputLocked = false;

			Interface::DrawMainMenuBar();
			Viewport::DrawOverlay();
			Viewport::ProcessSelectedObjectInputs();
			Interface::DrawPopupMenu();
			Interface::DrawInfoMenu();
			Viewport::DrawHoverMenu();
		}
	}
	else
	{
		
		if (!bTriedtoHideCursor)
		{
			m_bShowMouse = false;
			gConfig.WriteToDisk();
			bTriedtoHideCursor = true;
		}
	}
}

void Editor::ApplyStyle()
{
	ImGuiStyle* style = &ImGui::GetStyle();
	ImVec4* colors = style->Colors;

	style->WindowRounding = 1;
	style->ScrollbarRounding = 1;
	style->GrabRounding = 1;
	style->WindowRounding = 1;
	style->ChildRounding = 1;
	style->ScrollbarRounding = 1;
	style->GrabRounding = 1;
	style->FrameRounding = 0;
	style->TabRounding = 1.0;
	style->AntiAliasedLines = true;
	style->AntiAliasedFill = true;
	style->Alpha = 1;

	style->FrameBorderSize = 0;
	style->ChildBorderSize = 0;
	style->TabBorderSize = 0;
	style->WindowBorderSize = 0;
	style->PopupBorderSize = 0;

	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.0f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.35f, 0.58f, 0.86f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.41f, 0.68f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
}

void Editor::CheckForUpdate()
{
	const char* link = "https://api.github.com/repos/user-grinch/Map-Editor/tags";
	char* path = PLUGIN_PATH((char*)"MapEditor/json/versioninfo.json");
	HRESULT res = URLDownloadToFile(NULL, link, path, 0, NULL);

	if (res == E_OUTOFMEMORY || res == INET_E_DOWNLOAD_FAILURE)
	{
		CHud::SetHelpMessage("Failed to check for updates", false, false, false);
		return;
	}
	CJson verinfo = CJson("versioninfo");

	/* 
		fetch the version number
	*/
	if (verinfo.m_Data.empty())
	{
		m_LatestVersion = EDITOR_VERSION_NUMBER;
	}
	else
	{
		m_LatestVersion = verinfo.m_Data.items().begin().value()["name"].get<std::string>();
	}

	if (m_LatestVersion > EDITOR_VERSION_NUMBER)
	{
		Interface::m_bShowPopup = true;
		Interface::m_popupTitle = "Update found";
		Interface::m_pPopupFunc = Interface::UpdateFoundMenu;
		m_State = UPDATER_UPDATE_FOUND;
	}
	else
	{
		CHud::SetHelpMessage("No update found.", false, false, false);
		m_State = UPDATER_IDLE;
	}
}
