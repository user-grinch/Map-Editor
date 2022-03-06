#pragma once
#include "pch.h"

class Editor
{
private:
    static void ApplyStyle();

public:
    static inline bool m_bShowEditor;
    static inline bool m_bShowGUI = true;

    Editor() = delete;
    Editor(Editor&) = delete;

    static void Init();
    static void CheckForUpdate();
    static void DrawWindow();
    static void Cleanup();
};