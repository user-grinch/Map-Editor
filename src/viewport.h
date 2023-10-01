#pragma once
#include "plugin.h"

enum class eViewportState {
  Edit,
  View,
  Browser,
};

/*
* Object Browser Renderer
* Renders object clumps in the browser menu
*/
struct BrowserMgr {
  private:
    size_t m_nSelectedID = NULL; // Selected model id
    CVector m_fRot; // rotation of the rendering object

    // Loads model
    // TODO: Need some fail checks
    void LoadModel(size_t model, RpClump *&pClump, RpAtomic *&pAtomic, RwFrame *&pFrame);

    // Renders the model in viewport
    void RenderModel();
  public:
    bool m_bShowNextFrame; // Should the browser be shown next frame
    bool m_bShown; // Is the browser being shown
    bool m_bAutoRot; // Auto rotate object in browser

    float m_fScale = 1.0f; // ObjectBrowser object render scale

    size_t GetSelected();
    void Process();
    void SetSelected(int modelId);
};

/*
* Viewport Class
* Handles Object Rendering & Drawing
*/
class ViewportMgr {
  private:
    bool m_bInitialized; // Is the viewport init done
    ImVec2 m_fSize; // Size of the viewport
    CVector m_fMousePos; // Mouse position in the viewport

    // To allow updating camera position externally
    CVector m_fCamPos;
    bool m_bCamUpdated;

    // Processes the right click context menu
    void DrawContextMenu();

    // The object hover menu. Shows object model, name & type
    void DrawHoverMenu();

    // Creates a transparent layer over the viewport
    // Used for inputs and context menu creation
    void DrawOverlay();

    // Processes input for the currently selected object
    // TODO: Should be in ObjManager class?
    void ProcessInputs();

  public:
    eViewportState m_eState = eViewportState::Edit;
    bool m_bHovered; // Is mouse hovering viewport
    CEntity *m_HoveredEntity; // Currently hovered entity
    float m_nMoveSpeed = 0.25f; // movement speed multiplier
    float m_fFOV = 70.0f;
    CVector m_fWorldPos; // cursor world position
    BrowserMgr Browser;

    ViewportMgr();

    // Initiazes stuff for MapEditor open
    void Init();

    // Needs to be called each frame
    void Process();

    // Cleans stuff on MapEditor exit
    void Cleanup();

    // Returns the size of the viewport
    ImVec2 GetSize();

    // Allows changing camera position externally
    void SetCameraPosn(const CVector &pos);
};

extern ViewportMgr Viewport;