#pragma once

#include <Physics/Math/Vector2.h>
#include <Physics/Math/Vector3.h>
#include <Physics/Math/Vector4.h>
#include <Systems/Utility/Debug.h>
#include <string>
#include <vector>
#include <variant>

#include "json.hpp"

using json = nlohmann::json;

using ValueVariant = std::variant<int32_t, uint32_t, float, Vector2, Vector3, Vector4, std::string>;
class JsonLoader {

public:

    JsonLoader(bool _autoSave = true);
    JsonLoader(const std::string& _directory, bool _autoSave = true);
    ~JsonLoader();

    void LoadJson(const std::string& _filepath, bool _isMakeFile = false);
    void MakeJsonFile()const;

    void SetFolderPath(const std::string& _directory) { folderPath_ = _directory; }

    template<typename T>
    void GetValue(std::string _gName, std::string _vName, T& _v);
    template<typename T>
    void GetValue(std::string _gName, std::string _vName, std::vector<T>& _v);
    template<typename T>
    void GetValue(std::string _gName, std::string _vName, std::list<T>& _v);

    void OutPutJsonFile(const std::string& _groupName);

    template<typename T>
    inline void SetValue(std::string _gName, std::string _vName, T _v);

    template<typename T>
    inline void SetValue(std::string _gName, std::string _vName, std::vector<T> _v);

    template<typename T>
    inline void SetValue(std::string _gName, std::string _vName, std::list<T> _v);


private:
    json jsonData_;

    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<ValueVariant>>> values_;

    ValueVariant FromJson(const json& _data);
    json ToJson(const std::vector<ValueVariant>& _data);


    std::string folderPath_;
    std::string filePath_;
    bool autoSave_;
};

template<typename T>
inline void JsonLoader::GetValue(std::string _gName, std::string _vName, T& _v)
{
    if (jsonData_[_gName].contains(_vName))
    {
        if constexpr (std::is_same_v<T, int32_t>)
        {
            _v = static_cast<int32_t> (std::get<int32_t>(values_[_gName][_vName][0]));
        }

        else if constexpr (std::is_same_v<T, uint32_t>)
        {
            _v = static_cast<uint32_t> (std::get<int32_t>(values_[_gName][_vName][0]));
        }

        else
            _v = std::get<T>(values_[_gName][_vName][0]);
    }
}

template<typename T>
inline void JsonLoader::GetValue(std::string _gName, std::string _vName, std::vector<T>& _v)
{
    if (jsonData_[_gName].contains(_vName))
    {
        if constexpr (std::is_same_v<T, int32_t> ||
                      std::is_same_v<T, uint32_t>)
        {
            values_[_gName][_vName].clear();

            for (auto& v : jsonData_[_gName][_vName])
            {
                T val = v.get<T>();
                values_[_gName][_vName].push_back(val);
                _v.push_back(val);
            }
        }

        for (auto& v : values_[_gName][_vName])
        {
            _v.push_back(std::get<T>(v));
        }
    }
}

template<typename T>
inline void JsonLoader::GetValue(std::string _gName, std::string _vName, std::list<T>& _v)
{
    if (jsonData_[_gName].contains(_vName))
    {
        if constexpr (std::is_same_v<T, int32_t> ||
                      std::is_same_v<T, uint32_t>)
        {
            values_[_gName][_vName].clear();
            for (auto& v : jsonData_[_gName][_vName])
            {
                T val = v.get<T>();
                values_[_gName][_vName].push_back(val);
                _v.push_back(val);
            }
        }

        for (auto& v : values_[_gName][_vName])
        {
            _v.push_back(std::get<T>(v));
        }
    }
}

template<typename T>
inline void JsonLoader::SetValue(std::string _gName, std::string _vName, T _v)
{
    values_[_gName][_vName].clear();
    values_[_gName][_vName].push_back(_v);

    jsonData_[_gName][_vName].clear();
    jsonData_[_gName][_vName]=ToJson(values_[_gName][_vName]);
}

template<typename T>
inline void JsonLoader::SetValue(std::string _gName, std::string _vName, std::vector<T> _v)
{
    values_[_gName][_vName].clear();
    for (auto& v : _v)
        values_[_gName][_vName].push_back(v);

    jsonData_[_gName][_vName].clear();
    jsonData_[_gName][_vName] = ToJson(values_[_gName][_vName]);
}

template<typename T>
inline void JsonLoader::SetValue(std::string _gName, std::string _vName, std::list<T> _v)
{
    values_[_gName][_vName].clear();
    for (auto& v : _v)
        values_[_gName][_vName].push_back(v);

    jsonData_[_gName][_vName].clear();
    jsonData_[_gName][_vName] = ToJson(values_[_gName][_vName]);

}
