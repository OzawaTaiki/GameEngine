#include "UIColliderFactory.h"
#include <Features/UI/Collider/UICircleCollider.h>
#include <Features/UI/Collider/UIRentangleCollider.h>
#include <Features/UI/Collider/UIEllipseCollider.h>
#include <Features/UI/Collider/UIParallelogramCollider.h>
#include <Features/UI/Collider/UIQuadCollider.h>

#ifdef _DEBUG
#include <imgui.h>
#endif

// 静的メンバ変数の初期化
Vector2 UIColliderFactory::tempSkew_ = { 0.0f, 0.0f };

std::unique_ptr<IUICollider> UIColliderFactory::Create(ColliderType type)
{
    switch (type)
    {
    case ColliderType::Rectangle:
        return std::make_unique<UIRentangleCollider>();

    case ColliderType::Circle:
        return std::make_unique<UICircleCollider>();

    case ColliderType::Ellipse:
        return std::make_unique<UIEllipseCollider>();

    case ColliderType::Parallelogram:
        return std::make_unique<UIParallelogramCollider>(tempSkew_);

    case ColliderType::Quad:
        return std::make_unique<UIQuadCollider>();

    default:
        // デフォルトは矩形
        return std::make_unique<UIRentangleCollider>();
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

    case ColliderType::Parallelogram:
        return "Parallelogram";

    case ColliderType::Quad:
        return "Quad";

    default:
        return "Unknown";
    }
}

std::unique_ptr<IUICollider> UIColliderFactory::ImGuiSelectCollider(
    ColliderType& currentType,
    const std::string& label)
{
#ifdef _DEBUG
    std::unique_ptr<IUICollider> newCollider = nullptr;

    // ドロップダウンメニュー用のアイテム配列
    const char* items[] = {
        GetTypeName(ColliderType::Rectangle),
        GetTypeName(ColliderType::Circle),
        GetTypeName(ColliderType::Ellipse),
        GetTypeName(ColliderType::Parallelogram),
        GetTypeName(ColliderType::Quad)
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
