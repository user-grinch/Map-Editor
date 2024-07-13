#include "pch.h"
#include "widget.h"
#include "../interface.h"
#include "utils.h"

static struct {
    std::string root, key, val;
    bool show = false;
    bool added = false;
} contextMenu;

ImVec2 Widget::CalcSize(short count, bool spacing) {
    if (count == 1) {
        spacing = false;
    }

    float x = Utils::GetContentRegionWidth() - ImGui::GetStyle().ItemSpacing.x * (spacing ? count : 1);
    return ImVec2(x / count, ImGui::GetFrameHeight() * 1.3f);
}

ImVec2 Widget::CalcSizeFrame(const char* text) {
    return ImVec2(ImGui::CalcTextSize(text).x + 2 * ImGui::GetStyle().ItemSpacing.x, ImGui::GetFrameHeight());
}

void Widget::TextCentered(const std::string& text) {
    ImVec2 size = ImGui::CalcTextSize(text.c_str());
    ImGui::NewLine();
    float width = Utils::GetContentRegionWidth() - size.x;
    ImGui::SameLine(width/2);
    ImGui::Text(text.c_str());
}

void Widget::Tooltip(const char* text) {
    ImGui::SameLine();
    ImGui::TextDisabled("?");

    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text(text);
        ImGui::EndTooltip();
    }
}

bool Widget::Filter(const char* label, ImGuiTextFilter& filter, const char* hint) {
    bool state = filter.Draw(label);
    if (strlen(filter.InputBuf) == 0) {
        ImDrawList* drawlist = ImGui::GetWindowDrawList();

        ImVec2 min = ImGui::GetItemRectMin();
        min.x += ImGui::GetStyle().FramePadding.x;
        min.y += ImGui::GetStyle().FramePadding.y;

        drawlist->AddText(min, ImGui::GetColorU32(ImGuiCol_TextDisabled), hint);
    }
    return state;
}

void DrawClippedList(ResourceStore& data, fArg3_t clickFunc, bool favourites, fArgNone_t contextOptionsFunc) {
    // Category box
    ImGui::PushItemWidth(favourites ? Utils::GetContentRegionWidth() :
                         (Utils::GetContentRegionWidth() - ImGui::GetStyle().ItemSpacing.x)/2);

    if (!favourites) {
        if (Widget::ListBox("##Categories", data.m_Categories, data.m_Selected)) {
            data.UpdateSearchList(favourites);
        }
        ImGui::SameLine();
    }

    if (Widget::Filter("##Filter", data.m_Filter, "Search")) {
        data.UpdateSearchList(favourites);
    }
    if (ImGui::IsItemActive()) {
        Interface.m_bInputLocked = true;
    }
    ImGui::PopItemWidth();

    ImGui::Spacing();
    if (favourites && data.m_pData->GetTable("Favourites")->size() == 0) {
        Widget::TextCentered("No favourites found!");
    }
    ImGui::BeginChild("CliipingLIst");
    ImGuiListClipper clipper;
    clipper.Begin(data.m_nSearchList.size(), ImGui::GetTextLineHeight());
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
            std::string &label = std::get<ListLookup>(data.m_nSearchList[i]).key;
            std::string &cat = std::get<ListLookup>(data.m_nSearchList[i]).cat;
            std::string &val =  std::get<ListLookup>(data.m_nSearchList[i]).val;

            if (ImGui::MenuItem(label.c_str()) && clickFunc != nullptr) {
                clickFunc(cat, label, val);
            }

            if (ImGui::IsItemClicked(1)) {
                contextMenu = {cat, label, val, true};
            }
        }
    }

    if (contextMenu.show) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
        if (ImGui::BeginPopupContextWindow()) {
            ImGui::Text(contextMenu.key.c_str());
            ImGui::Separator();

            if (!favourites && ImGui::MenuItem("Add to favourites")) {
                data.m_pData->Set(std::format("Favourites.{}", contextMenu.key).c_str(), contextMenu.val);
                data.m_pData->Save();
            }
            if (ImGui::MenuItem("Remove")) {
                if (favourites) {
                    data.m_pData->RemoveKey("Favourites", contextMenu.key.c_str());
                    data.m_pData->Save();
                    data.UpdateSearchList(true);
                } else {
                    if (contextMenu.root == "Custom" || data.m_bAllowRemoveAll) {
                        data.m_pData->RemoveKey(contextMenu.root.c_str(), contextMenu.key.c_str());
                        data.m_pData->RemoveKey("Favourites", contextMenu.key.c_str());
                        data.m_pData->Save();
                        data.UpdateSearchList();
                        CHud::SetHelpMessage("Removed entry", false, false, false);
                    } else {
                        CHud::SetHelpMessage("You can only remove custom items!", false, false, false);
                    }
                }
            }
            if (contextOptionsFunc) {
                contextOptionsFunc();
            }

            if (ImGui::MenuItem("Close")) {
                contextMenu.show = false;
            }
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();
    }
    ImGui::EndChild();
}

void Widget::DataList(ResourceStore& data, fArg3_t clickFunc, fArgNone_t addFunc, fArgNone_t contextOptionsFunc, fArgNone_t tabsFunc) {
    if (ImGui::IsMouseClicked(1)) {
        contextMenu.show = false;
    }

    // Drawing the list here
    if (ImGui::BeginTabBar("MYTABS")) {
        if (ImGui::BeginTabItem("Search")) {
            ImGui::Spacing();
            DrawClippedList(data, clickFunc, false, contextOptionsFunc);
            ImGui::EndTabItem();
        }
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            data.UpdateSearchList();
        }
        if (ImGui::BeginTabItem("Favourites")) {
            ImGui::Spacing();
            DrawClippedList(data, clickFunc, true, contextOptionsFunc);
            ImGui::EndTabItem();
        }
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            data.UpdateSearchList(true);
        }
        if (addFunc) {
            if (ImGui::BeginTabItem("Add new")) {
                ImGui::Spacing();
                ImGui::BeginChild("AddNew2");
                addFunc();
                ImGui::EndChild();
                ImGui::EndTabItem();
            }
        }
        if (tabsFunc) {
            tabsFunc();
        }
        ImGui::EndTabBar();
    }
}


void Widget::DataListFav(ResourceStore& data, fArg3_t clickFunc, fArgNone_t contextOptionsFunc) {
    if (ImGui::IsMouseClicked(1)) {
        contextMenu.show = false;
    }
    ImGui::Spacing();
    DrawClippedList(data, clickFunc, true, contextOptionsFunc);
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        data.UpdateSearchList(true);
    }
}

bool Widget::ListBox(const char* label, VecStr& allItems, std::string& selected) {
    bool rtn = false;
    if (ImGui::BeginCombo(label, selected.c_str())) {
        for (std::string curItem : allItems) {
            if (ImGui::MenuItem(curItem.c_str())) {
                selected = curItem;
                rtn = true;
            }
        }
        ImGui::EndCombo();
    }

    return rtn;
}

bool Widget::ListBox(const char* label, VecStr& allItems, int& selected) {
    bool rtn = false;
    if (ImGui::BeginCombo(label, allItems[selected].c_str())) {
        for (size_t index = 0; index < allItems.size(); ++index) {
            if (ImGui::MenuItem(allItems[index].c_str())) {
                selected = index;
                rtn = true;
            }
        }
        ImGui::EndCombo();
    }

    return rtn;
}