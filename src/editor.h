#pragma once
#include "pch.h"

/*
  Editor Manager Class
  The Super Class that handles & initiates everything
  Execution starts here.
*/
class EditorMgr {
  private:
    void ApplyStyle();
    bool m_bOpened; // Is the editor currently open

    // Does basic cleaup to close the MapEditor
    void Cleanup();

  public:
    EditorMgr();
    EditorMgr(EditorMgr&);

    // Returns true if the MapEditor is open
    bool IsOpen();

    /* 
      Processes the MapEditor code
      Handles drawing & controls
    */
    void Process();
};

extern EditorMgr Editor;