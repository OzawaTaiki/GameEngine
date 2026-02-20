#include "UIColliderFactory.h"
#include <Features/UI/Collider/UICircleCollider.h>
#include <Features/UI/Collider/UIRecntangleCollider.h>
#include <Features/UI/Collider/UIEllipseCollider.h>
#include <Features/UI/Collider/UIQuadCollider.h>

#include <Debug/ImGuiDebugManager.h>
#include "UIConvexPolygonCollider.h"


namespace Engine {

std::unique_ptr<IUICollider> UIColliderFactory::Create(ColliderType type)
{
    switch (type)
    {
        case ColliderType::Rectangle:
            return std::make_unique<UIRectangleCollider>();

        case ColliderType::Circle:
            return std::make_unique<UICircleCollider>();

        case ColliderType::Ellipse:
            return std::make_unique<UIEllipseCollider>();

        case ColliderType::Quad:
            return std::make_unique<UIQuadCollider>();

        case ColliderType::ConvexPolygon:
            return std::make_unique<UIConvexPolygonCollider>();

        default:
            // デフォルトは矩形
            return std::make_unique<UIRectangleCollider>();
    }
}

const char* UIColliderFactory::GetTypeName(ColliderType type)
{
    switch (type)
    {
        case ColliderType::Rectangle:
            return "Rectangle";

        case ColliderType::Circle:
            return "Circle";

        case ColliderType::Ellipse:
            return "Ellipse";

        case ColliderType::Quad:
            return "Quad";
        case ColliderType::ConvexPolygon:
            return "ConvexPolygon";

        default:
            return "Unknown";
    }
}

std::unique_ptr<IUICollider> UIColliderFactory::ImGuiSelectCollider(
    [[maybe_unused]] ColliderType& currentType,
    [[maybe_unused]] const std::string& label)
{
#ifdef _DEBUG
    std::unique_ptr<IUICollider> newCollider = nullptr;

    // ドロップダウンメニュー用のアイテム配列
    const char* items[] = {
        GetTypeName(ColliderType::Rectangle),
        GetTypeName(ColliderType::Circle),
        GetTypeName(ColliderType::Ellipse),
        GetTypeName(ColliderType::Quad),
        GetTypeName(ColliderType::ConvexPolygon)
    };

    int currentIndex = static_cast<int>(currentType);

    // Colliderタイプ選択用のコンボボックス
    if (ImGui::Combo(label.c_str(), &currentIndex, items, IM_ARRAYSIZE(items)))
    {
        // タイプが変更された
        ColliderType newType = static_cast<ColliderType>(currentIndex);
        currentType = newType;

        // 新しいColliderを生成
        newCollider = Create(newType);
    }

    return newCollider;
#else
    // Release時は何もしない
    return nullptr;
#endif
}

} // namespace Engine
