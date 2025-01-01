#pragma once

#include <Physics/Math/Vector2.h>
#include <Physics/Math/Vector3.h>
#include <Physics/Math/Vector4.h>
#include <Systems/Utility/JsonLoader.h>

#include <variant>
#include <cstdint>
#include <string>
#include <vector>
#include <list>
#include <memory>

#include <unordered_map>


template<typename T>
using RefVector = std::reference_wrapper<std::vector<T>>;

// BinderとJsonを中継するシングルトンクラス
class JsonHub
{
public:
    struct VariableAddress
    {
        std::variant<int32_t*,uint32_t*, float*, Vector2*, Vector3*, Vector4*, std::string*,
            RefVector<int32_t>, RefVector<uint32_t>,
            RefVector<float>,
            RefVector<Vector2>, RefVector<Vector3>,
            RefVector<Vector4>, RefVector<std::string>,
            std::list<int32_t>*, std::list<uint32_t>*,
            std::list<float>*,
            std::list<Vector2>*, std::list<Vector3>*,
            std::list<Vector4>*, std::list<std::string>*
        > address;
    };

    struct VariableValue
    {
        std::variant<int32_t,uint32_t, float, Vector2, Vector3, Vector4, std::string,
            std::vector<int32_t>, std::vector<uint32_t>,
            std::vector<float>,
            std::vector<Vector2>, std::vector<Vector3>,
            std::vector<Vector4>, std::vector<std::string>,
            std::list<int32_t>, std::list<uint32_t>,
            std::list<float>,
            std::list<Vector2>, std::list<Vector3>,
            std::list<Vector4>, std::list<std::string>
        > variable;
    };

    static JsonHub* GetInstance();

    void Initialize(const std::string& _rootDirectory = "Resources/Data/");
    void Save(const std::string& _groupName);


    template<typename T>
    inline void GetVariableValue(const std::string& _groupName, const std::string& _variableName, T& _ptr);

    template<typename T>
    inline void GetVariableValue(const std::string& _groupName, const std::string& _variableName, std::vector<T>& _ptr);

    template<typename T>
    inline void GetVariableValue(const std::string& _groupName, const std::string& _variableName, std::list<T>& _ptr);

    template<typename T>
    inline void RegisterVariable(const std::string& _groupName, const std::string& _variableName, T _ptr);

    template<typename T>
    inline void RegisterVariable(const std::string& _groupName, const std::string& _variableName, std::vector<T> _ptr);

    template<typename T>
    inline void RegisterVariable(const std::string& _groupName, const std::string& _variableName, std::list<T> _ptr);

    template<typename T>
    void SetJsonValue(const std::string& _groupName, const std::string& _variableName, const T& _value);

    template<typename T>
    void SetJsonValue(const std::string& _groupName, const std::string& _variableName, const std::vector<T>& _value);

    template<typename T>
    void SetJsonValue(const std::string& _groupName, const std::string& _variableName, const std::list<T>& _value);

    void SetRootDirectoryPath(const std::string& _rootDirectoryPath) { rootDirectoryPath_ = _rootDirectoryPath; }
    void SetDirectoryPathFromRoot(const std::string& _directoryPathFromRoot) { jsonLoader_->SetFolderPath(_directoryPathFromRoot); }

private:

    void LoadRootDirectory();
    void LoadFilesRecursively(const std::string& _directoryPath);

    std::unique_ptr<JsonLoader> jsonLoader_ = nullptr;

    std::string rootDirectoryPath_ = "Resources/Data";
    std::string directoryPathFromRoot_;

    // コピー禁止
    JsonHub() = default;
    ~JsonHub() = default;
    JsonHub(const JsonHub&) = delete;
    JsonHub& operator=(const JsonHub&) = delete;

};

template<typename T>
inline void JsonHub::GetVariableValue(const std::string& _groupName, const std::string& _variableName, T& _ptr)
{
    jsonLoader_->GetValue(_groupName, _variableName, _ptr);
}

template<typename T>
inline void JsonHub::GetVariableValue(const std::string& _groupName, const std::string& _variableName, std::vector<T>& _ptr)
{
    jsonLoader_->GetValue(_groupName, _variableName, _ptr);
}

template<typename T>
inline void JsonHub::GetVariableValue(const std::string& _groupName, const std::string& _variableName, std::list<T>& _ptr)
{
    jsonLoader_->GetValue(_groupName, _variableName, _ptr);
}

template<typename T>
inline void JsonHub::RegisterVariable(const std::string& _groupName, const std::string& _variableName, T _ptr)
{
    jsonLoader_->SetValue(_groupName, _variableName, _ptr);
}

template<typename T>
inline void JsonHub::RegisterVariable(const std::string& _groupName, const std::string& _variableName, std::vector<T> _ptr)
{
    jsonLoader_->SetValue(_groupName, _variableName, _ptr);
}

template<typename T>
inline void JsonHub::RegisterVariable(const std::string& _groupName, const std::string& _variableName, std::list<T> _ptr)
{
    jsonLoader_->SetValue(_groupName, _variableName, _ptr);
}

template<typename T>
inline void JsonHub::SetJsonValue(const std::string& _groupName, const std::string& _variableName, const T& _value)
{
    jsonLoader_->SetJsonValue(_groupName, _variableName, _value);
}

template<typename T>
inline void JsonHub::SetJsonValue(const std::string& _groupName, const std::string& _variableName, const std::vector<T>& _value)
{
    jsonLoader_->SetJsonValue(_groupName, _variableName, _value);
}

template<typename T>
inline void JsonHub::SetJsonValue(const std::string& _groupName, const std::string& _variableName, const std::list<T>& _value)
{
    jsonLoader_->SetJsonValue(_groupName, _variableName, _value);
}
