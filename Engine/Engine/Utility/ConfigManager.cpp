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

void ConfigManager::Update()
{
}

void ConfigManager::Draw()
{
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

    if (sceneName_ == "")
    {
        return;
    }

    for (auto& [groupName, variable] : value_[sceneName_])
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
                        value_[sceneName_][groupName][variableName].variable = val;
                    }
                    else if (value.datum.index() == 1)
                    {
                        float val = std::get<float>(value.datum);
                        value_[sceneName_][groupName][variableName].variable = val;
                    }
                    else if (value.datum.index() == 2)
                    {
                        Vector2 val = std::get<Vector2>(value.datum);
                        value_[sceneName_][groupName][variableName].variable = val;
                    }
                    else if (value.datum.index() == 3)
                    {
                        Vector3 val = std::get<Vector3>(value.datum);
                        value_[sceneName_][groupName][variableName].variable = val;
                    }
                    else if (value.datum.index() == 4)
                    {
                        Vector4 val = std::get<Vector4>(value.datum);
                        value_[sceneName_][groupName][variableName].variable = val;
                    }
                    else if (value.datum.index() == 5)
                    {
                        std::string val = std::get<std::string>(value.datum);
                        value_[sceneName_][groupName][variableName].variable = val;
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

                        value_[sceneName_][groupName][variableName].variable = val;
                    }
                    else if (value.datum.index() == 1)
                    {
                        std::vector<float> val;
                        for (auto& v : values)
                            val.push_back(std::get<float>(v.datum));
                        value_[sceneName_][groupName][variableName].variable = val;
                    }
                    else if (value.datum.index() == 2)
                    {
                        std::vector<Vector2> val;
                        for (auto& v : values)
                            val.push_back(std::get<Vector2>(v.datum));
                        value_[sceneName_][groupName][variableName].variable = val;
                    }
                    else if (value.datum.index() == 3)
                    {
                        std::vector<Vector3> val;
                        for (auto& v : values)
                            val.push_back(std::get<Vector3>(v.datum));
                        value_[sceneName_][groupName][variableName].variable = val;
                    }
                    else if (value.datum.index() == 4)
                    {
                        std::vector<Vector4> val;
                        for (auto& v : values)
                            val.push_back(std::get<Vector4>(v.datum));
                        value_[sceneName_][groupName][variableName].variable = val;
                    }
                    else if (value.datum.index() == 5)
                    {
                        std::vector<std::string> val;
                        for (auto& v : values)
                            val.push_back(std::get<std::string>(v.datum));
                        value_[sceneName_][groupName][variableName].variable = val;
                    }
                }
            }
        }
    }
}

void ConfigManager::SaveData()
{
    for (auto [groupName, variable] : ptr_[sceneName_])
    {
        SaveData(groupName);
    }
}

void ConfigManager::SaveData(const std::string& _groupName)
{
    for (auto [variableName, value] : ptr_[sceneName_][_groupName])
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
        else if (value.address.index() == 6)
        {
            // std::vector<uint32_t>
            auto ptr = std::get<std::vector<uint32_t>*>(value.address);
            std::vector<uint32_t> data;
            for (auto& p : *ptr)
                data.push_back(p);
            json_->SetData(_groupName, variableName, data);
        }
        else if (value.address.index() == 7)
        {
            // std::vector<float>
            auto ptr = std::get<std::vector<float>*>(value.address);
            std::vector<float> data;
            for (auto& p : *ptr)
                data.push_back(p);
            json_->SetData(_groupName, variableName, data);
        }
        else if(value.address.index() == 8)
        {
            // std::vector<Vector2>
            auto ptr = std::get<std::vector<Vector2>*>(value.address);
            std::vector<Vector2> data;
            for (auto& p : *ptr)
                data.push_back(p);
            json_->SetData(_groupName, variableName, data);
        }
        else if (value.address.index() == 9)
        {
            // std::vector<Vector3>
            auto ptr = std::get<std::vector<Vector3>*>(value.address);
            std::vector<Vector3> data;
            for (auto& p : *ptr)
                data.push_back(p);
            json_->SetData(_groupName, variableName, data);
        }
        else if (value.address.index() == 10)
        {
            // std::vector<Vector4>
            auto ptr = std::get<std::vector<Vector4>*>(value.address);
            std::vector<Vector4> data;
            for (auto& p : *ptr)
                data.push_back(p);
            json_->SetData(_groupName, variableName, data);
        }
        else if (value.address.index() == 11)
        {
            // std::vector<std::string>
            auto ptr = std::get<std::vector<std::string>*>(value.address);
            std::vector<std::string> data;
            for (auto& p : *ptr)
                data.push_back(p);
            json_->SetData(_groupName, variableName, data);
        }

    }

    json_->SaveJson(_groupName);

}

void ConfigManager::SetSceneName(const std::string& _scene)
{
    sceneName_ = _scene;
    if (configs_.contains(_scene))

    directoryPath_ = configs_[_scene]->GetDirectoryPath();
    else
        Create(_scene);

    json_->SetFolderPath(directoryPath_ + "/");

}

void ConfigManager::SetDirectoryPath(const std::string& _directoryPath)
{
    // directoryPath_のさいごに"/" がない場合は追加
    if (_directoryPath.back() != '/')
        json_->SetFolderPath(_directoryPath + "/");
    else
        directoryPath_ = _directoryPath;
}

Config* ConfigManager::Create(const std::string& _sceneName)
{
    // 存在しない場合は新規作成
    if (!configs_.contains(_sceneName))
    {
        configs_[_sceneName] = std::make_unique<Config>();
        directoryPath_ = configs_[_sceneName]->GetDirectoryPath() + "/" + _sceneName;
        configs_[_sceneName]->SetDirectoryPath(directoryPath_);
        LoadData();
    }

    return configs_[_sceneName].get();
}

void ConfigManager::LoadFilesRecursively(const std::string& _directoryPath)
{
    for (auto& entry : std::filesystem::directory_iterator(_directoryPath, std::filesystem::directory_options::skip_permission_denied))
    {
        Utils::Log(entry.path().string() + "\n");

        // ディレクトリであれば、再帰的に呼び出してその中も読み込む
        if (std::filesystem::is_directory(entry.path()))
        {
            LoadFilesRecursively(entry.path().string());
        }

        // ファイルがjsonでない場合はスキップ
        if (entry.path().extension() != ".json")
        {
            // parameter以降をsceneNameとして取得
            std::string path = entry.path().string();
            size_t pos = path.find("Parameter");
            if (pos == std::string::npos||
                pos + 10 >= path.length())
                continue;
            std::string sceneName = path.substr(pos + 10);
            if (sceneName .empty())
                continue;
            sceneName = sceneName.substr(0, sceneName.find_last_of("/"));

            // Configを作成
            Create(sceneName);
            configs_[sceneName]->SetDirectoryPath(path);
            sceneName_ = sceneName;

            continue;
        }

        // ファイル名を取得（拡張子を除去）
        std::string gName = entry.path().stem().string();

        // ファイルパスを作成
        std::string path = _directoryPath + "/" + gName;

        // JsonLoaderに読み込みを依頼
        json_->LoadJson(path);

        // グループ名リストにファイル名を追加
        groupNames_.push_back(gName);

        value_[sceneName_][gName] = {};

        Utils::Log("         load comp " + entry.path().string() + "\n");
    }
}
