#pragma once

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

#include <variant>
#include <cstdint>
#include <string>
#include <unordered_map>

class JsonLoader;
class ConfigManager
{
public:

    static ConfigManager* GetInstance();

    void Initialize();
    void Update();
    void Draw();

    void LoadData();
    void SaveData();
    void SaveData(const std::string& _groupName);

    template<typename T>
    void SetVariable(const std::string& _groupName, const std::string& _variableName, T* _variablePtr);


private:

    JsonLoader* json_;
    std::vector<std::string> groupNames_;
    std::string directoryPath_ = "resources/Data/Parameter";

    struct Type
    {
        std::variant<uint32_t*, float*, Vector2*, Vector3*, Vector4*, std::string*> address;
    };
    struct Type2
    {
        std::variant<uint32_t, float, Vector2, Vector3, Vector4, std::string> variable;
    };

    std::unordered_map<std::string, std::unordered_map<std::string, Type>> ptr_;
    std::unordered_map<std::string, std::unordered_map<std::string, Type2>> value_;


    ConfigManager() = default;
    ~ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
};

template<typename T>
inline void ConfigManager::SetVariable(const std::string& _groupName, const std::string& _variableName, T* _variablePtr)
{
    if(value_.contains(_groupName))
    {
        if (value_[_groupName].contains(_variableName))
        {
            if constexpr (std::is_same<T, uint32_t>::value)
            {
                *_variablePtr = std::get<uint32_t>(value_[_groupName][_variableName].variable);
                ptr_[_groupName][_variableName].address = _variablePtr;
            }
            else if constexpr (std::is_same<T, float>::value)
            {
                *_variablePtr = std::get<float>(value_[_groupName][_variableName].variable);
                ptr_[_groupName][_variableName].address = _variablePtr;
            }
            else if constexpr (std::is_same<T, Vector2>::value)
            {
                *_variablePtr = std::get<Vector2>(value_[_groupName][_variableName].variable);
                ptr_[_groupName][_variableName].address = _variablePtr;
            }
            else if constexpr (std::is_same<T, Vector3>::value)
            {
                *_variablePtr = std::get<Vector3>(value_[_groupName][_variableName].variable);
                ptr_[_groupName][_variableName].address = _variablePtr;
            }
            else if constexpr (std::is_same<T, Vector4>::value)
            {
                *_variablePtr = std::get<Vector4>(value_[_groupName][_variableName].variable);
                ptr_[_groupName][_variableName].address = _variablePtr;
            }
            else if constexpr (std::is_same<T, std::string>::value)
            {
                *_variablePtr = std::get<std::string>(value_[_groupName][_variableName].variable);
                ptr_[_groupName][_variableName].address = _variablePtr;
            }
        }
        else
        {
            ptr_[_groupName][_variableName] = Type();
            ptr_[_groupName][_variableName].address = _variablePtr;
        }
    }
    else
    {
        ptr_[_groupName] = std::unordered_map<std::string, Type>();
        ptr_[_groupName][_variableName].address = _variablePtr;
    }


}
