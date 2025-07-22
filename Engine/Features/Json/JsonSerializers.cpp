#include "JsonSerializers.h"

#include <Utility/StringUtils/StringUitls.h>

void to_json(json& _j, const Vector2& _v)
{
    _j = json{ {"x", _v.x}, {"y", _v.y} };
}

void from_json(const json& _j, Vector2& _v)
{
    if (_j.contains("x") && _j.contains("y"))
    {
        _v.x = _j["x"].get<float>();
        _v.y = _j["y"].get<float>();
    }
    else
    {
        throw std::runtime_error("Invalid Vector2 JSON format");
    }
}

void to_json(json& _j, const Vector3& _v)
{
    _j = json{ {"x", _v.x}, {"y", _v.y}, {"z", _v.z} };
}

void from_json(const json& _j, Vector3& _v)
{
    if (_j.contains("x") && _j.contains("y") && _j.contains("z"))
    {
        _v.x = _j["x"].get<float>();
        _v.y = _j["y"].get<float>();
        _v.z = _j["z"].get<float>();
    }
    else
    {
        throw std::runtime_error("Invalid Vector3 JSON format");
    }
}

void to_json(json& _j, const Vector4& _v)
{
    _j = json{ {"x", _v.x}, {"y", _v.y}, {"z", _v.z}, {"w", _v.w} };
}

void from_json(const json& _j, Vector4& _v)
{
    if (_j.contains("x") && _j.contains("y") && _j.contains("z") && _j.contains("w"))
    {
        _v.x = _j["x"].get<float>();
        _v.y = _j["y"].get<float>();
        _v.z = _j["z"].get<float>();
        _v.w = _j["w"].get<float>();
    }
    else
    {
        throw std::runtime_error("Invalid Vector4 JSON format");
    }
}

void to_json(json& _j, const Quaternion& _q)
{
    _j = json{ {"qx", _q.x}, {"qy", _q.y}, {"qz", _q.z}, {"qw", _q.w} };
}

void from_json(const json& _j, Quaternion& _q)
{
    if (_j.contains("qx") && _j.contains("qy") && _j.contains("qz") && _j.contains("qw"))
    {
        _q.x = _j["qx"].get<float>();
        _q.y = _j["qy"].get<float>();
        _q.z = _j["qz"].get<float>();
        _q.w = _j["qw"].get<float>();
    }
    else
    {
        throw std::runtime_error("Invalid Quaternion JSON format");
    }
}

void to_json(json& _j, const ParameterValue& _v)
{
    std::visit([&_j](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int32_t>)
        {
            _j = json{ {"type","int"}, {"value", arg} };
        }
        else if constexpr (std::is_same_v<T, float>)
        {
            _j = json{ {"type","float"}, {"value", arg} };
        }
        else if constexpr (std::is_same_v<T, Vector2>)
        {
            _j = json{ {"type","Vector2"}, {"value", arg} };
        }
        else if constexpr (std::is_same_v<T, Vector3>)
        {
            _j = json{ {"type","Vector3"}, {"value", arg} };
        }
        else if constexpr (std::is_same_v<T, Vector4>)
        {
            _j = json{ {"type","Vector4"}, {"value", arg} };
        }
        else if constexpr (std::is_same_v<T, Quaternion>)
        {
            _j = json{ {"type","Quaternion"}, {"value", arg} };
        }
        else {
            throw std::runtime_error("Invalid ParameterValue type");
        }
        }, _v);
}

void from_json(const json& _j, ParameterValue& _v)
{
    if (_j.contains("type") && _j.contains("value"))
    {
        std::string type = _j["type"].get<std::string>();
        if (type == "int")
        {
            _v = _j["value"].get<int32_t>();
        }
        else if (type == "float")
        {
            _v = _j["value"].get<float>();
        }
        else if (type == "Vector2")
        {
            _v = _j["value"].get<Vector2>();
        }
        else if (type == "Vector3")
        {
            _v = _j["value"].get<Vector3>();
        }
        else if (type == "Vector4")
        {
            _v = _j["value"].get<Vector4>();
        }
        else if (type == "Quaternion")
        {
            _v = _j["value"].get<Quaternion>();
        }
        else if (type = StringUtils::GetAfterLast(type, "struct "); !type.empty())
        {
            if (type == "int")
            {
                _v = _j["value"].get<int32_t>();
            }
            else if (type == "float")
            {
                _v = _j["value"].get<float>();
            }
            else if (type == "Vector2")
            {
                _v = _j["value"].get<Vector2>();
            }
            else if (type == "Vector3")
            {
                _v = _j["value"].get<Vector3>();
            }
            else if (type == "Vector4")
            {
                _v = _j["value"].get<Vector4>();
            }
            else if (type == "Quaternion")
            {
                _v = _j["value"].get<Quaternion>();
            }
        }
        else
        {
            throw std::runtime_error("Invalid ParameterValue type");
        }
    }
    else
    {
        throw std::runtime_error("Invalid ParameterValue JSON format");
    }
}

void to_json(json& _j, const SequenceEvent::KeyFrame& _v)
{
    _j = json
    {
        {"time", _v.time},
        {"easingType", _v.easingType},
        {"isSelect", _v.isSelect},
        {"isDelete", _v.isDelete}
    };

    to_json(_j["value"], _v.value);
}

void from_json(const json& _j, SequenceEvent::KeyFrame& _v)
{
    if (_j.contains("time") && _j.contains("value") && _j.contains("easingType"))
    {
        _v.time = _j["time"].get<float>();
        _v.easingType = _j["easingType"].get<uint32_t>();
        _v.isSelect = _j.value("isSelect", false);
        _v.isDelete = _j.value("isDelete", false);

        from_json(_j["value"], _v.value);
    }
    else
    {
        throw std::runtime_error("Invalid KeyFrame JSON format");
    }
}

void to_json(json& _j, const SequenceEvent& _v)
{
    _j = json{
        {"label",_v.GetLabel()},
        {"keyFrame",_v.GetKeyFrames()}
    };
}

void from_json(const json& _j, SequenceEvent& _v)
{
    if (_j.contains("label") && _j.contains("keyFrame"))
    {
        _v.SetLabel(_j["label"].get<std::string>());
        _v.SetKeyFrames(_j["keyFrame"].get<std::list<SequenceEvent::KeyFrame>>());
    }
    else
    {
        throw std::runtime_error("Invalid SequenceEvent JSON format");
    }
}

void to_json(json& _j, const TextParam& _v)
{
    _j = json{
        {"scale", _v.scale},
        {"rotate", _v.rotate},
        {"position", _v.position},
        {"useGradient", _v.useGradient},
        {"topColor", _v.topColor},
        {"bottomColor", _v.bottomColor},
        {"pivot", _v.pivot},
        {"useOutline", _v.useOutline},
        {"outlineColor", _v.outlineColor},
        {"outlineScale", _v.outlineScale}
    };
}

void from_json(const json& _j, TextParam& _v)
{
    if (_j.contains("scale") && _j.contains("rotate") && _j.contains("position") &&
        _j.contains("useGradient") && _j.contains("topColor") && _j.contains("bottomColor") &&
        _j.contains("pivot") && _j.contains("useOutline") && _j.contains("outlineColor") &&
        _j.contains("outlineScale"))
    {
        _v.scale = _j["scale"].get<Vector2>();
        _v.rotate = _j["rotate"].get<float>();
        _v.position = _j["position"].get<Vector2>();
        _v.useGradient = _j["useGradient"].get<bool>();
        _v.topColor = _j["topColor"].get<Vector4>();
        _v.bottomColor = _j["bottomColor"].get<Vector4>();
        _v.pivot = _j["pivot"].get<Vector2>();
        _v.useOutline = _j["useOutline"].get<bool>();
        _v.outlineColor = _j["outlineColor"].get<Vector4>();
        _v.outlineScale = _j["outlineScale"].get<float>();
    }
    else
    {
        throw std::runtime_error("Invalid TextParam JSON format");
    }
}
