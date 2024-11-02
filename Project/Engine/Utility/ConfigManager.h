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

    template<typename T>
    void SetVariable(const std::string& _groupName, const std::string& _variableName, T* _variablePtr);


private:

    JsonLoader* json_;
    std::vector<std::string> groupNames_;
    std::string directoryPath_ = "resources/Data/Parameter";

    struct Type
    {
        std::variant<uint32_t*, float*, Vector2*, Vector3*, Vector4*> variable;
    };

    std::unordered_map<std::string, std::unordered_map<std::string, Type>> data_;

    ConfigManager() = default;
    ~ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
};

template<typename T>
inline void ConfigManager::SetVariable(const std::string& _groupName, const std::string& _variableName, T* _variablePtr)
{
    if(data_.contains(_groupName))
    {
        if (data_[_groupName].contains(_variableName))
        {
            if constexpr (std::is_same<T, uint32_t>::value)
            {
                *_variablePtr = *std::get<uint32_t*>(data_[_groupName][_variableName].variable);
                delete std::get<uint32_t*>(data_[_groupName][_variableName].variable);
                data_[_groupName][_variableName].variable = _variablePtr;
            }
            else if constexpr (std::is_same<T, float>::value)
            {
                *_variablePtr = *std::get<float*>(data_[_groupName][_variableName].variable);
                delete std::get<float*>(data_[_groupName][_variableName].variable);
                data_[_groupName][_variableName].variable = _variablePtr;
            }
            else if constexpr (std::is_same<T, Vector2>::value)
            {
                *_variablePtr = *std::get<Vector2*>(data_[_groupName][_variableName].variable);
                delete std::get<Vector2*>(data_[_groupName][_variableName].variable);
                data_[_groupName][_variableName].variable = _variablePtr;
            }
            else if constexpr (std::is_same<T, Vector3>::value)
            {
                *_variablePtr = *std::get<Vector3*>(data_[_groupName][_variableName].variable);
                delete std::get<Vector3*>(data_[_groupName][_variableName].variable);
                data_[_groupName][_variableName].variable = _variablePtr;
            }
            else if constexpr (std::is_same<T, Vector4>::value)
            {
                *_variablePtr = *std::get<Vector4*>(data_[_groupName][_variableName].variable);
                delete std::get<Vector4*>(data_[_groupName][_variableName].variable);
                data_[_groupName][_variableName].variable = _variablePtr;
            }
        }
        else
        {
            data_[_groupName][_variableName] = Type();
            data_[_groupName][_variableName].variable = _variablePtr;
        }
    }
    else
    {
        data_[_groupName] = std::unordered_map<std::string, Type>();
        data_[_groupName][_variableName].variable = _variablePtr;
    }


}
