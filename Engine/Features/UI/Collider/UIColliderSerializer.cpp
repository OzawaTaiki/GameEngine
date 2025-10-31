#include "UIColliderSerializer.h"
#include "UICircleCollider.h"
#include "UIRentangleCollider.h"
#include "UIEllipseCollider.h"
#include "UIParallelogramCollider.h"
#include "UIQuadCollider.h"
#include "UIColliderFactory.h"

#include <Features/Json/JsonSerializers.h>

// ColliderTypeのJSON変換
void to_json(json& _j, const ColliderType& _type)
{
    switch (_type)
    {
    case ColliderType::Rectangle:
        _j = "Rectangle";
        break;
    case ColliderType::Circle:
        _j = "Circle";
        break;
    case ColliderType::Ellipse:
        _j = "Ellipse";
        break;
    case ColliderType::Parallelogram:
        _j = "Parallelogram";
        break;
    case ColliderType::Quad:
        _j = "Quad";
        break;
    default:
        _j = "Unknown";
        break;
    }
}

void from_json(const json& _j, ColliderType& _type)
{
    std::string typeStr = _j.get<std::string>();
    if (typeStr == "Rectangle")
        _type = ColliderType::Rectangle;
    else if (typeStr == "Circle")
        _type = ColliderType::Circle;
    else if (typeStr == "Ellipse")
        _type = ColliderType::Ellipse;
    else if (typeStr == "Parallelogram")
        _type = ColliderType::Parallelogram;
    else if (typeStr == "Quad")
        _type = ColliderType::Quad;
    else
        _type = ColliderType::Rectangle; // デフォルト
}

// TransformModeのJSON変換
void to_json(json& _j, const IUICollider::TransformMode& _mode)
{
    switch (_mode)
    {
    case IUICollider::TransformMode::UIDependent:
        _j = "UIDependent";
        break;
    case IUICollider::TransformMode::Independent:
        _j = "Independent";
        break;
    default:
        _j = "UIDependent";
        break;
    }
}

void from_json(const json& _j, IUICollider::TransformMode& _mode)
{
    std::string modeStr = _j.get<std::string>();
    if (modeStr == "UIDependent")
        _mode = IUICollider::TransformMode::UIDependent;
    else if (modeStr == "Independent")
        _mode = IUICollider::TransformMode::Independent;
    else
        _mode = IUICollider::TransformMode::UIDependent; // デフォルト
}

// UIColliderDataのJSON変換
void to_json(json& _j, const UIColliderData& _data)
{
    _j = json{
        {"type", _data.type},
        {"transformMode", _data.transformMode},
        {"parameters", _data.parameters}
    };
}

void from_json(const json& _j, UIColliderData& _data)
{
    if (_j.contains("type"))
        _data.type = _j["type"].get<ColliderType>();

    if (_j.contains("transformMode"))
        _data.transformMode = _j["transformMode"].get<IUICollider::TransformMode>();

    if (_j.contains("parameters"))
        _data.parameters = _j["parameters"];
}

// UIColliderDataからIUIColliderインスタンスを生成
std::unique_ptr<IUICollider> UIColliderData::CreateCollider() const
{
    std::unique_ptr<IUICollider> collider = nullptr;

    switch (type)
    {
    case ColliderType::Rectangle:
    {
        auto rect = std::make_unique<UIRentangleCollider>();
        if (parameters.contains("localOffset"))
            rect->SetLocalOffset(parameters["localOffset"].get<Vector2>());
        if (parameters.contains("localSize"))
            rect->SetLocalSize(parameters["localSize"].get<Vector2>());
        collider = std::move(rect);
        break;
    }

    case ColliderType::Circle:
    {
        auto circle = std::make_unique<UICircleCollider>();
        if (parameters.contains("localOffset"))
            circle->SetLocalOffset(parameters["localOffset"].get<Vector2>());
        if (parameters.contains("independentRadius"))
            circle->SetRadius(parameters["independentRadius"].get<float>());
        collider = std::move(circle);
        break;
    }

    case ColliderType::Ellipse:
    {
        auto ellipse = std::make_unique<UIEllipseCollider>();
        // Ellipseのパラメータを設定
        // TODO: UIEllipseColliderの実装に合わせて調整
        collider = std::move(ellipse);
        break;
    }

    case ColliderType::Parallelogram:
    {
        Vector2 skew = { 0.0f, 0.0f };
        if (parameters.contains("skew"))
            skew = parameters["skew"].get<Vector2>();

        auto parallelogram = std::make_unique<UIParallelogramCollider>(skew);
        if (parameters.contains("localOffset"))
            parallelogram->SetLocalOffset(parameters["localOffset"].get<Vector2>());
        if (parameters.contains("localSize"))
            parallelogram->SetLocalSize(parameters["localSize"].get<Vector2>());
        if (parameters.contains("localRotate"))
            parallelogram->SetLocalRotate(parameters["localRotate"].get<float>());
        collider = std::move(parallelogram);
        break;
    }

    case ColliderType::Quad:
    {
        auto quad = std::make_unique<UIQuadCollider>();
        if (parameters.contains("localCorners") && parameters["localCorners"].is_array())
        {
            auto corners = parameters["localCorners"];
            for (int i = 0; i < 4 && i < corners.size(); ++i)
            {
                quad->SetLocalCorner(i, corners[i].get<Vector2>());
            }
        }
        collider = std::move(quad);
        break;
    }

    default:
        // デフォルトは矩形
        collider = std::make_unique<UIRentangleCollider>();
        break;
    }

    if (collider)
    {
        collider->SetTransformMode(transformMode);
    }

    return collider;
}

// IUIColliderインスタンスからUIColliderDataを生成
UIColliderData UIColliderData::FromCollider(const IUICollider* _collider, ColliderType _type)
{
    UIColliderData data;
    data.type = _type;
    data.transformMode = _collider->GetTransformMode();

    switch (_type)
    {
    case ColliderType::Rectangle:
    {
        auto rect = static_cast<const UIRentangleCollider*>(_collider);
        data.parameters = json{
            {"localOffset", rect->GetLocalOffset()},
            {"localSize", rect->GetLocalSize()}
        };
        break;
    }

    case ColliderType::Circle:
    {
        auto circle = static_cast<const UICircleCollider*>(_collider);
        data.parameters = json{
            {"localOffset", circle->GetLocalOffset()},
            {"independentRadius", circle->GetIndependentRadius()}
        };
        break;
    }

    case ColliderType::Ellipse:
    {
        // TODO: UIEllipseColliderの実装に合わせて調整
        data.parameters = json::object();
        break;
    }

    case ColliderType::Parallelogram:
    {
        auto parallelogram = static_cast<const UIParallelogramCollider*>(_collider);
        data.parameters = json{
            {"skew", parallelogram->GetSkew()},
            {"localOffset", parallelogram->GetLocalOffset()},
            {"localSize", parallelogram->GetLocalSize()},
            {"localRotate", parallelogram->GetLocalRotate()}
        };
        break;
    }

    case ColliderType::Quad:
    {
        auto quad = static_cast<const UIQuadCollider*>(_collider);
        json corners = json::array();
        for (int i = 0; i < 4; ++i)
        {
            corners.push_back(quad->GetLocalCorner(i));
        }
        data.parameters = json{
            {"localCorners", corners}
        };
        break;
    }

    default:
        data.parameters = json::object();
        break;
    }

    return data;
}
