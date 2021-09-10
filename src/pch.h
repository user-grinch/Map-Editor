#pragma once
#pragma warning(disable:4503 4244 4005)

#define DISCORD_INVITE "https://discord.gg/ZzW7kmf"
#define GITHUB_LINK "https://github.com/user-grinch/Map-Editor"
#define HRESULT int
#define RAD_TO_DEG(x) x*-57.2958f

#include <d3d9.h>
#include <sstream>
#include <vector>
#include <windows.h>
#include <filesystem>
#include <functional>

#include <plugin.h>
#include <CCamera.h>
#include <CWorld.h>
#include <extensions/ScriptCommands.h>
#include <extensions/scripting/ScriptCommandNames.h>
#include <CTimer.h>

#include "vkeys.h"
#include "versioninfo.h"
#include "resourcestore.h"

#include "../depend/imgui/imgui.h"
#include "../depend/imgui/imgui_impl_dx9.h"
#include "../depend/imgui/imgui_impl_win32.h"

using namespace plugin;

extern std::ofstream flog;
extern CJson config;
