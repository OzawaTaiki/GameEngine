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
        }
    }
}

void ConfigManager::SaveData()
{
    for (auto [groupName, variable] : ptr_)
    {
        SaveData(groupName);
    }

}

void ConfigManager::SaveData(const std::string& _groupName)
{
    for (auto [variableName, value] : ptr_[_groupName])
    {
        if (value.address.index() == 0)
        {
            // uint32_t
            uint32_t* ptr = std::get<uint32_t*>(value.address);
            json_->SetData(_groupName, variableName, *ptr);
        }
        else if (value.address.index() == 1)
        {
            // float
            float* ptr = std::get<float*>(value.address);
            json_->SetData(_groupName, variableName, *ptr);
        }
        else if (value.address.index() == 2)
        {
            // Vector2
            Vector2* ptr = std::get<Vector2*>(value.address);
            json_->SetData(_groupName, variableName, *ptr);
        }
        else if (value.address.index() == 3)
        {
            // Vector3
            Vector3* ptr = std::get<Vector3*>(value.address);
            json_->SetData(_groupName, variableName, *ptr);
        }
        else if (value.address.index() == 4)
        {
            // Vector4
            Vector4* ptr = std::get<Vector4*>(value.address);
            json_->SetData(_groupName, variableName, *ptr);
        }
        else if (value.address.index() == 5)
        {
            // std::string
            std::string* ptr = std::get<std::string*>(value.address);
            json_->SetData(_groupName, variableName, *ptr);
        }
    }
    json_->SaveJson(_groupName);

}
