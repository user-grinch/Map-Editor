#pragma once
#pragma warning(disable:4503 4244 4005)
#include "defines.h"

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

#include "utils/hotkeys.h"
#include "utils/resourcestore.h"
#include "utils/datastore.h"
#include "utils/updater.h"
#include "utils/fontmgr.h"
#include "utils/d3dhook.h"
#include "utils/log.h"

#include "imgui/imgui.h"

using namespace plugin;
enum class eRenderer {
    DirectX9,
    DirectX11,
    Unknown
};

using str = std::string;
using fArgNone_t = std::function<void()>;
using fArg1_t = std::function<void(str&)>;
using fArg3_t = std::function<void(str&, str&, str&)>;
using fRtnArg1_t = std::function<std::string(str&)>;
using fRtnBoolArg1_t = std::function<bool(str&)>;

#define fArg3Wrapper(x) [](str& a, str& b, str& c){x(a, b, c);}
#define fArgWrapper(x) [](str& a){x(a);}
#define fRtnArgWrapper(x) [](str& a){return x(a);}
#define fArgNoneWrapper(x) [](){x();}

struct ContextMenu {
    bool m_bShow;
    fArg3_t m_pFunc;
    std::string m_Root, m_Key, m_Val;
};

struct PopupMenu {
    bool m_bShow;
    std::string m_Title;
    std::function<void()> m_pFunc;
};

static eRenderer gRenderer = eRenderer::Unknown;
extern DataStore gConfig;

extern Hotkey newObjKey;
extern Hotkey copyKey;
extern Hotkey pasteKey;
extern Hotkey deleteKey;
extern Hotkey snapKey;
extern Hotkey editorOpenKey;
extern Hotkey viewportSwitchKey;
extern Hotkey toggleUIKey;
extern Hotkey copyHoveredObjName;
