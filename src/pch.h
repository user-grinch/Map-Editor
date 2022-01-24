#pragma once
#pragma warning(disable:4503 4244 4005)

#define DISCORD_INVITE "https://discord.gg/ZzW7kmf"
#define GITHUB_LINK "https://github.com/user-grinch/Map-Editor"
#define HRESULT int
#define RAD_TO_DEG(x) x*-57.2958f
#define IMGUI_DEFINE_MATH_OPERATORS

#ifdef GTASA
#define BY_GAME(sa, vc, iii) sa
#elif GTAVC
#define BY_GAME(sa, vc, iii) vc
#elif GTA3
#define BY_GAME(sa, vc, iii) iii
#endif

#include <d3d9.h>
#include <d3d11.h>
#include <d3d11Shader.h>
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
#include <CHud.h>

#include "vkeys.h"
#include "version.h"
#include "resourcestore.h"
#include "updater.h"
#include "fontmgr.h"
#include "d3dhook.h"

#include "../depend/imgui/imgui.h"

using namespace plugin;

enum eRenderer
{
    Render_DirectX9,
    Render_DirectX11,
    Render_Unknown
};

static eRenderer gRenderer = Render_Unknown;
extern std::ofstream gLog;
extern CJson gConfig;
