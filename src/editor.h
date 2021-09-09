#pragma once 
#include "pch.h"
#include "hook.h"

enum UPDATER_STATE
{
	UPDATER_IDLE,
	UPDATER_CHECKING,
	UPDATER_UPDATE_FOUND
};

class Editor : Hook
{
private:
	static void ApplyStyle();
	
public:
	static inline bool m_bShowEditor;
	static inline UPDATER_STATE m_State = UPDATER_CHECKING;
	static inline std::string m_LatestVersion;
	
	Editor();
	static void Init();
	static void CheckForUpdate();
	static void DrawWindow();
};