#include "pch.h"
#include "editor.h"
#include "objmanager.h"

static void EditorThread(void* param) {
    ObjManager::Init();

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

        if (Updater::IsUpdateAvailable()) {
            Log::Print<eLogLevel::Info>("New update available: %s", Updater::GetUpdateVersion().c_str());
        }

        if (!std::filesystem::exists(PLUGIN_PATH((char*)FILE_NAME))) {
            Log::Print<eLogLevel::Error>("Failed to find MapEditor directory!");
            return;
        }
    };

    Editor::Init();
}

BOOL WINAPI DllMain(HINSTANCE hDllHandle, DWORD nReason, LPVOID Reserved) {
    if (nReason == DLL_PROCESS_ATTACH) {
        uint gameVersion = GetGameVersion();
        if (gameVersion == GAME_10US_HOODLUM || gameVersion == GAME_10US_COMPACT) {
            CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)&EditorThread, nullptr, NULL, nullptr);
        } else {
            MessageBox(HWND_DESKTOP, "Unknown game version. GTA SA v1.0 US is required.", "CheatMenu", MB_ICONERROR);
        }
    }

    return TRUE;
}