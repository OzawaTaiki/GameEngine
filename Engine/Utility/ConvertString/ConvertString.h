#pragma once

#include <string>
#include <Windows.h>

std::wstring ConvertString(const std::string& str);

std::string ConvertString(const std::wstring& str);
