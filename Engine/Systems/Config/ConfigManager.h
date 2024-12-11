#pragma once

#include <Physics/Math/Vector2.h>
#include <Physics/Math/Vector3.h>
#include <Physics/Math/Vector4.h>

#include <variant>
#include <cstdint>
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <unordered_map>
#include <optional>

template<typename T>
using RefVector = std::reference_wrapper<std::vector<T>>;

class JsonLoader;
class Config;
class ConfigManager
{
public: // 構造体

    // クラス外で使用
    struct VariableAddress
    {
        std::variant<uint32_t*, float*, Vector2*, Vector3*, Vector4*, std::string*,
            RefVector<uint32_t>, RefVector<float>,
            RefVector<Vector2>, RefVector<Vector3>,
            RefVector<Vector4>, RefVector<std::string>,
            std::list<uint32_t>, std::list<float>,
            std::list<Vector2>, std::list<Vector3>,
            std::list<Vector4>, std::list<std::string>
        > address;
    };

    struct VariableValue
    {
        std::variant<uint32_t, float, Vector2, Vector3, Vector4, std::string,
            std::vector<uint32_t>, std::vector<float>,
            std::vector<Vector2>, std::vector<Vector3>,
            std::vector<Vector4>, std::vector<std::string>,
            std::list<uint32_t>, std::list<float>,
            std::list<Vector2>, std::list<Vector3>,
            std::list<Vector4>, std::list<std::string>
        > variable;
    };

public:

    static ConfigManager* GetInstance();

    void Initialize();

    void LoadRootDirectory();

    void LoadData();
    void SaveData(const std::string& _groupName);

    template<typename T>
    inline void GetVariableValue(const std::string& _groupName, const std::string& _variableName,T*& _ptr);

    template<typename T>
    inline void GetVariableValue(const std::string& _groupName, const std::string& _variableName,std::vector<T>*& _ptr);

    template<typename T>
    inline void GetVariableValue(const std::string& _groupName, const std::string& _variableName, std::list<T>*& _ptr);


    void EraseData(const std::string& _groupName, const std::string& _variableName);

    void SetDirectoryPath(const std::string& _directoryPath);

private:


    JsonLoader* json_;
    std::vector<std::string> groupNames_;
    std::string rootDirectory_ = "resources/Data";
    std::string directoryPath_ = "resources/Data/Parameter";


    std::unordered_map<std::string, std::unordered_map<std::string, VariableValue>> value_;

    // ディレクトリ内のファイルを再帰的に読み込む
    void LoadFilesRecursively(const std::string& _directoryPath);

    ConfigManager() = default;
    ~ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
};

template<typename T>
inline void ConfigManager::GetVariableValue(const std::string& _groupName, const std::string& _variableName, T*& _ptr)
{
    if (!value_.contains(_groupName) || !value_[_groupName].contains(_variableName))
    {
        // グループ名 変数名 が存在しない場合は新しく作成
        value_[_groupName][_variableName].variable = T();
    }

    _ptr = &std::get<T>(value_[_groupName][_variableName].variable);

}

template<typename T>
inline void ConfigManager::GetVariableValue(const std::string& _groupName, const std::string& _variableName, std::vector<T>*& _ptr)
{
    if (!value_.contains(_groupName) || !value_[_groupName].contains(_variableName))
    {
        // グループ名 変数名 が存在しない場合は新しく作成
        value_[_groupName][_variableName].variable = std::vector<T>();
    }

    _ptr = &std::get<std::vector<T>>(value_[_groupName][_variableName].variable);

}

template<typename T>
inline void ConfigManager::GetVariableValue(const std::string& _groupName, const std::string& _variableName, std::list<T>*& _ptr)
{
    if (!value_.contains(_groupName) || !value_[_groupName].contains(_variableName))
    {
        // グループ名 変数名 が存在しない場合は新しく作成
        value_[_groupName][_variableName].variable = std::list<T>();
    }

    _ptr = &std::get<std::list<T>>(value_[_groupName][_variableName].variable);
}
