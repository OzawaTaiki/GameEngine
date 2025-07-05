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

#ifdef CreateFile
#undef CreateFile // WindowsのCreateFileマクロを無効化
#endif // CreateFile

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
    // 単体フィルター文字列取得（Windows API直接使用時）
    static std::string GetFilterString(FilterType _filterType);

    // ビルダーにフィルターを追加
    FileFilterBuilder& Add(FilterType _filterType);

    // カスタムフィルターを追加
    FileFilterBuilder& AddCustom(const std::string& _description, const std::string& _extension);

    // 個別拡張子として追加
    FileFilterBuilder& AddSeparateExtensions(FilterType _filterType);

    // 最終的なフィルター文字列を構築
    std::string Build();

    // ビルダーをリセット
    FileFilterBuilder& Reset();

private:
    void AddImageExtensions();
    void AddAudioExtensions();
    void AddDataExtensions();
    void AppendFilter(const std::string& _description, const std::string& _extension);

    static std::map<FilterType, std::pair<std::string, std::string>> filterMap;
    std::vector<char> combinatedFilter_;
};




class FileDialog
{
public:
    static std::string OpenFile(const std::string& _filter);

    static std::vector<std::string> OpenMultipleFiles(const std::string& _filter);

    static std::string SaveFile(const std::string& _filter);

    static std::string CreateFile(const std::string& _filter, const std::string& _defaultName = "NewFile");

    static std::string SaveFileAs(const std::string& _filter, const std::string& _defaultName = "NewFile");


};