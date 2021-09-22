#pragma once
#include "plugin.h"
#define PLAYER_Z_OFFSET 20.0f
#define MOUSE_FACTOR_X 13.0f
#define MOUSE_FACTOR_Y 6.0f

enum VIEWPORT_STATE
{
    EDIT_MODE,
    VIEW_MODE,
    OBJECT_VIEW_MODE,
};

class Viewport
{
private:
    static inline bool m_bInitialized; 
    static inline CVector m_fTotalMouse; // stores the mouse position
    static inline BYTE m_bHudState; // backup for Shutdown()
    static inline BYTE m_bRadarState; // backup for Shutdown()
    static inline bool m_bNeverWanted; // backup for Shutdown()

    // To allow updating camera position externally
    static inline CVector m_fCameraPos;
    static inline bool m_bCameraPosWasUpdated;

    static void LoadNewClump(int model, RpClump *&pClump, RpAtomic *&pAtomic, RwFrame *&pFrame);
    static void RenderObjectBrowserModel();
    
public:
    static inline float m_nRenderScale = 1.0f; // ObjectBrowser object render scale
    static inline bool m_bShowContextMenu;
	static inline bool m_bShowHoverMenu; // tooltup menu showing info on hover
    static inline bool m_bBeingHovered;
    static inline VIEWPORT_STATE m_eViewportMode = EDIT_MODE; 
    static inline int m_nMul = 1; // movement speed multiplier for viewport realted controls
    static inline float m_fFOV = 70.0f; // camera field of view
    static inline CVector m_vecWorldPos;
    static inline CEntity *m_HoveredEntity;
    static inline CVector m_vecRenderRot;
    static inline bool m_bObjBrowserAutoRot;
    static inline ImVec2 m_fViewportSize;
    struct COPY_MODEL
    {
        static inline int m_nModel;
        static inline CVector m_vecRot;
    };
    
    Viewport() = delete;
    Viewport(Viewport&) = delete;

    static void Init();
    static void DrawHoverMenu();
    /*
    * Creates a transparent overlay over the viewport
    * Used for inputs and context menu creation
    */
    static void DrawOverlay();
    static void Process();
    static void ProcessContextMenu();
    static void ProcessSelectedObjectInputs();
    static void SetCameraPosn(const CVector &pos);
    static void Shutdown();
};