#pragma once
#include "imgui/imgui.h"

/*
    Widgets Class
    Contains useful ui utilities
*/
class Widget {
  private:
    using VecStr = const std::vector<std::string>;

  public:
    Widget() = delete;
    Widget(Widget&) = delete;

    // Calculates button size based on window width & spacing flags
    static ImVec2 CalcSize(short count = 1, bool spacing = true);

    // Calculates button size based on frame size
    static ImVec2 CalcSizeFrame(const char* text);

    // Draws DataStore data in the interface
    static void DataList(ResourceStore& data, fArg3_t clickFunc = nullptr, fArgNone_t addFunc = nullptr,
                         fArgNone_t contextOptionsFunc = nullptr, fArgNone_t tabsFunc = nullptr);

    // ImGui::TextFilter with hint support
    static bool Filter(const char* label, ImGuiTextFilter& filter, const char* hint);

    // Draws a dropdown listbox
    static bool ListBox(const char* label, VecStr& allItems, int& selected);
    static bool ListBox(const char* label, VecStr& allItems, std::string& selected);

    // Text aligned to the center of the window
    static void TextCentered(const std::string& text);

    // Displays a popup with helpful text
    static void Tooltip(const char* text);
};