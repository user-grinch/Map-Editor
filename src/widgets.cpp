#include "pch.h"
#include "widgets.h"
#include "interface.h"

bool Widgets::ListBoxStr(const char* label, std::vector<std::string>& all_items, std::string& selected)
{
    bool rtn = false;
    if (ImGui::BeginCombo(label, selected.c_str()))
    {
        for (std::string current_item : all_items)
        {
            if (ImGui::MenuItem(current_item.c_str()))
            {
                selected = current_item;
                rtn = true;
            }
        }
        ImGui::EndCombo();
    }

    return rtn;
}

void Widgets::DrawJSON(ResourceStore& data,
                       std::function<void(std::string&, std::string&, std::string&)> func_left_click,
                       std::function<void(std::string&, std::string&, std::string&)> func_right_click)
{
    ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() / 2 - 5);
    ListBoxStr("##Categories", data.m_Categories, data.m_Selected);
    ImGui::SameLine();

    data.m_Filter.Draw("##Filter");
    if (ImGui::IsItemActive())
    {
        Interface::m_bIsInputLocked = true;
    }
    if (strlen(data.m_Filter.InputBuf) == 0)
    {
        ImDrawList* drawlist = ImGui::GetWindowDrawList();

        ImVec2 min = ImGui::GetItemRectMin();
        min.x += ImGui::GetStyle().FramePadding.x;
        min.y += ImGui::GetStyle().FramePadding.y;

        drawlist->AddText(min, ImGui::GetColorU32(ImGuiCol_TextDisabled), "Search");
    }

    ImGui::PopItemWidth();
    ImGui::Spacing();

    ImGui::BeginChild(1);
    for (auto root : data.m_pJson->m_Data.items())
    {
        if (root.key() == data.m_Selected || data.m_Selected == "All")
        {
            for (auto _data : root.value().items())
            {
                std::string name = _data.key();
                if (data.m_Filter.PassFilter(name.c_str()))
                {
                    if (ImGui::MenuItem(name.c_str()) && func_left_click != nullptr)
                    {
                        std::string root_key = root.key();
                        std::string data_key = _data.key();
                        std::string data_val = _data.value();

                        func_left_click(root_key, data_key, data_val);
                    }

                    if (ImGui::IsItemClicked(1) && func_right_click != nullptr)
                    {
                        Interface::m_contextMenu.function = func_right_click;
                        Interface::m_contextMenu.rootKey = root.key();
                        Interface::m_contextMenu.key = name;
                        Interface::m_contextMenu.value = _data.value();
                    }
                }
            }
        }
    }
    Interface::ProcessContextMenu();

    ImGui::EndChild();
}

void Widgets::CenterdText(const std::string& text)
{
    float font_size = ImGui::GetFontSize() * text.size() / 2;
    ImGui::NewLine();
    ImGui::SameLine(ImGui::GetWindowSize().x / 2 - font_size + (font_size / 1.8));
    ImGui::Text(text.c_str());
}