#pragma once

#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>

#include <variant>
#include <cstdint>
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <unordered_map>

class JsonLoader;
class Config;
class ConfigManager
{
public:

    static ConfigManager* GetInstance();

    void Initialize();
    void Update();
    void Draw();

    void LoadRootDirectory();

    void LoadData();
    void SaveData();
    void SaveData(const std::string& _groupName);

    template<typename T>
    void SetVariable(const std::string& _groupName, const std::string& _variableName, T* _variablePtr);
    template<typename T>
    void SetVariable(const std::string& _groupName, const std::string& _variableName, std::vector<T>* _variablePtr);

    void SetSceneName(const std::string& _scene);
    void SetDirectoryPath(const std::string& _directoryPath);

    Config* Create(const std::string& _sceneName);

private:

    std::map<std::string, std::unique_ptr<Config>> configs_;
    std::string sceneName_ = "";

    JsonLoader* json_;
    std::vector<std::string> groupNames_;
    std::string rootDirectory_ = "resources/Data";
    std::string directoryPath_ = "resources/Data/Parameter";

    struct Type
    {
        std::vector<std::variant<uint32_t*, float*, Vector2*, Vector3*, Vector4*, std::string*>> address;
    };
    struct Type2
    {
        std::vector<std::variant<uint32_t, float, Vector2, Vector3, Vector4, std::string>> variable;
    };

    std::map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, Type >>> ptr_;
    std::map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, Type2>>> value_;

    // ディレクトリ内のファイルを再帰的に読み込む
    void LoadFilesRecursively(const std::string& _directoryPath);

    ConfigManager() = default;
    ~ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
};

template<typename T>
inline void ConfigManager::SetVariable(const std::string& _groupName, const std::string& _variableName, T* _variablePtr)
{
    if (sceneName_ == "")
    {
        return;
    }

    if(value_[sceneName_].contains(_groupName))
    {
        if (value_[sceneName_][_groupName].contains(_variableName))
        {
            if constexpr (std::is_same<T, uint32_t>::value)
            {
                for (auto& value : value_[sceneName_][_groupName][_variableName].variable)
                {
                    *_variablePtr = std::get<uint32_t>(value);
                    ptr_[sceneName_][_groupName][_variableName].address.emplace_back(_variablePtr);
                }
            }
            else if constexpr (std::is_same<T, float>::value)
            {
                for (auto& value : value_[sceneName_][_groupName][_variableName].variable)
                {
                    *_variablePtr = std::get<float>(value);
                    ptr_[sceneName_][_groupName][_variableName].address.emplace_back(_variablePtr);
                }
            }
            else if constexpr (std::is_same<T, Vector2>::value)
            {
                for (auto& value : value_[sceneName_][_groupName][_variableName].variable)
                {
                    *_variablePtr = std::get<Vector2>(value);
                    ptr_[sceneName_][_groupName][_variableName].address.emplace_back(_variablePtr);
                }
            }
            else if constexpr (std::is_same<T, Vector3>::value)
            {
                for (auto& value : value_[sceneName_][_groupName][_variableName].variable)
                {
                    *_variablePtr = std::get<Vector3>(value);
                    ptr_[sceneName_][_groupName][_variableName].address.emplace_back(_variablePtr);
                }
            }
            else if constexpr (std::is_same<T, Vector4>::value)
            {
                for (auto& value : value_[sceneName_][_groupName][_variableName].variable)
                {
                    *_variablePtr = std::get<Vector4>(value);
                    ptr_[sceneName_][_groupName][_variableName].address.emplace_back(_variablePtr);
                }
            }
            else if constexpr (std::is_same<T, std::string>::value)
            {
                for (auto& value : value_[sceneName_][_groupName][_variableName].variable)
                {
                    *_variablePtr = std::get<std::string>(value);
                    ptr_[sceneName_][_groupName][_variableName].address.emplace_back(_variablePtr);
                }
            }
        }
        else
        {
            ptr_[sceneName_][_groupName][_variableName] = Type();
            ptr_[sceneName_][_groupName][_variableName].address.emplace_back(_variablePtr);

            value_[sceneName_][_groupName][_variableName] = Type2();
            value_[sceneName_][_groupName][_variableName].variable.emplace_back(*_variablePtr);
        }
    }
    else
    {
        ptr_[sceneName_][_groupName] = std::unordered_map<std::string, Type>();
        ptr_[sceneName_][_groupName][_variableName].address.emplace_back(_variablePtr);

        value_[sceneName_][_groupName] = std::unordered_map<std::string, Type2>();
        value_[sceneName_][_groupName][_variableName].variable.emplace_back(*_variablePtr);
    }


}

template<typename T>
inline void ConfigManager::SetVariable(const std::string& _groupName, const std::string& _variableName, std::vector<T>* _variablePtr)
{
    if (sceneName_ == "")
    {
        return;
    }
    if (value_[sceneName_].contains(_groupName))
    {
        if (value_[sceneName_][_groupName].contains(_variableName))
        {
            if constexpr (std::is_same<T, uint32_t>::value)
            {
                *_variablePtr = std::get<std::vector<uint32_t>>(value_[sceneName_][_groupName][_variableName].variable);
                ptr_[sceneName_][_groupName][_variableName].address = _variablePtr;
            }
            else if constexpr (std::is_same<T, float>::value)
            {
                *_variablePtr = std::get<std::vector<float>>(value_[sceneName_][_groupName][_variableName].variable);
                ptr_[sceneName_][_groupName][_variableName].address = _variablePtr;
            }
            else if constexpr (std::is_same<T, Vector2>::value)
            {
                *_variablePtr = std::get<std::vector<Vector2>>(value_[sceneName_][_groupName][_variableName].variable);
                ptr_[sceneName_][_groupName][_variableName].address = _variablePtr;
            }
            else if constexpr (std::is_same<T, Vector3>::value)
            {
                *_variablePtr = std::get<std::vector<Vector3>>(value_[sceneName_][_groupName][_variableName].variable);
                ptr_[sceneName_][_groupName][_variableName].address = _variablePtr;
            }
            else if constexpr (std::is_same<T, Vector4>::value)
            {
                *_variablePtr = std::get<std::vector<Vector4>>(value_[sceneName_][_groupName][_variableName].variable);
                ptr_[sceneName_][_groupName][_variableName].address = _variablePtr;
            }
            else if constexpr (std::is_same<T, std::string>::value)
            {
                *_variablePtr = std::get<std::vector<std::string>>(value_[sceneName_][_groupName][_variableName].variable);
                ptr_[sceneName_][_groupName][_variableName].address = _variablePtr;
            }
        }
        else
        {
            ptr_[sceneName_][_groupName][_variableName] = Type();
            ptr_[sceneName_][_groupName][_variableName].address = _variablePtr;
            value_[sceneName_][_groupName][_variableName] = Type2();
            value_[sceneName_][_groupName][_variableName].variable = *_variablePtr;
        }
    }
    else
    {
        ptr_[sceneName_][_groupName] = std::unordered_map<std::string, Type>();
        ptr_[sceneName_][_groupName][_variableName].address = _variablePtr;

        value_[sceneName_][_groupName] = std::unordered_map<std::string, Type2>();
        value_[sceneName_][_groupName][_variableName].variable = *_variablePtr;
    }
}
