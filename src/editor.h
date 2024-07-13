#pragma once
#include "pch.h"

/*
  Editor Manager Class
  The Super Class that handles & initiates everything
  Execution starts here.
*/
class EditorMgr {
  private:
    bool m_bOpened; // Is the editor currently open

    void ApplyStyle();
    void Cleanup();

  public:
    EditorMgr();
    EditorMgr(EditorMgr&);

    bool IsOpen();
    void Process();
};

extern EditorMgr Editor;