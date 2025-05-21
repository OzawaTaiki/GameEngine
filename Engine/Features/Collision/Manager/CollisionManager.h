#pragma once

#include <Features/Collision/Collider/Collider.h>
#include <Features/Collision/Detector/CollisionDetector.h>
#include <Features/Collision/Tree/QuadTree.h>
#include <vector>
#include <unordered_map>
#include <functional>

// 衝突判定の管理を行うクラス
class CollisionManager
{
public:
    // インスタンスを取得する
    static CollisionManager* GetInstance();

    // 初期化
    void Initialize(const Vector2& _fieldSize,uint32_t _level);

    // 終了処理
    void Finalize();

    // 毎フレームの更新
    void Update();

    // コライダーを登録する
    void RegisterCollider(Collider* _collider);

    // 衝突判定を実行する
    void CheckCollisions();

    // 衝突応答を実行する
    void ResolveCollisions();

    // 衝突状態を更新する
    void UpdateCollisionStates();

    // コライダーを描画する
    void DrawColliders();

    // デバッグ描画の有効/無効を設定
    void SetDrawEnabled(bool _enabled) { isDrawEnabled_ = _enabled; }

    // デバッグ描画の有効/無効を取得
    bool IsDrawEnabled() const { return isDrawEnabled_; }

    void UnregisterCollider(Collider* _collider);


    void ImGui();

private:
    // 衝突応答を実行する
    void ResolveCollision(const CollisionPair& _pair);

    // 空間分割などの最適化のためのユーティリティ
    void UpdateBroadPhase();

private:
    // コライダーのリスト
    std::vector<Collider*> colliders_;

    // 衝突ペアのリスト
    std::vector<CollisionPair> collisionPairs_;

    // デバッグ描画の有効/無効
    bool isDrawEnabled_ = true;

    // 空間分割のためのグリッド（将来的な拡張のために用意）
    // std::unordered_map<int, std::vector<Collider*>> grid_;

    // 衝突ペアのリスト（衝突する可能性があるpair）
    std::vector<std::pair<Collider*, Collider*>> potentialCollisions_;

    std::unique_ptr<QuadTree> quadTree_; // 空間分割のための四分木

private:
    // コンストラクタ
    CollisionManager();

    // デストラクタ
    ~CollisionManager() = default;

    // コピーコンストラクタとコピー代入演算子を禁止
    CollisionManager(const CollisionManager&) = delete;
    CollisionManager& operator=(const CollisionManager&) = delete;
};