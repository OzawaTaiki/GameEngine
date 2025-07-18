#pragma once

#include <functional>
#include <Features/Json/JsonSerializers.h>

#include <json.hpp>

class VariableHolder
{
public:

    template<typename T>
    VariableHolder(T* _var) :
        variable_(static_cast<void*>(_var)),
        loader_([](void* var, const nlohmann::json& j) { *static_cast<T*>(var) = j.get<T>(); }),
        saver_([](void* var) { return nlohmann::json(*static_cast<T*>(var)); })
    {}

    void Load(const nlohmann::json& _j) { loader_(variable_, _j); }
    nlohmann::json Save() { return saver_(variable_); }

private:

    void* variable_ = nullptr;

    // jsonをvoid* に変換
    std::function<void(void*,const nlohmann::json& )> loader_;
    // void* を json に復元
    std::function< nlohmann::json(void*)> saver_;
};
