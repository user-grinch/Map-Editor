#pragma once
#include "pch.h"

class InterfaceMgr {
  private:
    ResourceStore m_locData{ "locations", eResourceType::TYPE_TEXT };

    // Draws the top black bar contains menu options
    void DrawMainMenuBar();

    // Draws the object specific right side pnael
    void DrawSidepanel();

    // Draws the popups at the center of the screen
    void DrawPopupMenu();

  public:
    bool m_bAutoSave = true;
    bool m_bAutoTpToLoc;
    bool m_bAutoSnapToGround;
    bool m_bDrawAxisLines;
    bool m_bDrawBoundingBox; // bouding box around selected objects
    bool m_bInputLocked;
    bool m_bShowFPS;
    bool m_bShowGUI = true;
    bool m_bShowHoverMenu; // object hoverer tooltip ( shows model id & model name )
    bool m_bShowSidepanel;
    bool m_bWelcomeShown;
    bool m_bRandomRot; // places objects randomly
    float m_RandomRotX[2], m_RandomRotY[2], m_RandomRotZ[2]; // min max rotations

    ContextMenu m_ContextMenu; // right click context menu
    PopupMenu m_PopupMenu; // center screen popup menus
    ResourceStore m_favData{ "favourites", eResourceType::TYPE_TEXT };


    InterfaceMgr();
    InterfaceMgr(InterfaceMgr&);

    // Needs to be called each frame
    void Process();

    // Cleans stuff on MapEditor exit
    void Cleanup();

    // process right click context menu
    void DrawContextMenu();
};

extern InterfaceMgr Interface;

