#pragma once
#include "pch.h"

class Editor {
  private:
    static void ApplyStyle();
    static inline bool m_bOpened; // Is the editor being shown

  public:

    Editor() = delete;
    Editor(Editor&) = delete;

    static void Init();
    static bool IsOpen();
    static void Draw();
    static void Cleanup();
};