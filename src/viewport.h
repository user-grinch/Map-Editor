#pragma once
#include <plugin.h>
#include "renderer.h"

enum class eViewportState {
  Edit,
  View,
};

/*
* Viewport Class
* Handles Object Rendering & Drawing
*/
class ViewportMgr {
  private:
    bool m_bInitialized;  
    ImVec2 m_fViewportSize;       
    CVector m_fMousePos;  
    CVector m_fCamPos;
    bool m_bCamUpdateRequired; // Allows updating camera externally

    // The object hover menu. Shows object model, name & type
    void DrawHoverMenu();

    // Creates a transparent layer over the viewport
    // Used for inputs and context menu creation
    void DrawOverlay();

    // Highlights the currently selected object in red
    void HighlightSelection(CEntity *pEntity); 

    // Processes input for the currently selected object
    // TODO: Should be in ObjManager class?
    void ProcessInputs();

  public:
    eViewportState m_eState = eViewportState::Edit;
    bool m_bHovered; 
    CEntity *m_HoveredEntity; 
    CVector m_fCursorWorldPos; 
    float m_nMoveSpeedMul = 0.25f; 
    float m_fFOV = 70.0f;
    ModelRenderer m_Renderer;

    ViewportMgr();

    void Init();
    void Process();
    void Cleanup();
    ImVec2 GetSize();
    void SetCameraPosn(const CVector &pos);
};

extern ViewportMgr Viewport;