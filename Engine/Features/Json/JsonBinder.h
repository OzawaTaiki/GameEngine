#pragma once

#include <Features/Json/Loader/JsonFileIO.h>
#include <Features/Json/JsonSerializers.h>
#include <Features/Json/VariableHolder.h>

#include <string>
#include <vector>
#include <list>

// 各クラスでインスタンスをもつ
class JsonBinder
{
public:
    JsonBinder(const std::string& _name, const std::string& _directioy);
    ~JsonBinder() = default;

    void Save();

#pragma region 登録関数
    // 登録する関数
    template<typename T>
    void RegisterVariable(const std::string& _variableName, T* _variablePtr);

    template<typename T>
    void GetVariableValue(const std::string& _variableName, T& _var);

    template<typename T>
    void SendVariable(const std::string& _variableName, const T& _var);
#pragma endregion


private:

    std::unordered_map<std::string, VariableHolder> memberPtrMap_;
    json jsonData_;
    std::string folderPath_ = "";
    std::string groupName_ = "";

};


template<typename T>
inline void JsonBinder::RegisterVariable(const std::string& _variableName, T* _variablePtr)
{
    auto [it, inserted] = memberPtrMap_.emplace(_variableName, VariableHolder(_variablePtr));

    if(jsonData_[groupName_].contains(_variableName))
        it->second.Load(jsonData_[groupName_][_variableName]);
}

template<typename T>
inline void JsonBinder::GetVariableValue(const std::string& _variableName, T& _var)
{
    if (jsonData_[groupName_].contains(_variableName))
    {
        _var = jsonData_[groupName_][_variableName].get<T>();
    }
}

template<typename T>
inline void JsonBinder::SendVariable(const std::string& _variableName, const T& _var)
{
    jsonData_[groupName_][_variableName] = _var; // 変数の値をjsonに設定
}
