#pragma once
#include <string>
#include <functional>

class Popups {
    public:
        bool m_bShow;
        std::string m_Title;
        std::function<void()> m_pFunc;

        void Draw();
};

extern Popups Popup;

void Popup_AboutEditor();
void Popup_Controls();
void Popup_Export();
void Popup_Import();
void Popup_UpdateFound();
void Popup_Welcome();