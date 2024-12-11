#include "ConfigManager.h"
#include "Config.h"
#include <Systems/Utility/JsonLoader.h>
#include <Windows.h>
#include <Systems/Utility/Debug.h>

#include <sstream>
#include <fstream>
#include <filesystem>

// TODO : アニメーションカーブみたいなのを作る

ConfigManager* ConfigManager::GetInstance()
{
    static ConfigManager instance;
    return &instance;
}

void ConfigManager::Initialize()
{
    rootDirectory_ = "resources/Data";
    directoryPath_ = "resources/Data";
    json_ = new JsonLoader(directoryPath_, false);
    LoadRootDirectory();
}


void ConfigManager::LoadRootDirectory()
{
    LoadFilesRecursively(rootDirectory_);
}


void ConfigManager::LoadData()
{
    // directoryPath_のさいごに"/" がない場合は追加
    if (directoryPath_.back() != '/')
    {
        directoryPath_ += "/";
    }

    // directoryPath_が存在しない場合は作成
    if (!std::filesystem::exists(directoryPath_))
    {
        std::filesystem::create_directories(directoryPath_);
    }


    for (std::string& groupName : groupNames_)
    {
        auto data = json_->GetData(groupName);

        // データがない場合はスキップ
        if (data.has_value())
        {
            for (const auto& [variableName, values] : data.value().data)
            {
                if (values.size() == 0)
                    continue;

                if (values.size() == 1)
                {
                    auto value = values[0];

                    switch (value.datum.index())
                    {
                    case 0:// uint32_t
                        {
                            uint32_t val = std::get<uint32_t>(value.datum);
                            value_[groupName][variableName].variable = val;
                        }
                        break;
                    case 1:// float
                        {
                            float val = std::get<float>(value.datum);
                            value_[groupName][variableName].variable = val;
                        }
                        break;
                    case 2:// Vector2
                        {
                            Vector2 val = std::get<Vector2>(value.datum);
                            value_[groupName][variableName].variable = val;
                        }
                        break;
                    case 3:// Vector3
                        {
                            Vector3 val = std::get<Vector3>(value.datum);
                            value_[groupName][variableName].variable = val;
                        }
                        break;
                    case 4:// Vector4
                        {
                            Vector4 val = std::get<Vector4>(value.datum);
                            value_[groupName][variableName].variable = val;
                        }
                        break;
                    case 5:// std::string
                        {
                            std::string val = std::get<std::string>(value.datum);
                            value_[groupName][variableName].variable = val;
                        }
                        break;

                    default:
                        break;
                    }
                }
                else
                {
                    auto value = values[0];

                    switch (value.datum.index())
                    {
                    case 0:// uint32_t
                        {
                            std::vector<uint32_t> val;
                            for (auto& v : values)
                                val.push_back(std::get<uint32_t>(v.datum));
                            value_[groupName][variableName].variable = val;
                        }
                        break;
                    case 1:// float
                        {
                            std::vector<float> val;
                            for (auto& v : values)
                                val.push_back(std::get<float>(v.datum));
                            value_[groupName][variableName].variable = val;
                        }
                        break;
                    case 2:// Vector2
                        {
                            std::vector<Vector2> val;
                            for (auto& v : values)
                                val.push_back(std::get<Vector2>(v.datum));
                            value_[groupName][variableName].variable = val;
                        }
                        break;
                    case 3:// Vector3
                        {
                            std::vector<Vector3> val;
                            for (auto& v : values)
                                val.push_back(std::get<Vector3>(v.datum));
                            value_[groupName][variableName].variable = val;
                        }
                        break;
                    case 4:// Vector4
                        {
                            std::vector<Vector4> val;
                            for (auto& v : values)
                                val.push_back(std::get<Vector4>(v.datum));
                            value_[groupName][variableName].variable = val;
                        }
                        break;
                    case 5:// std::string
                        {
                            std::vector<std::string> val;
                            for (auto& v : values)
                                val.push_back(std::get<std::string>(v.datum));
                            value_[groupName][variableName].variable = val;
                        }
                        break;

                    default:
                        break;
                    }
                }
            }
        }
    }
}


void ConfigManager::SaveData(const std::string& _groupName)
{
    for (auto& [variableName, value] : value_[_groupName])
    {
        switch (value.variable.index())
        {
        case 0:// uint32_t
            {
                uint32_t val = std::get<uint32_t>(value.variable);
                json_->SetData(_groupName, variableName, val);
            }
            break;
        case 1:// float
            {
                float val = std::get<float>(value.variable);
                json_->SetData(_groupName, variableName, val);
            }
            break;
        case 2:// Vector2
            {
                Vector2 val = std::get<Vector2>(value.variable);
                json_->SetData(_groupName, variableName, val);
            }
            break;
        case 3:// Vector3
            {
                Vector3 val = std::get<Vector3>(value.variable);
                json_->SetData(_groupName, variableName, val);
            }
            break;
        case 4:// Vector4
            {
                Vector4 val = std::get<Vector4>(value.variable);
                json_->SetData(_groupName, variableName, val);
            }
            break;
        case 5:// std::string
            {
                std::string val = std::get<std::string>(value.variable);
                json_->SetData(_groupName, variableName, val);
            }
            break;
        case 6:// std::vector<uint32_t>
            {
                std::vector<uint32_t> val = std::get<std::vector<uint32_t>>(value.variable);
                json_->SetData(_groupName, variableName, val);
            }
            break;
        case 7:// std::vector<float>
            {
                std::vector<float> val = std::get<std::vector<float>>(value.variable);
                json_->SetData(_groupName, variableName, val);
            }
            break;
        case 8:// std::vector<Vector2>
            {
                std::vector<Vector2> val = std::get<std::vector<Vector2>>(value.variable);
                json_->SetData(_groupName, variableName, val);
            }
            break;
        case 9:// std::vector<Vector3>
            {
                std::vector<Vector3> val = std::get<std::vector<Vector3>>(value.variable);
                json_->SetData(_groupName, variableName, val);
            }
            break;
        case 10:// std::vector<Vector4>
            {
                std::vector<Vector4> val = std::get<std::vector<Vector4>>(value.variable);
                json_->SetData(_groupName, variableName, val);
            }
            break;
        case 11:// std::vector<std::string>
            {
                std::vector<std::string> val = std::get<std::vector<std::string>>(value.variable);
                json_->SetData(_groupName, variableName, val);
            }
            break;
        case 12:// std::list<uint32_t>
            {
                std::list<uint32_t> val = std::get<std::list<uint32_t>>(value.variable);
                // listをvectorに変換
                std::vector<uint32_t> vec(val.begin(), val.end());
                json_->SetData(_groupName, variableName, vec);
            }
            break;
        case 13:// std::list<float>
            {
                std::list<float> val = std::get<std::list<float>>(value.variable);
                // listをvectorに変換
                std::vector<float> vec(val.begin(), val.end());
                json_->SetData(_groupName, variableName, vec);
            }
            break;
        case 14:// std::list<Vector2>
            {
                std::list<Vector2> val = std::get<std::list<Vector2>>(value.variable);
                // listをvectorに変換
                std::vector<Vector2> vec(val.begin(), val.end());
                json_->SetData(_groupName, variableName, vec);
            }
            break;
        case 15:// std::list<Vector3>
            {
                std::list<Vector3> val = std::get<std::list<Vector3>>(value.variable);
                // listをvectorに変換
                std::vector<Vector3> vec(val.begin(), val.end());
                json_->SetData(_groupName, variableName, vec);
            }
            break;
        case 16:// std::list<Vector4>
            {
                std::list<Vector4> val = std::get<std::list<Vector4>>(value.variable);
                // listをvectorに変換
                std::vector<Vector4> vec(val.begin(), val.end());
                json_->SetData(_groupName, variableName, vec);
            }
            break;
        case 17:// std::list<std::string>
            {
                std::list<std::string> val = std::get<std::list<std::string>>(value.variable);
                // listをvectorに変換
                std::vector<std::string> vec(val.begin(), val.end());
                json_->SetData(_groupName, variableName, vec);
            }
            break;
        default:
            break;
        }

    }
    json_->SaveJson(_groupName);
}

void ConfigManager::EraseData(const std::string& _groupName, const std::string& _variableName)
{
    if (value_.contains(_groupName)&&value_[_groupName].contains(_variableName))
    {
        value_[_groupName].erase(_variableName);
    }
}

void ConfigManager::SetDirectoryPath(const std::string& _directoryPath)
{
    // directoryPath_のさいごに"/" がない場合は追加
    if (_directoryPath.back() != '/')
        json_->SetFolderPath(_directoryPath + "/");
    else
    {
        directoryPath_ = _directoryPath;
        json_->SetFolderPath(directoryPath_);
    }

}


void ConfigManager::LoadFilesRecursively(const std::string& _directoryPath)
{

    std::vector<std::string> groupNames;

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
                // ファイル名からグループ名を取得
                std::string groupName = path.substr(path.find_last_of("\\") + 1);

                // jsonファイルを読み込む
                json_->LoadJson(entry.path().string());

                groupName = groupName.substr(0, groupName.find_last_of("."));
                groupNames_.push_back(groupName);
            }

        }
    }
}
