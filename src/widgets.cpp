#include "pch.h"
#include "widgets.h"
#include "interface.h"

void Widgets::ShowTooltip(const char* text)
{
    ImGui::SameLine();
    ImGui::TextDisabled("?");

    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text(text);
        ImGui::EndTooltip();
    }
}

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

void Widgets::DrawJSON(ResourceStore& data, ContextMenu& context, f_Arg3 func_left_click, f_Arg3 func_right_click)
{
    ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() / 2 - 5);
    ListBoxStr("##Categories", data.m_Categories, data.m_Selected);
    ImGui::SameLine();

    data.m_Filter.Draw("##Filter");

    Interface::m_bInputLocked = ImGui::IsItemActive();
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
                        context.m_bShow = true;
                        context.m_pFunc = func_right_click;
                        context.m_Root = root.key();
                        context.m_Key = name;
                        context.m_Val = _data.value();
                    }
                }
            }
        }
    }

    // context menu
    if (context.m_bShow)
    {
        if (ImGui::BeginPopupContextWindow("TMenu"))
        {
            ImGui::Text(context.m_Key.c_str());
            ImGui::Separator();

            context.m_pFunc(context.m_Root, context.m_Key, context.m_Val);

            if (ImGui::MenuItem("Close"))
            {
                context.m_bShow = false;
            }

            ImGui::EndPopup();
        }
    }
    ImGui::EndChild();
}

void Widgets::CenterdText(const std::string& text)
{
    float font_size = ImGui::GetFontSize() * text.size() / 2;
    ImGui::NewLine();
    ImGui::SameLine(ImGui::GetWindowSize().x / 2 - font_size + (font_size / 1.8));
    ImGui::Text(text.c_str());
}