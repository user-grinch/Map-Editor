#pragma once
#include "plugin.h"

enum class eRwModelType {
  Unknown,
  RpAtomic,
  RpClump,
};

/*
* Model Renderer
* Renders object in the browser section
*/
class ModelRenderer {
  private:
    size_t m_nSelectedModel = NULL;   // Model selected for render
    size_t m_nRenderModel = NULL;     // Model that's being rendered currently
    RpClump *m_pRpClump = nullptr;
    RwFrame *m_pRwFrame = nullptr;
    RpAtomic *m_pRpAtomic = nullptr;
    CVector m_fRot; 
    RwV3d m_fPos, m_fSize, m_fAxisX, m_fAxisY;
    
    // Helper functions
    bool LoadModel();
    void RenderModel();
    void UnloadModel();

  public:
    bool m_bShowNextFrame;  // Should the browser be shown next frame
    bool m_bShown;          // Is the browser being shown
    bool m_bAutoRot = true; // Auto rotate object in browser
    float m_fScale = 1.0f;  // ObjectBrowser object render scale

    void Init();
    void Process();
    size_t GetSelected();
    void SetSelected(int modelId);
};