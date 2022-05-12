#pragma once
#include "pch.h"

class Interface
{
private:
    static inline ResourceStore m_locData{ "locations", eResourceType::TYPE_TEXT };

    // Draws the top black bar contains menu options
    static void DrawMainMenuBar();

    // Draws the object specific right side pnael
    static void DrawSidepanel();

    // Draws the popups at the center of the screen
    static void DrawPopupMenu();

public:
    static inline bool m_bAutoSave = true;
    static inline bool m_bAutoTpToLoc;
    static inline bool m_bAutoSnapToGround;
    static inline bool m_bDrawAxisLines;
    static inline bool m_bDrawBoundingBox; // bouding box around selected objects
    static inline bool m_bInputLocked; 
    static inline bool m_bShowFPS;
    static inline bool m_bShowGUI = true;
    static inline bool m_bShowHoverMenu; // object hoverer tooltip ( shows model id & model name )
    static inline bool m_bShowSidepanel;
    static inline bool m_bWelcomeShown;
    static inline bool m_bRandomRot; // places objects randomly
    static inline float m_RandomRotX[2], m_RandomRotY[2], m_RandomRotZ[2]; // min max rotations

    static inline ContextMenu m_ContextMenu; // right click context menu
    static inline PopupMenu m_PopupMenu; // center screen popup menus
    static inline ResourceStore m_favData{ "favourites", eResourceType::TYPE_TEXT };
    

    Interface() = delete;
    Interface(Interface&) = delete;
    
    // Initiazes stuff for MapEditor open
    static void Init();

    // Needs to be called each frame
    static void Process();

    // Cleans stuff on MapEditor exit
    static void Cleanup();

    // process right click context menu
    static void DrawContextMenu();
};

