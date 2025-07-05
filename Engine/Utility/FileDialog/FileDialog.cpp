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
        // Windows API用：説明\0パターン\0\0 形式
        std::string result = it->second.first + '\0' + it->second.second + '\0' + '\0';
        return result;
    }
    else
    {
        // デフォルトフィルター
        return std::string("All Files (*.*)\0*.*\0\0", 18);
    }
}

FileFilterBuilder& FileFilterBuilder::Add(FilterType _filterType)
{
    auto it = filterMap.find(_filterType);
    if (it != filterMap.end())
    {
        AppendFilter(it->second.first, it->second.second);
    }
    else
    {
        // 見つからない場合はAllFilesを追加
        AppendFilter("All Files (*.*)", "*.*");
    }
    return *this;
}

FileFilterBuilder& FileFilterBuilder::AddCustom(const std::string& _description, const std::string& _extension)
{
    AppendFilter(_description, _extension);
    return *this;
}

std::string FileFilterBuilder::Build()
{
    if (combinatedFilter_.empty())
    {
        // 空の場合はデフォルトのAll Filesを追加
        AppendFilter("All Files (*.*)", "*.*");
    }

    // 最終的な終端文字を追加（\0\0にするため）
    combinatedFilter_.push_back('\0');

    return std::string(combinatedFilter_.data(), combinatedFilter_.size());
}

FileFilterBuilder& FileFilterBuilder::Reset()
{
    combinatedFilter_.clear();
    return *this;
}

FileFilterBuilder& FileFilterBuilder::AddSeparateExtensions(FilterType _filterType)
{
    switch (_filterType)
    {
    case FilterType::AllFiles:
        Add(FilterType::AllFiles);
        break;
    case FilterType::TextFiles:
        Add(FilterType::TextFiles);
        break;
    case FilterType::ImageFiles:
        AddImageExtensions();
        break;
    case FilterType::AudioFiles:
        AddAudioExtensions();
        break;
    case FilterType::DataFiles:
        AddDataExtensions();
        break;
    default:
        break;
    }
    return *this;
}

void FileFilterBuilder::AddImageExtensions()
{
    AddCustom("PNG Files (*.png)", "*.png");
    AddCustom("JPG Files (*.jpg)", "*.jpg");
    AddCustom("JPEG Files (*.jpeg)", "*.jpeg");
}

void FileFilterBuilder::AddAudioExtensions()
{
    AddCustom("MP3 Files (*.mp3)", "*.mp3");
    AddCustom("WAV Files (*.wav)", "*.wav");
}

void FileFilterBuilder::AddDataExtensions()
{
    AddCustom("JSON Files (*.json)", "*.json");
    AddCustom("CSV Files (*.csv)", "*.csv");
    AddCustom("XML Files (*.xml)", "*.xml");
    AddCustom("DAT Files (*.dat)", "*.dat");
}

void FileFilterBuilder::AppendFilter(const std::string& _description, const std::string& _extension)
{
    // 説明文字列を追加
    combinatedFilter_.insert(combinatedFilter_.end(), _description.begin(), _description.end());
    combinatedFilter_.push_back('\0');

    // パターン文字列を追加
    combinatedFilter_.insert(combinatedFilter_.end(), _extension.begin(), _extension.end());
    combinatedFilter_.push_back('\0');
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

std::string FileDialog::CreateFile(const std::string& _filter, const std::string& _defaultName)
{
    OPENFILENAMEA ofn = { 0 };
    char szFile[260] = { 0 };

    strcpy_s(szFile, _defaultName.c_str()); // デフォルト名を設定

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = WinApp::GetInstance()->GetHwnd();
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = _filter.c_str();
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;

    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR | OFN_EXPLORER | OFN_HIDEREADONLY;

    if (GetSaveFileNameA(&ofn))
    {
        return std::string(szFile);
    }

    static const std::string emptyResult;
    return emptyResult; // ファイル作成がキャンセルされた場合は空の文字列を返す
}

std::string FileDialog::SaveFileAs(const std::string& _filter, const std::string& _defaultName)
{
    OPENFILENAMEA ofn = { 0 };
    char szFile[260] = { 0 };

    strcpy_s(szFile, _defaultName.c_str()); // デフォルト名を設定

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = WinApp::GetInstance()->GetHwnd();
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = _filter.c_str();
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;

    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR | OFN_EXPLORER | OFN_HIDEREADONLY;
    if (GetSaveFileNameA(&ofn))
    {
        return std::string(szFile);
    }
    static const std::string emptyResult;
    return emptyResult; // ファイル保存がキャンセルされた場合は空の文字列を返す
}
