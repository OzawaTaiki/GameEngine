#pragma once
#include <Features/UI/Collider/Interface/IUICollider.h>
#include <memory>
#include <string>

/// <summary>
/// UIColliderの生成を行うファクトリクラス
/// ImGuiでのColliderタイプ選択UIも提供
/// </summary>
class UIColliderFactory
{
public:
    /// <summary>
    /// ColliderTypeから対応するColliderを生成
    /// </summary>
    /// <param name="type">生成するColliderのタイプ</param>
    /// <returns>生成されたCollider</returns>
    static std::unique_ptr<IUICollider> Create(ColliderType type);

    /// <summary>
    /// ImGuiでColliderタイプを選択するUI
    /// タイプが変更された場合、新しいColliderを返す
    /// </summary>
    /// <param name="currentType">現在のColliderタイプ（変更される可能性がある）</param>
    /// <param name="label">ImGuiのラベル</param>
    /// <returns>新しいCollider（変更なしならnullptr）</returns>
    static std::unique_ptr<IUICollider> ImGuiSelectCollider(
        ColliderType& currentType,
        const std::string& label = "Collider Type");

    /// <summary>
    /// ColliderTypeの名前を取得
    /// </summary>
    /// <param name="type">Colliderタイプ</param>
    /// <returns>タイプ名</returns>
    static const char* GetTypeName(ColliderType type);

private:
    // ImGui用の一時パラメータ
    static Vector2 tempSkew_;  // 平行四辺形のスキュー値
};
