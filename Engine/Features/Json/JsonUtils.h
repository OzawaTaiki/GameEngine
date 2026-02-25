#pragma once

#include <Externals/nlohmann/json.hpp>

namespace JsonUtils
{
    template<typename T>
    static bool GetIfExists(const std::string& key, const nlohmann::json& jsonData, T* out)
    {
        if (jsonData.contains(key))
        {
            *out = jsonData[key].get<T>();
            return true;
        }
        return false;
    }

    template<typename T>
    static T GetOrDefault(const std::string& key, const nlohmann::json& jsonData, const T& defaultValue)
    {
        if (jsonData.contains(key))
        {
            return jsonData[key].get<T>();
        }
        return defaultValue;
    }

};

