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
    inline void SetVariable(const std::string& _groupName, const std::string& _variableName, T* _variablePtr);
    template<typename T>
    inline void SetVariable(const std::string& _groupName, const std::string& _variableName, std::vector<T>* _variablePtr);


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
        std::variant<uint32_t*, float*, Vector2*, Vector3*, Vector4*, std::string*,
            std::vector<uint32_t>*, std::vector<float>*,
            std::vector<Vector2>*, std::vector<Vector3>*,
            std::vector<Vector4>*, std::vector<std::string>*
        > address;
    };
    struct Type2
    {
        std::variant<uint32_t, float, Vector2, Vector3, Vector4, std::string,
            std::vector<uint32_t>, std::vector<float>,
            std::vector<Vector2>, std::vector<Vector3>,
            std::vector<Vector4>, std::vector<std::string>
        > variable;
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
        return;

    // 既存のグループと変数の確認
    if (value_[sceneName_].contains(_groupName))
    {
        if (value_[sceneName_][_groupName].contains(_variableName))
        {
            // 単一の値を登録
            ptr_[sceneName_][_groupName][_variableName].address = _variablePtr;
            *_variablePtr = std::get<T>(value_[sceneName_][_groupName][_variableName].variable);
        }
        else
        {
            // 新しい変数を登録
            ptr_[sceneName_][_groupName][_variableName].address = _variablePtr;
            value_[sceneName_][_groupName][_variableName].variable = *_variablePtr;
        }
    }
    else
    {
        // 新しいグループを登録
        ptr_[sceneName_][_groupName][_variableName].address = _variablePtr;
        value_[sceneName_][_groupName][_variableName].variable = *_variablePtr;
    }
}

template<typename T>
inline void ConfigManager::SetVariable(const std::string& _groupName, const std::string& _variableName, std::vector<T>* _variablePtr)
{
    if (sceneName_ == "")
        return;

    // 既存のグループと変数の確認
    if (value_[sceneName_].contains(_groupName))
    {
        // 既存のグループがある場合
        if (value_[sceneName_][_groupName].contains(_variableName))
        {
            // ベクタの値を登録
            ptr_[sceneName_][_groupName][_variableName].address = _variablePtr;
            *_variablePtr = std::get<std::vector<T>>(value_[sceneName_][_groupName][_variableName].variable);
        }
        else
        {
            // 新しいベクタ変数を登録
            ptr_[sceneName_][_groupName][_variableName].address = _variablePtr;
            value_[sceneName_][_groupName][_variableName].variable = *_variablePtr;
        }
    }
    else
    {
        // 新しいグループを登録
        ptr_[sceneName_][_groupName][_variableName].address = _variablePtr;
        value_[sceneName_][_groupName][_variableName].variable = *_variablePtr;
    }
}
