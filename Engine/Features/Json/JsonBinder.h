#pragma once

#include <Features/Json/Loader/JsonLoader.h>
#include <Features/Json/JsonHub.h>

#include <string>
#include <vector>
#include <list>

// 各クラスでインスタンスをもつ
class JsonBinder
{
public:
    JsonBinder(const std::string& _name, const std::string& _folderPath);
    ~JsonBinder() = default;

    void SetFolderPath();
    void Save();

#pragma region 変数登録関数
    // 変数を登録する関数
    template<typename T>
    inline void RegisterVariable(const std::string& _variableName, T* _variablePtr);

    // 変数を登録する関数
    template<typename T>
    inline void RegisterVariable(const std::string& _variableName, std::vector<T>* _variablePtr);

    // 変数を登録する関数
    template<typename T>
    inline void RegisterVariable(const std::string& _variableName, std::list<T>* _variablePtr);

#pragma endregion

#pragma region 変数取得関数
    // データを取得する関数
    template<typename T>
    inline void GetVariableValue(const std::string& _variableName, T& _var);

    // データを取得する関数
    template<typename T>
    inline void GetVariableValue(const std::string& _variableName, std::vector<T>& _var);

    // データを取得する関数
    template<typename T>
    inline void GetVariableValue(const std::string& _variableName, std::list<T>& _var);

#pragma endregion

#pragma region 変数送信関数

    // 登録した変数を送信する関数
    void RegisterVariable();

    // データを送信する関数
    template<typename T>
    inline void SendVariable(const std::string& _variableName, const T& _var);

    // データを送信する関数
    template<typename T>
    inline void SendVariable(const std::string& _variableName, const std::vector<T>& _var);

    // データを送信する関数
    template<typename T>
    inline void SendVariable(const std::string& _variableName, const std::list<T>& _var);

    //void SetDirectoryPath() const {jsonHub_->SetDirectoryPathFromRoot}

#pragma endregion

private:
    JsonHub* jsonHub_ = nullptr;

    std::unordered_map<std::string, JsonHub::VariableAddress> memberPtrMap_;

    std::string folderPath_ = "";
    std::string groupName_ = "";

    // 既定の型か否かを確認する関数
    template <typename T>
    inline bool IsDefaultType();


};


template<typename T>
inline void JsonBinder::RegisterVariable(const std::string& _variableName, T* _variablePtr)
{
    if (!IsDefaultType<T>())
    {
        throw std::runtime_error("Not suppor this type. Please use the \"GetVariableValue\" function.");
    }
    else
    {
        memberPtrMap_[_variableName].address = _variablePtr;
        jsonHub_->GetVariableValue(groupName_, _variableName, *_variablePtr);
    }

}

template<typename T>
inline void JsonBinder::RegisterVariable(const std::string& _variableName, std::vector<T>* _variablePtr)
{
    if (!IsDefaultType<T>())
    {
        throw std::runtime_error("Not suppor this type. Please use the \"GetVariableValue\" function.");
    }
    else
    {
        memberPtrMap_[_variableName].address = RefVector<T>(*_variablePtr);
        jsonHub_->GetVariableValue(groupName_, _variableName, *_variablePtr);
    }

}

template<typename T>
inline void JsonBinder::RegisterVariable(const std::string& _variableName, std::list<T>* _variablePtr)
{
    if (!IsDefaultType<T>())
    {
        throw std::runtime_error("Not suppor this type. Please use the \"GetVariableValue\" function.");
    }
    else
    {
        memberPtrMap_[_variableName].address = RefVector<T>(*_variablePtr);
        jsonHub_->GetVariableValue(groupName_, _variableName, *_variablePtr);
    }
}

template<typename T>
inline void JsonBinder::GetVariableValue(const std::string& _variableName, T& _var)
{
    jsonHub_->GetVariableValue(groupName_, _variableName, _var);
}

template<typename T>
inline void JsonBinder::GetVariableValue(const std::string& _variableName, std::vector<T>& _var)
{
    jsonHub_->GetVariableValue(groupName_, _variableName, _var);
}

template<typename T>
inline void JsonBinder::GetVariableValue(const std::string& _variableName, std::list<T>& _var)
{
    jsonHub_->GetVariableValue(groupName_, _variableName, _var);
}

template<typename T>
inline void JsonBinder::SendVariable(const std::string& _variableName, const T& _var)
{
    jsonHub_->SetJsonValue(groupName_, _variableName, _var);
}

template<typename T>
inline void JsonBinder::SendVariable(const std::string& _variableName, const std::vector<T>& _var)
{
    jsonHub_->SetJsonValue(groupName_, _variableName, _var);
}

template<typename T>
inline void JsonBinder::SendVariable(const std::string& _variableName, const std::list<T>& _var)
{
    jsonHub_->SetJsonValue(groupName_, _variableName, _var);
}

template<typename T>
inline bool JsonBinder::IsDefaultType()
{
    if constexpr (std::is_same_v<T, int32_t> ||
                  std::is_same_v<T, uint32_t> ||
                  std::is_same_v<T, float> ||
                  std::is_same_v<T, Vector2> ||
                  std::is_same_v<T, Vector3> ||
                  std::is_same_v<T, Vector4> ||
                  std::is_same_v<T, std::string>)
    {
        return true;
    }
    else if constexpr (std::is_same_v<T, std::vector<int32_t>> ||
                       std::is_same_v<T, std::vector<uint32_t>> ||
                       std::is_same_v<T, std::vector<float>> ||
                       std::is_same_v<T, std::vector<Vector2>> ||
                       std::is_same_v<T, std::vector<Vector3>> ||
                       std::is_same_v<T, std::vector<Vector4>> ||
                       std::is_same_v<T, std::vector<std::string>>)
    {
        return true;
    }
    else if constexpr (std::is_same_v<T, std::list<int32_t>> ||
                       std::is_same_v<T, std::list<uint32_t> > ||
                       std::is_same_v<T, std::list<float>> ||
                       std::is_same_v<T, std::list<Vector2>> ||
                       std::is_same_v<T, std::list<Vector3>> ||
                       std::is_same_v<T, std::list<Vector4>> ||
                       std::is_same_v<T, std::list<std::string>>)
    {
        return true;
    }
    return false;

}
