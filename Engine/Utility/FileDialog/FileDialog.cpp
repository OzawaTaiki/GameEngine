#include "FileDialog.h"


std::map<FileFilterBuilder::FilterType, std::pair<std::string, std::string>>
FileFilterBuilder::filterMap = {
    { FilterType::AllFiles, { "All Files (*.*)", "*.*" } },
    { FilterType::TextFiles, { "Text Files (*.txt)", "*.txt" } },
    { FilterType::ImageFiles, { "Image Files (*.png;*.jpg;*.jpeg)", "*.png;*.jpg;*.jpeg" } },
    { FilterType::AudioFiles, { "Audio Files (*.mp3;*.wav)", "*.mp3;*.wav" } },
    { FilterType::DataFiles, { "Data Files (*.dat;*.json;*.csv;*.xml)", "*.dat;*.json;*.csv;*.xml" } }
};


std::string FileFilterBuilder::GetFilterString(FilterType filterType)
{
    auto it = filterMap.find(filterType);
    if (it != filterMap.end())
    {
        return it->second.first + '\0' + it->second.second + '\0';
    }
    else
    {
        static const std::string emptyFilter = "All Files(*.*)\0*.*\0";
        return emptyFilter; // デフォルトのフィルター名を返す
    }
}

FileFilterBuilder& FileFilterBuilder::Add(FilterType _filterType)
{
    auto it = filterMap.find(_filterType);
    if (it != filterMap.end())
    {
        combinatedFilter_ += it->second.first + '\0' + it->second.second + '\0';
    }
    return *this;
}

FileFilterBuilder& FileFilterBuilder::AddCustom(const std::string& _description, const std::string& _extension)
{
    combinatedFilter_ += _description + '\0' + _extension + '\0';
    return *this;
}

std::string FileFilterBuilder::Build()
{
    if (combinatedFilter_.empty())
    {
        combinatedFilter_ = GetFilterString(FilterType::AllFiles);
    }
    return combinatedFilter_;
}


std::string FileDialog::OpenFile(const std::string& _filter)
{
    OPENFILENAMEA ofn = { 0 };

    char szFile[260] = { 0 };

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = WinApp::GetInstance()->GetHwnd();
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = _filter.c_str();
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_EXPLORER;
    if (GetOpenFileNameA(&ofn))
    {
        return std::string(szFile);
    }

    static const std::string emptyResult;
    return emptyResult; // ファイル選択がキャンセルされた場合は空の文字列を返す
}

std::vector<std::string> FileDialog::OpenMultipleFiles(const std::string& _filter)
{
    OPENFILENAMEA ofn = { 0 };

    char szFile[260] = { 0 };

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = WinApp::GetInstance()->GetHwnd();
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = _filter.c_str();
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

    std::vector<std::string> result;

    // ファイル選択ダイアログを表示
    if (GetOpenFileNameA(&ofn))
    {
        // 選択されたファイル名を取得
        char* ptr = szFile;
        std::string directory = ptr; // 最初の部分はディレクトリ名
        ptr += directory.length() + 1; // ディレクトリ名の後にあるNULL文字をスキップ

        if (*ptr == '\0') // もしディレクトリ名の後にNULLがある場合
        {
            result.push_back(directory); // ディレクトリ名を結果に追加
        }
        else
        {
            while (*ptr != '\0')
            {
                std::string filename = ptr;
                result.push_back(directory + "\\" + filename);
                ptr += filename.length() + 1; // ファイル名の後にあるNULL文字をスキップ
            }
        }
    }

    return result; // 選択されたファイル名のリストを返す

}

std::string FileDialog::SaveFile(const std::string& _filter)
{
    OPENFILENAMEA ofn = { 0 };
    char szFile[260] = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = WinApp::GetInstance()->GetHwnd();
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = _filter.c_str();
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR | OFN_EXPLORER;

    if (GetSaveFileNameA(&ofn))
    {
        return std::string(szFile);
    }
    static const std::string emptyResult;
    return emptyResult; // ファイル保存がキャンセルされた場合は空の文字列を返す
}
