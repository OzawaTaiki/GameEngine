#pragma once


#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>
#include <Math/Quaternion/Quaternion.h>
#include <Math/Easing.h>

#include <Features/Json/JsonBinder.h>

#include <json.hpp>

#include <list>
#include <variant>
#include <cstdint>
#include <typeindex>

using ParameterValue = std::variant<int32_t, float, Vector2, Vector3, Vector4, Quaternion>;

using json = nlohmann::json;

inline void to_json(json& _j, const ParameterValue& _v) {
    std::visit([&_j](const auto& value) {
        _j = json{
            {"type", std::type_index(typeid(value)).name()},
            {"value", value}
        };
    }, _v);
}

inline void from_json(const json& _j, ParameterValue& _v) {

    std::string type = _j.at("type").get<std::string>();
    if (type == typeid(int32_t).name()) {
        _v = _j.at("value").get<int32_t>();
    }
    else if (type == typeid(float).name()) {
        _v = _j.at("value").get<float>();
    }
    else if (type == typeid(Vector2).name()) {
        _v = _j.at("value").get<Vector2>();
    }
    else if (type == typeid(Vector3).name()) {
        _v = _j.at("value").get<Vector3>();
    }
    else if (type == typeid(Vector4).name()) {
        _v = _j.at("value").get<Vector4>();
    }
    else if (type == typeid(Quaternion).name()) {
        _v = _j.at("value").get<Quaternion>();
    }
    else {
        throw std::runtime_error("Invalid type");
    }
}


class SequenceEvent
{
public:

    struct KeyFrame
    {
        float time;
        ParameterValue value;
        uint32_t easingType;
        bool isSelect;
        bool isDelete;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(KeyFrame, time, value, easingType, isSelect, isDelete)
    };

public:
    SequenceEvent() = default;
    SequenceEvent(const std::string& _label,JsonBinder* _jsonBinder = nullptr);
    SequenceEvent(const std::string& _label, ParameterValue _value, JsonBinder* _jsonBinder = nullptr);
    ~SequenceEvent() = default;

    void Initialize(const std::string& _label = "");

    void Update(float _currentTime);

    void Save();
    void RegisterVariables();
    void SetJsonBinder(JsonBinder* _jsonBinder);

    std::string GetLabel() const { return label_; }
    void SetSelect(bool _isSelect) { isSelect_ = _isSelect; }
    bool IsSelect() const { return isSelect_; }

    template<typename T>
    T GetValue() const { return std::get<T>(value_); }
    ParameterValue GetValue() const { return value_; }

    void ClearSelectKeyFrames();

    std::list<KeyFrame>& GetKeyFrames() { return keyFrames_; }
    void AddKeyFrame(float _time, ParameterValue _value, uint32_t _easingType);
    void AddKeyFrame(float _time);

    void InsertKeyFrame(const KeyFrame& _keyFrame);
    void SortKeyFrames();

    void DeleteMarkedKeyFrame();

    void MarkForDelete();
    bool IsDelete() const { return isDelete_; }

    static void EditKeyFrameValue(KeyFrame& _keyFrame);


private:
    enum class UseType {
        Int,
        Float,
        Vector2,
        Vector3,
        Vector4,
        Quaternion,

        Error
    };

    UseType useType_;

    // 型のチェックを行うため
    UseType CheckType(ParameterValue _value);


    std::string label_;
    bool isSelect_;
    bool isDelete_;

    ParameterValue value_; // 補間した値

    std::list<KeyFrame> keyFrames_;

    JsonBinder* jsonBinder_ = nullptr;

public:
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(SequenceEvent, label_, keyFrames_)

};
