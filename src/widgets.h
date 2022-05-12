#pragma once
#include <vector>
#include <string>
#include "pch.h"

class Widgets
{
public:
    Widgets() = delete;;
    Widgets(Widgets&) = delete;

    static void CenterdText(const std::string& text);
    static bool ListBoxStr(const char* label, std::vector<std::string>& all_items, std::string& selected);
    static void DrawJSON(ResourceStore& data, ContextMenu& context, f_Arg3 func_left_click, f_Arg3 func_right_click);
    static void ShowTooltip(const char* text);
};