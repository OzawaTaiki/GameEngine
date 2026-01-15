#pragma once

#include "Interface/IUICollider.h"
#include <memory>
#include <json.hpp>

using json = nlohmann::json;

/// <summary>
/// UIColliderのシリアライゼーション用データ構造体
/// JSONとの相互変換に使用
/// </summary>

namespace Engine {

struct UIColliderData
{
    ColliderType type = ColliderType::Rectangle;
    IUICollider::TransformMode transformMode = IUICollider::TransformMode::UIDependent;
    json parameters;  // 各コライダー固有のパラメータ

    /// <summary>
    /// UIColliderDataからIUIColliderインスタンスを生成
    /// </summary>
    /// <returns>生成されたコライダー</returns>
    std::unique_ptr<IUICollider> CreateCollider() const;

    /// <summary>
    /// IUIColliderインスタンスからUIColliderDataを生成
    /// </summary>
    /// <param name="_collider">変換元のコライダー</param>
    /// <param name="_type">コライダーのタイプ</param>
    /// <returns>生成されたUIColliderData</returns>
    static UIColliderData FromCollider(const IUICollider* _collider, ColliderType _type);
};

// ColliderTypeのJSON変換
void to_json(json& _j, const ColliderType& _type);
void from_json(const json& _j, ColliderType& _type);

// IUICollider::TransformModeのJSON変換
void to_json(json& _j, const IUICollider::TransformMode& _mode);
void from_json(const json& _j, IUICollider::TransformMode& _mode);

// UIColliderDataのJSON変換
void to_json(json& _j, const UIColliderData& _data);
void from_json(const json& _j, UIColliderData& _data);

} // namespace Engine
