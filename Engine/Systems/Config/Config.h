#pragma once

#include <Physics/Math/Vector2.h>
#include <Physics/Math/Vector3.h>
#include <Physics/Math/Vector4.h>

#include <Systems/Config/ConfigManager.h>

#include <cstdint>
#include <variant>
#include <string>
#include <vector>
#include <unordered_map>

#include <stdexcept>

class Config
{
public:

    Config() = default;
    Config(const std::string& _name, const std::string& _folderpath);
    ~Config() = default;

    void Save();

    template< typename T>
    inline void SetVariable( const std::string& _variableName, T* _variablePtr);

    template<typename T>
    inline void SetVariable( const std::string& _variableName, std::vector<T>* _variablePtr);

    template<typename T>
    inline void SetVariable(const std::string& _variableName, std::list<T>* _variablePtr);

private:

    std::unordered_map<std::string, ConfigManager::VariableAddress> mPtr_;
    std::unordered_map<std::string, ConfigManager::VariableAddress> valueAddress_;

    std::string ptrString_;
    std::string folderPath_ = "";

    std::string groupName_ = "";
};

template<typename T>
inline void Config::SetVariable( const std::string& _variableName, T* _variablePtr)
{
    if(groupName_.empty())
        throw std::runtime_error("groupName is empty");

    // variableNameがない
    if ( !mPtr_.contains(_variableName))
    {
        // 追加
        mPtr_[_variableName].address = {};
        valueAddress_[_variableName].address = {};
    }

    // configManagerから値のアドレスを取得
    T* variablePtr = nullptr;
    ConfigManager::GetInstance()->GetVariableValue(groupName_, _variableName, variablePtr);

    // 値のアドレスを保存
    valueAddress_[_variableName].address = variablePtr;
    // メンバのアドレスを保存
    mPtr_[_variableName].address = _variablePtr;

    // 値のアドレスに値をコピー
    *_variablePtr = *variablePtr;
}

template<typename T>
inline void Config::SetVariable( const std::string& _variableName, std::vector<T>* _variablePtr)
{
    if (groupName_.empty())
        throw std::runtime_error("groupName is empty");

    // variableNameがない
    if (!mPtr_.contains(_variableName))
    {
        // 追加
        mPtr_[_variableName].address = {};
        valueAddress_[_variableName].address = {};
    }

    // configManagerから値のアドレスを取得
    std::vector<T>* variablePtr = nullptr;
    ConfigManager::GetInstance()->GetVariableValue(groupName_, _variableName, variablePtr);

    // 値のアドレスを保存
    valueAddress_[_variableName].address = std::ref(*variablePtr);
    // メンバのアドレスを保存
    mPtr_[_variableName].address = std::ref(*_variablePtr);

    // 値のアドレスに値をコピー
    *_variablePtr = *variablePtr;

}

template<typename T>
inline void Config::SetVariable(const std::string& _variableName, std::list<T>* _variablePtr)
{
    if (groupName_.empty())
        throw std::runtime_error("groupName is empty");

    // variableNameがない
    if (!mPtr_.contains(_variableName))
    {
        // 追加
        mPtr_[_variableName].address = {};
        valueAddress_[_variableName].address = {};
    }

    // configManagerから値のアドレスを取得
    std::list<T>* variablePtr = nullptr;
    ConfigManager::GetInstance()->GetVariableValue(groupName_, _variableName, variablePtr);

    // 値のアドレスを保存
    valueAddress_[_variableName].address = std::ref(*variablePtr);
    // メンバのアドレスを保存
    mPtr_[_variableName].address = std::ref(*_variablePtr);

    // 値のアドレスに値をコピー
    *_variablePtr = *variablePtr;
}
