#include "pch.h"
#include "updater.h"

bool Updater::IsUpdateAvailable() {
    return Updater::curState == States::FOUND;
}

void Updater::ResetUpdaterState() {
    Updater::curState = States::IDLE;
}

std::string Updater::GetUpdateVersion() {
    return Updater::latestVer;
}

void Updater::CheckUpdate() {
    if (Updater::curState == States::IDLE) {
        Updater::curState = States::CHECKING;
        CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)&Updater::Process, nullptr, NULL, nullptr);
    }
}

void Updater::Process() {
    if (Updater::curState != States::CHECKING) {
        return;
    }

    const char* link = "https://api.github.com/repos/user-grinch/Map-Editor/tags";
    char* path = PLUGIN_PATH((char*)FILE_NAME "/data/versioninfo.json");
    HRESULT res = URLDownloadToFile(NULL, link, path, 0, NULL);

    if (res == E_OUTOFMEMORY || res == INET_E_DOWNLOAD_FAILURE) {
        CHud::SetHelpMessage("Failed connecting to GitHub server", false, false, false);
        return;
    }

    // Extract the version number
    FILE *pFile= fopen(path, "r");
    if (pFile != NULL) {
        char buf[64];
        float version = 0.0f;
        while (fgets(buf, 64, pFile) != NULL) {
            sscanf(buf, "[{\"name\": \"%f\",", &version);
            if (version != 0.0f) {
                latestVer = std::format("{}", version);
                break;
            }
        }
        fclose(pFile);
    }
    remove(path);

    if (latestVer > EDITOR_VERSION_NUMBER) {
        CHud::SetHelpMessage("A version of MapEditorSA is available", false, false, false);
        curState = States::FOUND;
    } else {
        CHud::SetHelpMessage("No update found", false, false, false);
        Updater::curState = States::IDLE;
    }
}
