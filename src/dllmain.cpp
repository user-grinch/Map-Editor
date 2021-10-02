#include "pch.h"
#include "editor.h"

void EditorThread(void* param)
{
	Editor::Init();

	// -------------------------------------------------------------
	// wait for the game to initialize
	static bool bGameInit = false;
	Events::initGameEvent += []
	{
		bGameInit = true;
	};
	
	while (!bGameInit)
	{
		Sleep(1000);
	}
	// -------------------------------------------------------------

	gLog << "Starting...\nVersion: "  EDITOR_VERSION  "\nAuthor: Grinch_\nDiscord: " DISCORD_INVITE "\nMore Info: "
		GITHUB_LINK "\n" << std::endl;

	Editor menu;
	while (true)
	{
		if (Editor::m_State == UPDATER_CHECKING)
		{
			Editor::CheckForUpdate();
		}

		// wait infinitely here
		Sleep(5000);
	}
}

BOOL WINAPI DllMain(HINSTANCE hDllHandle, DWORD nReason, LPVOID Reserved)
{
	if (nReason == DLL_PROCESS_ATTACH)
	{
		uint gameVersion = GetGameVersion();
		if (gameVersion == GAME_10US_HOODLUM || gameVersion == GAME_10US_COMPACT)
		{
			CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)&EditorThread, nullptr, NULL, nullptr);
		}
		else
		{
			MessageBox(HWND_DESKTOP, "Unknown game version. GTA SA v1.0 US is required.", "CheatMenu", MB_ICONERROR);
		}
	}

	return TRUE;
}