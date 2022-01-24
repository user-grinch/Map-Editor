#include "pch.h"
#include "updater.h"

bool Updater::IsUpdateAvailable()
{
    return Updater::curState == States::FOUND;
}

void Updater::ResetUpdaterState()
{
    Updater::curState = States::IDLE;
}

std::string Updater::GetUpdateVersion()
{
    return Updater::latestVer;
}

void Updater::CheckUpdate()
{
    if (Updater::curState == States::IDLE)
    {
        Updater::curState = States::CHECKING;
    }
}

void Updater::Process()
{
    if (Updater::curState != States::CHECKING)
    {
        return;
    }

    const char* link = "https://api.github.com/repos/user-grinch/Map-Editor/tags";
    char* path = PLUGIN_PATH((char*)"MapEditor/json/versioninfo.json");
    HRESULT res = URLDownloadToFile(NULL, link, path, 0, NULL);

    if (res == E_OUTOFMEMORY || res == INET_E_DOWNLOAD_FAILURE)
    {
        CHud::SetHelpMessage("Failed to check for updates", false, false, false);
        return;
    }

    CJson verinfo = CJson("versioninfo");

    // fetch the version number
    if (verinfo.m_Data.empty())
    {
        latestVer = EDITOR_VERSION_NUMBER;
    }
    else
    {
        latestVer = verinfo.m_Data.items().begin().value()["name"].get<std::string>();
    }

    if (latestVer > EDITOR_VERSION_NUMBER)
    {
        CHud::SetHelpMessage("Update found", false, false, false);
        curState = States::FOUND;
    }
    else
    {
        CHud::SetHelpMessage("No update found.", false, false, false);
        Updater::curState = States::IDLE;
    }
}
