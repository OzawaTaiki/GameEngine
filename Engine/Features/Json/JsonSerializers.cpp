#include "JsonSerializers.h"
#include <Features/UI/Collider/UIColliderSerializer.h>

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

void to_json(json& _j, const Rect& _v)
{
    _j = json
    {
        {"leftTop" , _v.leftTop},
        {"size", _v.size}
    };
}

void from_json(const json& _j, Rect& _v)
{
    if (_j.contains("leftTop") && _j.contains("size"))
    {
        _v.leftTop = _j["leftTop"].get<Vector2>();
        _v.size = _j["size"].get<Vector2>();
    }
    else
    {
        throw std::runtime_error("Invalid Rect JSON format");
    }
}

void to_json(json& _j, const ParameterValue& _v)
{
    std::visit([&_j](auto&& arg)
               {
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
                   else
                   {
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

void to_json(json& _j, const PrimitiveType& _type)
{
    switch (_type)
    {
        case PrimitiveType::Plane:
            _j = "Plane";
            break;
        case PrimitiveType::Triangle:
            _j = "Triangle";
            break;
        case PrimitiveType::Cylinder:
            _j = "Cylinder";
            break;
        case PrimitiveType::Ring:
            _j = "Ring";
            break;
        case PrimitiveType::Cube:
            _j = "Cube";
            break;
        default:
            _j = "Unknown";
            break;
    }
}

void from_json(const json& _j, PrimitiveType& _type)
{
    std::string typeStr = _j.get<std::string>();
    if (typeStr == "Plane")
        _type = PrimitiveType::Plane;
    else if (typeStr == "Triangle")
        _type = PrimitiveType::Triangle;
    else if (typeStr == "Cylinder")
        _type = PrimitiveType::Cylinder;
    else if (typeStr == "Ring")
        _type = PrimitiveType::Ring;
    else if (typeStr == "Cube")
        _type = PrimitiveType::Cube;
    else
        throw std::runtime_error("Invalid PrimitiveType: " + typeStr);
}

void to_json(json& _j, const PrimitiveSettings::PlaneData& _plane)
{
    _j = json{
        {"normal", _plane.normal},
        {"size", _plane.size},
        {"pivot", _plane.pivot}
    };
}

void from_json(const json& _j, PrimitiveSettings::PlaneData& _plane)
{
    if (_j.contains("normal"))
        _plane.normal = _j["normal"].get<Vector3>();
    if (_j.contains("size"))
        _plane.size = _j["size"].get<Vector2>();
    if (_j.contains("pivot"))
        _plane.pivot = _j["pivot"].get<Vector3>();
}

void to_json(json& _j, const PrimitiveSettings::TriangleData& _triangle)
{
    _j = json{
        {"vertex0", _triangle.vertex0},
        {"vertex1", _triangle.vertex1},
        {"vertex2", _triangle.vertex2},
        {"normal", _triangle.normal}
    };
}

void from_json(const json& _j, PrimitiveSettings::TriangleData& _triangle)
{
    if (_j.contains("vertex0"))
        _triangle.vertex0 = _j["vertex0"].get<Vector3>();
    if (_j.contains("vertex1"))
        _triangle.vertex1 = _j["vertex1"].get<Vector3>();
    if (_j.contains("vertex2"))
        _triangle.vertex2 = _j["vertex2"].get<Vector3>();
    if (_j.contains("normal"))
        _triangle.normal = _j["normal"].get<Vector3>();
}

void to_json(json& _j, const PrimitiveSettings::CylinderData& _cylinder)
{
    _j = json{
        {"topRadius", _cylinder.topRadius},
        {"bottomRadius", _cylinder.bottomRadius},
        {"height", _cylinder.height},
        {"hasTop", _cylinder.hasTop},
        {"hasBottom", _cylinder.hasBottom},
        {"startAngle", _cylinder.startAngle},
        {"endAngle", _cylinder.endAngle},
        {"loop", _cylinder.loop}
    };
}

void from_json(const json& _j, PrimitiveSettings::CylinderData& _cylinder)
{
    _cylinder.topRadius = _j.value("topRadius", 1.0f);
    _cylinder.bottomRadius = _j.value("bottomRadius", 1.0f);
    _cylinder.height = _j.value("height", 2.0f);
    _cylinder.hasTop = _j.value("hasTop", false);
    _cylinder.hasBottom = _j.value("hasBottom", false);
    _cylinder.startAngle = _j.value("startAngle", 0.0f);
    _cylinder.endAngle = _j.value("endAngle", 6.28318f);
    _cylinder.loop = _j.value("loop", true);
}

void to_json(json& _j, const PrimitiveSettings::RingData& _ring)
{
    _j = json{
        {"innerRadius", _ring.innerRadius},
        {"outerRadius", _ring.outerRadius},
        {"startAngle", _ring.startAngle},
        {"endAngle", _ring.endAngle},
        {"startOuterRadiusRatio", _ring.startOuterRadiusRatio},
        {"endOuterRadiusRatio", _ring.endOuterRadiusRatio}
    };
}

void from_json(const json& _j, PrimitiveSettings::RingData& _ring)
{
    _ring.innerRadius = _j.value("innerRadius", 0.5f);
    _ring.outerRadius = _j.value("outerRadius", 1.0f);
    _ring.startAngle = _j.value("startAngle", 0.0f);
    _ring.endAngle = _j.value("endAngle", 6.28318f);
    _ring.startOuterRadiusRatio = _j.value("startOuterRadiusRatio", 1.0f);
    _ring.endOuterRadiusRatio = _j.value("endOuterRadiusRatio", 1.0f);
}

void to_json(json& _j, const PrimitiveSettings::CubeData& _cube)
{
    _j = json{
        {"size", _cube.size},
        {"pivot", _cube.pivot},
        {"hasTop", _cube.hasTop},
        {"hasBottom", _cube.hasBottom}
    };
}

void from_json(const json& _j, PrimitiveSettings::CubeData& _cube)
{
    if (_j.contains("size"))
        _cube.size = _j["size"].get<Vector3>();
    if (_j.contains("pivot"))
        _cube.pivot = _j["pivot"].get<Vector3>();
    _cube.hasTop = _j.value("hasTop", true);
    _cube.hasBottom = _j.value("hasBottom", true);
}

void to_json(json& _j, const PrimitiveSettings& _settings)
{
    _j = json{
        {"name", _settings.name},
        {"divide", _settings.divide},
        {"flipU", _settings.flipU},
        {"flipV", _settings.flipV},
        {"plane", _settings.plane},
        {"triangle", _settings.triangle},
        {"cylinder", _settings.cylinder},
        {"ring", _settings.ring}
    };
}

void from_json(const json& _j, PrimitiveSettings& _settings)
{
    _settings.name = _j.value("name", "NewPrimitive");
    _settings.divide = _j.value("divide", 16u);
    _settings.flipU = _j.value("flipU", false);
    _settings.flipV = _j.value("flipV", false);

    if (_j.contains("plane"))
        _settings.plane = _j["plane"].get<PrimitiveSettings::PlaneData>();
    if (_j.contains("triangle"))
        _settings.triangle = _j["triangle"].get<PrimitiveSettings::TriangleData>();
    if (_j.contains("cylinder"))
        _settings.cylinder = _j["cylinder"].get<PrimitiveSettings::CylinderData>();
    if (_j.contains("ring"))
        _settings.ring = _j["ring"].get<PrimitiveSettings::RingData>();
}

void to_json(json& _j, const CreatedPrimitive& _primitive)
{
    _j = json{
        {"type", _primitive.type},
        {"settings", _primitive.settings},
        {"isSaved", _primitive.isSaved}
        // modelは保存しない（ポインタなので）
    };
}

void from_json(const json& _j, CreatedPrimitive& _primitive)
{
    if (_j.contains("type"))
        _primitive.type = _j["type"].get<PrimitiveType>();
    if (_j.contains("settings"))
        _primitive.settings = _j["settings"].get<PrimitiveSettings>();

    _primitive.isSaved = _j.value("isSaved", false);
    _primitive.model = nullptr; // モデルは別途生成する必要がある
}
