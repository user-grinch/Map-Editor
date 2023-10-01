#pragma once
#define EDITOR_NAME "Map Editor"
#define EDITOR_VERSION_NUMBER "0.10"
#define EDITOR_VERSION EDITOR_VERSION_NUMBER"-alpha"
#define EDITOR_TITLE EDITOR_NAME " v" EDITOR_VERSION

#define DISCORD_INVITE "https://discord.gg/ZzW7kmf"
#define GITHUB_LINK "https://github.com/user-grinch/Map-Editor"
#define PATREON_LINK "https://www.patreon.com/grinch_"

#ifdef GTASA
#define BY_GAME(sa, vc, iii) sa
#elif GTAVC
#define BY_GAME(sa, vc, iii) vc
#elif GTA3
#define BY_GAME(sa, vc, iii) iii
#endif

#define FILE_NAME "MapEditorSA"
#define MENU_DATA_PATH(x) (PLUGIN_PATH((char*)FILE_NAME##"/"##x))
#define MENU_DATA_EXISTS(x) (std::filesystem::exists(MENU_DATA_PATH(x)))
#define OPEN_LINK(x) ShellExecute(nullptr, "open", x, nullptr, nullptr, SW_SHOWNORMAL)

#define RAD_TO_DEG(x) x*-57.2958f
#define IMGUI_DEFINE_MATH_OPERATORS

#define MENU_WIDTH_FACTOR_X 2.5f
#define MENU_HEIGHT_FACTOR_X 1.0f