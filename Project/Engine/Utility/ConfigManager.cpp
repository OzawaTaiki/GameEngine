#include "ConfigManager.h"
#include "JsonLoader.h"
#include "Windows.h"

#include <sstream>
#include <fstream>
#include <filesystem>

ConfigManager* ConfigManager::GetInstance()
{
    static ConfigManager instance;
    return &instance;
}

void ConfigManager::Initialize()
{
    directoryPath_ = "resources/Data/Parameter/";
    json_ = new JsonLoader(directoryPath_, false);
}

void ConfigManager::Update()
{
}

void ConfigManager::Draw()
{
}

void ConfigManager::LoadData()
{
    // ディレクトリ内のファイルを読み込む
    for (auto& entry : std::filesystem::directory_iterator(directoryPath_, std::filesystem::directory_options::skip_permission_denied))
    {
        // ディレクトリは無視
        if (std::filesystem::is_directory(entry.path()) || entry.path().extension() != ".json")
        {
            continue;
        }

        std::string gName = entry.path().stem().string();
        // ファイル名を取得
        std::string path = directoryPath_ + gName;

        // ファイル名を元にJsonLoaderに読み込みを依頼
        json_->LoadJson(path);

        groupNames_.push_back(gName);
    }

    for (const std::string& groupName : groupNames_)
    {
        auto data = json_->GetData(groupName);

        // データがない場合はスキップ
        if(!data.has_value())
        {
            continue;
        }

        for (const auto& [variableName, values] : data.value().data)
        {
            for (const auto& value : values)
            {
                // valueの型を取得
                if (value.datum.index() == 0)
                {
                    // uint32_t
                    uint32_t* ptr = new uint32_t;
                    *ptr = std::get<uint32_t>(value.datum);
                    data_[groupName][variableName].variable = ptr;
                }
                else if (value.datum.index() == 1)
                {
                    // float
                    float* ptr = new float;
                    *ptr = std::get<float>(value.datum);
                    data_[groupName][variableName].variable = ptr;
                }
                else if (value.datum.index() == 2)
                {
                    // Vector2
                    Vector2* ptr = new Vector2;
                    *ptr = std::get<Vector2>(value.datum);
                    data_[groupName][variableName].variable = ptr;
                }
                else if (value.datum.index() == 3)
                {
                    // Vector3
                    Vector3* ptr = new Vector3;
                    *ptr = std::get<Vector3>(value.datum);
                    data_[groupName][variableName].variable = ptr;
                }
                else if (value.datum.index() == 4)
                {
                    // Vector4
                    Vector4* ptr = new Vector4;
                    *ptr = std::get<Vector4>(value.datum);
                    data_[groupName][variableName].variable = ptr;
                }
            }
        }
    }
}

void ConfigManager::SaveData()
{
    for (auto [groupName, variable] : data_)
    {
        for (auto [variableName, value] : variable)
        {
            if (value.variable.index() == 0)
            {
                // uint32_t
                uint32_t* ptr = std::get<uint32_t*>(value.variable);
                json_->SetData(groupName, variableName, *ptr);
            }
            else if (value.variable.index() == 1)
            {
                // float
                float* ptr = std::get<float*>(value.variable);
                json_->SetData(groupName, variableName, *ptr);
            }
            else if (value.variable.index() == 2)
            {
                // Vector2
                Vector2* ptr = std::get<Vector2*>(value.variable);
                json_->SetData(groupName, variableName, *ptr);
            }
            else if (value.variable.index() == 3)
            {
                // Vector3
                Vector3* ptr = std::get<Vector3*>(value.variable);
                json_->SetData(groupName, variableName, *ptr);
            }
            else if (value.variable.index() == 4)
            {
                // Vector4
                Vector4* ptr = std::get<Vector4*>(value.variable);
                json_->SetData(groupName, variableName, *ptr);
            }
        }
        json_->SaveJson(groupName);
    }

}
