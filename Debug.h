#pragma once
#include <format>
#include <string>

namespace Debug {
	void Log(const std::string& message);
	std::wstring ConvertString(const std::string& _str);
	std::string ConvertString(const std::wstring& _str);
};