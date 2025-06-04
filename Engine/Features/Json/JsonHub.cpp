#include <Features/Json/JsonHub.h>

JsonHub* JsonHub::GetInstance()
{
    static JsonHub instance;
    return &instance;
}

void JsonHub::Initialize(const std::string& _rootDirectory)
{
    rootDirectoryPath_ = _rootDirectory;
    directoryPathFromRoot_ = rootDirectoryPath_;

    jsonLoader_ = std::make_unique<JsonLoader>(directoryPathFromRoot_, false);

    LoadRootDirectory();
}

void JsonHub::Save(const std::string& _groupName)
{
    jsonLoader_->OutPutJsonFile(_groupName);
}


void JsonHub::LoadRootDirectory()
{
    LoadFilesRecursively(rootDirectoryPath_);
}

void JsonHub::LoadFilesRecursively(const std::string& _directoryPath)
{
    if (!std::filesystem::exists(_directoryPath))
    {
        // ディレクトリが存在しない場合は早期リターン
        return;
    }

    // ディレクトリ内のファイルを読み込む
    for (auto& entry : std::filesystem::directory_iterator(_directoryPath, std::filesystem::directory_options::skip_permission_denied))
    {
        // ディレクトリの場合は再帰的に読み込む
        if (entry.is_directory())
        {
            // 再帰的に読み込む
            LoadFilesRecursively(entry.path().string());
        }
        else
        {
            // パスを取得
            std::string path = entry.path().string();
            // 拡張子を取得
            std::string ext = entry.path().extension().string();
            // 拡張子がjsonの場合は読み込む
            if (ext == ".json")
            {
                // jsonファイルを読み込む
                jsonLoader_->LoadJson(entry.path().string());
            }
        }
    }
}
