#pragma once
#include <string>
#include <functional>

class ContextMenus {
    public:
        bool m_bShow;
        std::function<void(std::string&, std::string&, std::string&)> m_pFunc;
        std::string m_Root, m_Key, m_Val;
        
        void Draw();
};

void ContextMenu_Copy();
void ContextMenu_Delete();
void ContextMenu_NewObject();
void ContextMenu_Paste();
void ContextMenu_SnapToGround();
void ContextMenu_Search(std::string& root, std::string& key, std::string& value);
void ContextMenu_Viewport(std::string& root, std::string& key, std::string& value);

extern ContextMenus ContextMenu;