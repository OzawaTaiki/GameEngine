#pragma once

#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>

#include <Utility/ConfigManager.h>

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
    Config(const std::string& _name);
    ~Config() = default;

    void Save() const;

    template< typename T>
    inline void SetVariable( const std::string& _variableName, T* _variablePtr);

    template< typename T>
    inline void SetVariable( const std::string& _variableName, std::vector<T>* _variablePtr);

private:

    std::unordered_map<std::string, std::unordered_map<std::string, ConfigManager::VariableAddress>> ptr_;
    std::string ptrString_;

    std::string groupName_ = "";
};

template<typename T>
inline void Config::SetVariable( const std::string& _variableName, T* _variablePtr)
{
    if(groupName_.empty())
        throw std::runtime_error("groupName is empty");

    if (!ptr_.contains(groupName_) || !ptr_[groupName_].contains(_variableName))
    {
        ptr_[groupName_][_variableName].address = {};
    }

    ConfigManager::GetInstance()->GetVariableValue(groupName_, _variableName, _variablePtr);

    ptr_[_groupName][_variableName].address = _variablePtr;
}

template<typename T>
inline void Config::SetVariable( const std::string& _variableName, std::vector<T>* _variablePtr)
{
    if (groupName_.empty())
        throw std::runtime_error("groupName is empty");

    if (!ptr_.contains(groupName_) || !ptr_[groupName_].contains(_variableName))
    {
        ptr_[groupName_][_variableName].address = {};
    }

    ConfigManager::GetInstance()->GetVariableValue(groupName_, _variableName, _variablePtr);

    ptr_[_groupName][_variableName].address = _variablePtr;
}
