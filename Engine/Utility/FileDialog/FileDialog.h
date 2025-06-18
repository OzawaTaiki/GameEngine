#pragma once

#include <Core/WinApp/WinApp.h>

#include <commdlg.h>
#include <shlobj.h>

#include <string>
#include <map>
#include <vector>
#include <utility>

// 必要なライブラリ
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shell32.lib")

class FileFilterBuilder
{
public:
    enum class FilterType
    {
        AllFiles,
        TextFiles,
        ImageFiles,
        AudioFiles,
        DataFiles
    };

public:

    static std::string GetFilterString(FilterType _filterType);

    FileFilterBuilder& Add(FilterType _filterType);

    FileFilterBuilder& AddCustom(const std::string& _description, const std::string& _extension);

    std::string Build();

private:
    static std::map<FilterType, std::pair<std::string, std::string>> filterMap;
    std::string combinatedFilter_;

};



class FileDialog
{
public:
    static std::string OpenFile(const std::string& _filter);

    static std::vector<std::string> OpenMultipleFiles(const std::string& _filter);

    static std::string SaveFile(const std::string& _filter);

};