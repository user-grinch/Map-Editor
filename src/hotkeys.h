#pragma once

class Hotkey
{
private:
    static inline std::string m_CurrentHotkey;

    int m_key1;
    int m_key2;
    bool m_bPressed;

public:
    Hotkey(int key1, int key2)
        : m_key1(key1), m_key2(key2)
    {}

    // Draws ui to change the hotkeys from frontend
    bool DrawUI(const char* label);
    bool Pressed();
    std::string GetNameString();
};

extern Hotkey newObjKey;
extern Hotkey copyKey;
extern Hotkey pasteKey;
extern Hotkey deleteKey;
extern Hotkey snapKey;
extern Hotkey editorOpenKey;
extern Hotkey viewportSwitchKey;
extern Hotkey toggleUIKey;
extern Hotkey copyHoveredObjName;