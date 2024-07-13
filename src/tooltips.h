#pragma once
#include <string>
#include <functional>

class Tooltips {
    public:
        bool m_bShow;
        std::string m_Title;
        std::function<void()> m_pFunc;

        void Draw();
};

extern Tooltips Tooltip;

void Tooltip_Browser();