#pragma once

#include <string>
#include <Windows.h>


namespace Engine {

std::wstring ConvertString(const std::string& str);

std::string ConvertString(const std::wstring& str);

} // namespace Engine
