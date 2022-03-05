#include "pch.h"
#include "editor.h"
#include "objmanager.h"

void EditorThread(void* param)
{
    ObjManager::Init();
    Updater::CheckUpdate();
     /* 
        Wait for game init
        Doing it like this doesn't prevent you to attach debugger
    */ 
    static bool gameStarted = false;
    Events::processScriptsEvent +=[]{
        gameStarted = true;
    };

    while (!gameStarted)
    {
        Sleep(500);
    }
    // -------------------------------------------------------------

    gLog << "Starting...\nVersion: "  EDITOR_VERSION  "\nAuthor: Grinch_\nDiscord: " DISCORD_INVITE "\nMore Info: "
         GITHUB_LINK "\n" << std::endl;

    /*
    	Need SilentPatch since all gta games have issues with mouse input
    	Implementing mouse fix is a headache anyway
    */
    if (!GetModuleHandle("SilentPatchSA.asi"))
    {
        gLog << "Error: SilentPatch not found. Please install it from here https://gtaforums.com/topic/669045-silentpatch/" << std::endl;
        int msgID = MessageBox(RsGlobal.ps->window, "SilentPatch not found. Do you want to install Silent Patch? (Game restart required)", "CheatMenu", MB_OKCANCEL | MB_DEFBUTTON1);

        if (msgID == IDOK)
        {
            ShellExecute(nullptr, "open", "https://gtaforums.com/topic/669045-silentpatch/", nullptr, nullptr, SW_SHOWNORMAL);
        };
        return;
    }

    Editor::Init();
    while (true)
    {
        Updater::Process();
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