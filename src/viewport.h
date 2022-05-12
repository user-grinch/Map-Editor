#pragma once
#include "plugin.h"

enum class eViewportState
{
    Edit,
    View,
    Browser,
};

class Viewport
{
private:
    static inline bool m_bInitialized; // Is the viewport init done
    static inline ImVec2 m_fSize; // Size of the viewport
    static inline CVector m_fMousePos; // Mouse position in the viewport

    // To allow updating camera position externally
    static inline CVector m_fCamPos;
    static inline bool m_bCamUpdated;

    // Processes the right click context menu
    static void DrawContextMenu();

    // The object hover menu. Shows object model, name & type
    static void DrawHoverMenu();

    // Creates a transparent layer over the viewport
    // Used for inputs and context menu creation
    static void DrawOverlay();

    // Processes input for the currently selected object
    // TODO: Should be in ObjManager class?
    static void ProcessInputs();

public:
    static inline eViewportState m_eState = eViewportState::Edit;
    static inline bool m_bHovered; // Is mouse hovering viewport
    static inline CEntity *m_HoveredEntity; // Currently hovered entity
    static inline int m_nMoveSpeed = 1; // movement speed multiplier controls
    static inline float m_fFOV = 70.0f;
    static inline CVector m_fWorldPos; // cursor world position

    struct Browser
    {
    private:
        static inline size_t m_nSelectedID = NULL; // Selected model id
        static inline CVector m_fRot; // rotation of the rendering object

        // Loads model
        // TODO: Need some fail checks
        static void LoadModel(size_t model, RpClump *&pClump, RpAtomic *&pAtomic, RwFrame *&pFrame);
        
        // Renders the model in viewport
        static void RenderModel();
    public:
        static inline bool m_bShowNextFrame; // Should the browser be shown next frame
        static inline bool m_bShown; // Is the browser being shown
        static inline bool m_bAutoRot; // Auto rotate object in browser

        static inline float m_fScale = 1.0f; // ObjectBrowser object render scale

        static size_t GetSelected();
        static void Process();
        static void SetSelected(int modelId);
    };

    Viewport() = delete;
    Viewport(Viewport&) = delete;


    // Initiazes stuff for MapEditor open
    static void Init();

    // Needs to be called each frame
    static void Process();

    // Cleans stuff on MapEditor exit
    static void Cleanup();

    // Returns the size of the viewport
    static ImVec2 GetSize();

    // Allows changing camera position externally
    static void SetCameraPosn(const CVector &pos);
};