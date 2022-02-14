#pragma once

/*
    Font Manager class
    Handles loading, fetching, freeing & reloading fonts
*/
class FontMgr
{
private:
    struct FontInfo
    {
        ImFont *m_pFont;
        float m_fMul;
        std::string m_path;
    };
    static inline std::vector<FontInfo> m_vecFonts;
    static inline bool m_bMulChangedExternal;
    
public:
    FontMgr() = delete;
    FontMgr(FontMgr&) = delete;

    static ImFont* GetFont(const char* fontName);
    static ImFont* LoadFont(const char* fontName, float fontMul = 1.0f);
    static void SetMultiplier(float fontMul);

    // ImGui::GetIO().Default font must be loaded after unloading all fonts
    static void UnloadFonts();
    static void ReloadFonts();
    static bool IsReloadNeeded();
};


