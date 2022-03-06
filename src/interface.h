#pragma once
#include "pch.h"

#define DEFAULT_MODEL_ID 620

class Interface
{
private:
    static inline ResourceStore m_locData{ "locations", eResourceType::TYPE_TEXT };
    struct ContextMenuData
    {
        std::function<void(std::string&, std::string&, std::string&)> function;
        std::string key;
        std::string rootKey;
        std::string value;
    };
    static inline bool logImports;

public:
    static inline bool m_bAutoSave = true;
    static inline bool m_bAutoTpToLoc = false;
    static inline bool m_bAutoSnapToGround;
    static inline bool m_bWelcomeScreenDisplayed;
    static inline bool m_bShowInfoMenu; // right hand menu
    static inline ContextMenuData m_contextMenu;
    static inline ResourceStore m_favData{ "favourites", eResourceType::TYPE_TEXT };
    static inline bool m_bOpenObjectBrowser;
    static inline bool m_bObjectBrowserShown;
    static inline int m_nBrowserSelectedModelId = 0;
    static inline bool m_bShowPopup;
    static inline std::string m_popupTitle;
    static inline std::function<void()> m_pPopupFunc;
    static inline bool m_bIsInputLocked; // Input locked by some imgui widgets

    Interface() = delete;
    Interface(Interface&) = delete;

    static void DrawMainMenuBar();
    static void DrawInfoMenu();
    static void DrawPopupMenu();
    static void ImportMenu();
    static void ExportMenu();
    static void SettingsMenu();

    // Custom popup menu codes
    static void QuickObjectCreateMenu();
    static void AboutEditorMenu();
    static void WelcomeMenu();
    static void EditorControls();
    static void UpdateFoundMenu();

    static void ProcessContextMenu();
    static void SearchContextMenu(std::string& root, std::string& key, std::string& value);
};

