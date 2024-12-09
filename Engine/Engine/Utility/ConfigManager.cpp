#include "ConfigManager.h"
#include "JsonLoader.h"
#include "Config.h"
#include <Windows.h>
#include "Utility/Debug.h"

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

    // ディレクトリ内のファイルを読み込む
    LoadFilesRecursively(directoryPath_);


    for (auto& [groupName, variable] : value_)
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

                    // valueの型を取得
                    if (value.datum.index() == 0)
                    {
                        // uint32_t
                        uint32_t val = std::get<uint32_t>(value.datum);
                        value_[groupName][variableName].variable = val;
                    }
                    else if (value.datum.index() == 1)
                    {
                        float val = std::get<float>(value.datum);
                        value_[groupName][variableName].variable = val;
                    }
                    else if (value.datum.index() == 2)
                    {
                        Vector2 val = std::get<Vector2>(value.datum);
                        value_[groupName][variableName].variable = val;
                    }
                    else if (value.datum.index() == 3)
                    {
                        Vector3 val = std::get<Vector3>(value.datum);
                        value_[groupName][variableName].variable = val;
                    }
                    else if (value.datum.index() == 4)
                    {
                        Vector4 val = std::get<Vector4>(value.datum);
                        value_[groupName][variableName].variable = val;
                    }
                    else if (value.datum.index() == 5)
                    {
                        std::string val = std::get<std::string>(value.datum);
                        value_[groupName][variableName].variable = val;
                    }
                }
                else
                {
                    auto value = values[0];

                    // valueの型を取得
                    if (value.datum.index() == 0)
                    {
                        std::vector<uint32_t> val;
                        for (auto& v : values)
                            val.push_back(std::get<uint32_t>(v.datum));

                        value_[groupName][variableName].variable = val;
                    }
                    else if (value.datum.index() == 1)
                    {
                        std::vector<float> val;
                        for (auto& v : values)
                            val.push_back(std::get<float>(v.datum));
                        value_[groupName][variableName].variable = val;
                    }
                    else if (value.datum.index() == 2)
                    {
                        std::vector<Vector2> val;
                        for (auto& v : values)
                            val.push_back(std::get<Vector2>(v.datum));
                        value_[groupName][variableName].variable = val;
                    }
                    else if (value.datum.index() == 3)
                    {
                        std::vector<Vector3> val;
                        for (auto& v : values)
                            val.push_back(std::get<Vector3>(v.datum));
                        value_[groupName][variableName].variable = val;
                    }
                    else if (value.datum.index() == 4)
                    {
                        std::vector<Vector4> val;
                        for (auto& v : values)
                            val.push_back(std::get<Vector4>(v.datum));
                        value_[groupName][variableName].variable = val;
                    }
                    else if (value.datum.index() == 5)
                    {
                        std::vector<std::string> val;
                        for (auto& v : values)
                            val.push_back(std::get<std::string>(v.datum));
                        value_[groupName][variableName].variable = val;
                    }
                }
            }
        }
    }
}


void ConfigManager::SaveData(const std::string& _groupName)
{
    for (auto [groupName, values] : value_)
    {
        for (auto& [variableName, value] : values)
        {
            if (value.variable.index() == 0)
            {
                // uint32_t
                uint32_t val = std::get<uint32_t>(value.variable);
                json_->SetData(_groupName, variableName, val);
            }
            else if (value.variable.index() == 1)
            {
                float val = std::get<float>(value.variable);
                json_->SetData(_groupName, variableName, val);
            }
            else if (value.variable.index() == 2)
            {
                Vector2 val = std::get<Vector2>(value.variable);
                json_->SetData(_groupName, variableName, val);
            }
            else if (value.variable.index() == 3)
            {
                Vector3 val = std::get<Vector3>(value.variable);
                json_->SetData(_groupName, variableName, val);
            }
            else if (value.variable.index() == 4)
            {
                Vector4 val = std::get<Vector4>(value.variable);
                json_->SetData(_groupName, variableName, val);
            }
            else if (value.variable.index() == 5)
            {
                std::string val = std::get<std::string>(value.variable);
                json_->SetData(_groupName, variableName, val);
            }
            else if (value.variable.index() == 6)
            {
                std::vector<uint32_t> val = std::get<std::vector<uint32_t>>(value.variable);
                json_->SetData(_groupName, variableName, val);
            }
            else if (value.variable.index() == 7)
            {
                std::vector<float> val = std::get<std::vector<float>>(value.variable);
                json_->SetData(_groupName, variableName, val);
            }
            else if (value.variable.index() == 8)
            {
                std::vector<Vector2> val = std::get<std::vector<Vector2>>(value.variable);
                json_->SetData(_groupName, variableName, val);
            }
            else if (value.variable.index() == 9)
            {
                std::vector<Vector3> val = std::get<std::vector<Vector3>>(value.variable);
                json_->SetData(_groupName, variableName, val);
            }
            else if (value.variable.index() == 10)
            {
                std::vector<Vector4> val = std::get<std::vector<Vector4>>(value.variable);
                json_->SetData(_groupName, variableName, val);
            }
            else if (value.variable.index() == 11)
            {
                std::vector<std::string> val = std::get<std::vector<std::string>>(value.variable);
                json_->SetData(_groupName, variableName, val);
            }
        }
        json_->SaveJson(groupName);
    }
}

void ConfigManager::SetDirectoryPath(const std::string& _directoryPath)
{
    // directoryPath_のさいごに"/" がない場合は追加
    if (_directoryPath.back() != '/')
        json_->SetFolderPath(_directoryPath + "/");
    else
        directoryPath_ = _directoryPath;
}


void ConfigManager::LoadFilesRecursively(const std::string& _directoryPath)
{
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
            if (ext == "json")
            {
                // ファイル名からグループ名を取得
                std::string groupName = path.substr(path.find_last_of("/") + 1);
                groupName = groupName.substr(0, groupName.find_last_of("."));
                // ファイルを読み込む
                json_->LoadJson(groupName);
            }
        }
    }
}
