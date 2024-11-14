#include "JsonLoader.h"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <sstream>

JsonLoader::JsonLoader() {
    dataGroup_.clear();
    folderPath_ = "Resources/Data";
    autoSave_ = true;
}

JsonLoader::JsonLoader(const std::string& _directory, bool _autoSave) {
    dataGroup_.clear();
    folderPath_ = _directory;
    autoSave_ = _autoSave;
}

JsonLoader::~JsonLoader() {
    if(autoSave_)
        SaveJson();
}

void JsonLoader::LoadJson(const std::string& _groupName, bool _isMakeFile) {

    filePath_ = _groupName + ".json";
    std::ifstream inputFile(filePath_);
    if (!inputFile.is_open())
    {
        if (_isMakeFile)
            MakeJsonFile();
        else
            assert(inputFile.is_open() && "Cant Open inputFile");
    }
    else
    {
        if (inputFile.peek() != std::ifstream::traits_type::eof())
        {
            inputFile >> jsonData_;
            for (const auto& [groupName, dataObj] : jsonData_.items()) {
                Data data;
                for (const auto& [key, values] : dataObj.items()) {
                    for (const auto& value : values)
                    {
                        data.data[key].push_back(parseDatum(value, _groupName));
                    }
                }
                dataGroup_[groupName] = data;
            }
        }
        inputFile.close();
    }
}

void JsonLoader::SaveJson() {
    std::ofstream outputFile(filePath_);
    assert(outputFile.is_open() && "Cant Open OutputFile");
    jsonData_.clear();
    // objでfor
    for (const auto& [groupName, dataObj] : dataGroup_)
    {
        json data;
        for (const auto& [key, values] : dataObj.data)
        {
            for (const auto& value : values)
                data[key].push_back(DatumToJson(value));
        }
        jsonData_[groupName] = data;
    }

    outputFile << jsonData_.dump(4);
    outputFile.close();
}

void JsonLoader::MakeJsonFile() {

    std::filesystem::create_directories(folderPath_);
    std::ofstream outputFile(filePath_);
    outputFile.close();
}

void JsonLoader::SaveJson(const std::string& _groupName)
{
    // ファイル名を取得
    std::string fileapath = folderPath_ + _groupName + ".json";
    // ファイルを開く
    std::ofstream outputFile(fileapath);

    assert(outputFile.is_open() && "Cant Open OutputFile");
    // データをクリア
    json j;

    // データを追加
    for (const auto& [key, values] : dataGroup_[_groupName].data)
    {
        for (const auto& value : values)
            j[_groupName][key].push_back(DatumToJson(value));
    }
    // ファイルに書き込み
    outputFile << j.dump(4);
    outputFile.close();
}

void JsonLoader::SetData(const std::string& _groupname, const std::string& _name, const Datum& _data, bool _isOverride)
{
    //if (dataGroup_[_groupname].data[_name].empty())

    if(_isOverride)
    {
        dataGroup_[_groupname].data[_name].clear();
        dataGroup_[_groupname].data[_name].push_back(_data);
    }
    else
        dataGroup_[_groupname].data[_name].push_back(_data);
}

JsonLoader::Datum JsonLoader::parseDatum(const json& j, const std::string& _groupName)
{
    Datum d;
    if (j.is_number_unsigned())
    {
        d.datum = j.get<uint32_t>();
    }
    else if (j.is_number_float())
    {
        d.datum = j.get<float>();
    }
    else if (j.is_array() && j.size() == 2 || j.contains("x") && j.contains("y"))
    {
        d.datum=Vector2(j[1].get<float>(), j[1].get<float>());
    }
    else if (j.is_array() && j.size() == 3 || j.contains("x") && j.contains("y") && j.contains("z"))
    {
        d.datum = Vector3(j[0].get<float>(), j[1].get<float>(), j[2].get<float>());
    }
    else if (j.is_array() && j.size() == 4 || j.contains("x") && j.contains("y") && j.contains("z") && j.contains("w"))
    {
        d.datum = Vector4(j[0].get<float>(), j[1].get<float>(), j[2].get<float>(), j[3].get<float>());
    }
    else
    {
        throw std::runtime_error("Unsupported data format");
    }
    return d;
}

json JsonLoader::DatumToJson(const Datum& _datum)
{
    nlohmann::json j = json::array(); // 複数の datum を扱うために array として初期化

    // variant に応じた処理を行う
    std::visit([&j](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, uint32_t>)
        {
            j = arg; // uint32_t の場合
        }
        else if constexpr (std::is_same_v<T, float>)
        {
            j = arg; // float の場合
        }
        else if constexpr (std::is_same_v<T, Vector2>)
        {
            j = { arg.x,arg.y };
        }
        else if constexpr (std::is_same_v<T, Vector3>)
        {
            j = { arg.x, arg.y,arg.z };
        }
        else if constexpr (std::is_same_v<T, Vector4>)
        {
            j = { arg.x, arg.y,arg.z,arg.w };
        }
               }, _datum.datum);
    return j;
}

std::optional<JsonLoader::Data> JsonLoader::GetData(const std::string& _groupName)
{
    // オブジェクトのデータを取得
    auto it = dataGroup_.find(_groupName);
    if (it != dataGroup_.end())
    {
        return it->second;
    }
    return std::nullopt;
}


void JsonLoader::PrepareForSave() { jsonData_.clear(); }
