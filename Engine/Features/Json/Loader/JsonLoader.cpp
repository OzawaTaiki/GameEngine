#include <Features/Json/Loader/JsonLoader.h>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <sstream>

JsonLoader::JsonLoader(bool _autoSave) {
    folderPath_ = "Resources/Data/";
    autoSave_ = _autoSave;
}

JsonLoader::JsonLoader(const std::string& _directory, bool _autoSave) {
    folderPath_ = _directory;
    autoSave_ = _autoSave;

    // folderPath_のさいごに'\\'がない場合は追加
    if (folderPath_.back() != '\\')
        folderPath_ += "\\";
}

JsonLoader::~JsonLoader() {
}

void JsonLoader::LoadJson(const std::string& _filepath, bool _isMakeFile) {

    // 拡張子の有無
    if (_filepath.find(".json") == std::string::npos)
        filePath_ = _filepath + ".json";
    else
        filePath_ = _filepath;

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
        //Debug::Log("Begin Load :" + _filepath + "\n");

        if (inputFile.peek() != std::ifstream::traits_type::eof())
        {
            // 名前を取得
            json j;
            inputFile >> j;

            // groupName
            for (const auto& [groupName, dataObj] : j.items()) {
                jsonData_[groupName] = dataObj;
                // variableName
                for (const auto& [key, values] : dataObj.items())
                {
                    //for (const auto& val : values)
                        values_[groupName][key] = FromJson(values);
                        //values_[groupName][key].push_back(0);
                }
            }
        }
        inputFile.close();
        //Debug::Log("End Load   :" + filePath_ + "\n");

    }
}

void JsonLoader::MakeJsonFile() const {

    std::filesystem::create_directories(folderPath_);
    std::ofstream outputFile(filePath_);
    outputFile.close();
}

void JsonLoader::OutPutJsonFile(const std::string& _groupName)
{
    // /がない場合はフォルダパスを追加
    if (_groupName.find('/') == std::string::npos)
        filePath_ =  folderPath_+ _groupName + ".json";
    else
        filePath_ = _groupName;

    // 拡張子の有無
    if (filePath_.find(".json") == std::string::npos)
        filePath_ = filePath_ + ".json";

    std::filesystem::path path(folderPath_);
    if (!std::filesystem::exists(path))
        std::filesystem::create_directories(path);

    // ファイルを開く
    std::ofstream outputFile(filePath_);

    json j;
    j[_groupName] = jsonData_[_groupName];

    /*for (const auto& [key, values] : jsonData_[_groupName].items())
    {
        for (const auto& value : values)
                j[_groupName][key].emplace_back(value);
    }*/

    outputFile << j.dump(4);
    outputFile.close();

}

std::vector<ValueVariant> JsonLoader::FromJson(const json& _j)
{
    // valuesを受け取る
    // これはたいてい配列 vectorで返す
    std::vector<ValueVariant> vec;

    if (_j.is_number_integer())
    {
        vec.push_back(_j.get<int32_t>());
    }
    else if (_j.is_number_float())
    {
        vec.push_back(_j.get<float>());
    }
    else if (_j.is_string())
    {
        vec.push_back(_j.get<std::string>());
    }
    else if (_j.is_object() && _j.size() == 2 && _j.contains("x") && _j.contains("y"))
    {
        vec.push_back(Vector2(_j["x"].get<float>(), _j["y"].get<float>()));
    }
    else if (_j.is_object() && _j.size() == 3 && _j.contains("x") && _j.contains("y") && _j.contains("z"))
    {
        vec.push_back(Vector3(_j["x"].get<float>(), _j["y"].get<float>(), _j["z"].get<float>()));
    }
    else if (_j.is_object() && _j.size() == 4 && _j.contains("x") && _j.contains("y") && _j.contains("z") && _j.contains("w"))
    {
        vec.push_back(Vector4(_j["x"].get<float>(), _j["y"].get<float>(), _j["z"].get<float>(), _j["w"].get<float>()));
    }
    else if (_j.is_array())
    {
        for (auto j : _j)
        {
            if (j.is_number_integer())
            {
                vec.push_back(j.get<int32_t>());
            }
            else if (j.is_number_float())
            {
                vec.push_back(j.get<float>());
            }
            else if (j.is_string())
            {
                vec.push_back(j.get<std::string>());
            }
        }
    }

    return vec;
}

json JsonLoader::ToJson(const std::vector<ValueVariant>& _data)
{
    json j;

    for (auto v : _data)
    {
        switch (v.index())
        {
        case 0: // int32_t
            j.push_back(std::get<int32_t>(v));
            break;
        case 1: // uint32_t
            j.push_back(std::get<uint32_t>(v));
            break;
        case 2: // float
            j.push_back(std::get<float>(v));
            break;
        case 3: // Vector2
            {
                Vector2 vec = std::get<Vector2>(v);
                j.push_back({ vec.x,vec.y });
            }
            break;
        case 4: // Vector3
            {
                Vector3 vec3 = std::get<Vector3>(v);
                j.push_back({ vec3.x,vec3.y,vec3.z });
            }
            break;
        case 5: // Vector4
            {
                Vector4 vec4 = std::get<Vector4>(v);
                j.push_back({ vec4.x,vec4.y,vec4.z,vec4.w });
            }
            break;
        case 6: // std::string
            j.push_back(std::get<std::string>(v));
            break;
        default:
            break;
        }

        /*if constexpr (v.index() == 0)
        {
            j.push_back(std::get<int32_t>(v));
        }
        else if constexpr (std::is_same_v<T, float>)
        {
            j.push_back(std::get<float>(v));
        }
        else if constexpr (std::is_same_v<T, Vector2>)
        {
            Vector2 vec = std::get<Vector2>(v);
            j.push_back({ vec.x,vec.y });
        }
        else if constexpr (std::is_same_v<T, Vector3>)
        {
            Vector3 vec = std::get<Vector3>(v);
            j.push_back({ vec.x,vec.y,vec.z });
        }
        else if constexpr (std::is_same_v<T, Vector4>)
        {
            Vector4 vec = std::get<Vector4>(v);
            j.push_back({ vec.x,vec.y,vec.z,vec.w });
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
            j.push_back(std::get<std::string>(v));
        }*/
    }

    return j;
}

