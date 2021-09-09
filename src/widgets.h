#pragma once
#include <vector>
#include <string>
#include "pch.h"

class Widgets
{
private:
	struct JsonPopUpData
	{
		std::function<void(std::string&, std::string&, std::string&)> function;
		std::string key;
		std::string rootKey;
		std::string value;
	};

public:
    static inline JsonPopUpData jsonPopup;

	Widgets() = delete;;
	Widgets(Widgets&) = delete;
	
	static void CenterdText(const std::string& text);
    static bool ListBoxStr(const char* label, std::vector<std::string>& all_items, std::string& selected);
    static void DrawJSON(ResourceStore& data,
				std::function<void(std::string&, std::string&, std::string&)> func_left_click,
				std::function<void(std::string&, std::string&, std::string&)> func_right_click);
};