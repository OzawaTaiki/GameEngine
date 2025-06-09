#pragma once

#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>
#include <Math/Quaternion/Quaternion.h>
#include <Debug/Debug.h>
#include <string>
#include <vector>
#include <variant>

#include "json.hpp"

using json = nlohmann::json;

using ValueVariant = std::variant<int32_t, uint32_t, float, Vector2, Vector3, Vector4,Quaternion, std::string>;
class JsonLoader {

public:

    JsonLoader(bool _autoSave = true);
    JsonLoader(const std::string& _directory, bool _autoSave = true);
    ~JsonLoader();

    /// <summary>
    /// Jsonファイルを読み込む
    /// </summary>
    static json LoadFile(const std::string& _filepath);


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
    void SetValue(std::string _gName, std::string _vName, const T& _v);

    template<typename T>
    void SetValue(std::string _gName, std::string _vName, std::vector<T> _v);

    template<typename T>
    void SetValue(std::string _gName, std::string _vName, std::list<T> _v);

    template<typename T>
    void SetJsonValue(const std::string& _gName, const std::string& _vName, const T& _v);

    template<typename T>
    void SetJsonValue(const std::string& _gName, const std::string& _vName, const std::vector<T>& _v);

    template<typename T>
    void SetJsonValue(const std::string& _gName, const std::string& _vName, const std::list<T>& _v);

private:
    json jsonData_;

    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<ValueVariant>>> values_;

    std::vector<ValueVariant> FromJson(const json& _data);
    json ToJson(const std::vector<ValueVariant>& _data);

    std::string folderPath_;
    std::string filePath_;
    bool autoSave_;
};

template<typename T>
void JsonLoader::GetValue(std::string _gName, std::string _vName, T& _v)
{
    if (!values_.contains(_gName) && !jsonData_.contains(_gName))
        return;

    if ((!values_[_gName].contains(_vName) ||
        values_[_gName][_vName].empty()) &&
        (!jsonData_[_gName].contains(_vName) ||
        jsonData_[_gName][_vName].empty()))
    {
        return;
    }
    std::string name = typeid (T).name();
    if constexpr (std::is_same_v<T, int32_t> ||
                  std::is_same_v<T, uint32_t>)
    {
        _v = static_cast<T>(std::get<int32_t>(values_[_gName][_vName][0]));
    }
    else if constexpr (std::is_same_v<T, float> ||
                       std::is_same_v<T, Vector2> ||
                       std::is_same_v<T, Vector3> ||
                       std::is_same_v<T, Vector4> ||
                       std::is_same_v<T, Quaternion> ||
                       std::is_same_v<T, std::string>)
    {
        _v = std::get<T>(values_[_gName][_vName][0]);
    }
    else
    {
        //jsonData_[_gName][_vName].get_to(_v);
        _v = jsonData_[_gName][_vName].get<T>();
    }


}

template<typename T>
void JsonLoader::GetValue(std::string _gName, std::string _vName, std::vector<T>& _v)
{
    if (!values_[_gName].contains(_vName))
        return;
    std::string name = typeid (T).name();
    if constexpr (std::is_same_v<T, int32_t> ||
                  std::is_same_v<T, uint32_t>)
    {
        std::vector<T> vec;
        for (auto& v : values_[_gName][_vName])
        {
            vec.push_back(static_cast<T>(std::get<int32_t>(v)));
        }
        _v = vec;
    }
    else if constexpr (std::is_same_v<T, float> ||
                       std::is_same_v<T, Vector2> ||
                       std::is_same_v<T, Vector3> ||
                       std::is_same_v<T, Vector4> ||
                        std::is_same_v<T, Quaternion> ||
                        std::is_same_v<T, std::string>)
    {
        std::vector<T> vec;
        for (auto& v : values_[_gName][_vName])
        {
            vec.push_back(std::get<T>(v));
        }
        _v = vec;
    }
    else
    {
        _v = jsonData_[_gName][_vName].get<std::vector<T>>();
    }
}

template<typename T>
void JsonLoader::GetValue(std::string _gName, std::string _vName, std::list<T>& _v)
{
    if (!values_[_gName].contains(_vName))
        return;
    std::string name = typeid (T).name();
    if constexpr (std::is_same_v<T, int32_t> ||
                  std::is_same_v<T, uint32_t>)
    {
        std::list<T> list;
        for (auto& v : values_[_gName][_vName])
        {
            list.push_back(static_cast<T>(std::get<int32_t>(v)));
        }
        _v = list;
    }
    else if constexpr (std::is_same_v<T, float> ||
                       std::is_same_v<T, Vector2> ||
                       std::is_same_v<T, Vector3> ||
                       std::is_same_v<T, Vector4> ||
                        std::is_same_v<T, Quaternion> ||
                        std::is_same_v<T, std::string>)
    {
        std::list<T> list;
        for (auto& v : values_[_gName][_vName])
        {
            list.push_back(std::get<T>(v));
        }
        _v = list;
    }
    else
    {
        _v = jsonData_[_gName][_vName].get<std::list<T>>();
    }


}

template<typename T>
void JsonLoader::SetValue(std::string _gName, std::string _vName, const T& _v)
{
    std::string name = typeid (T).name();

    if (values_[_gName].contains(_vName))
    {
        values_[_gName][_vName].clear();
        values_[_gName][_vName].push_back(_v);

        jsonData_[_gName][_vName].clear();
        jsonData_[_gName][_vName] = ToJson(values_[_gName][_vName]);
    }
    else
        jsonData_[_gName][_vName] = _v;
}

template<typename T>
void JsonLoader::SetValue(std::string _gName, std::string _vName, std::vector<T> _v)
{
    std::string name = typeid (T).name();

    if (values_[_gName].contains(_vName))
    {
        values_[_gName][_vName].clear();
        for (auto& v : _v)
        {
            values_[_gName][_vName].push_back(v);
        }

        jsonData_[_gName][_vName].clear();
        jsonData_[_gName][_vName] = ToJson(values_[_gName][_vName]);
    }
    else
        jsonData_[_gName][_vName] = _v;
}

template<typename T>
void JsonLoader::SetValue(std::string _gName, std::string _vName, std::list<T> _v)
{
    std::string name = typeid (T).name();
    if (values_[_gName].contains(_vName))
    {
        values_[_gName][_vName].clear();
        for (auto& v : _v)
        {
            values_[_gName][_vName].push_back(v);
        }

        jsonData_[_gName][_vName].clear();
        jsonData_[_gName][_vName] = ToJson(values_[_gName][_vName]);
    }
    else
        jsonData_[_gName][_vName] = _v;

}

template<typename T>
void JsonLoader::SetJsonValue(const std::string& _gName, const std::string& _vName, const T& _v)
{
    jsonData_[_gName][_vName] = _v;
    json j;
    j[_gName][_vName] = _v;
}

template<typename T>
void JsonLoader::SetJsonValue(const std::string& _gName, const std::string& _vName, const std::vector<T>& _v)
{
    jsonData_[_gName][_vName] = _v;
}

template<typename T>
void JsonLoader::SetJsonValue(const std::string& _gName, const std::string& _vName, const std::list<T>& _v)
{
    jsonData_[_gName][_vName] = _v;
}
