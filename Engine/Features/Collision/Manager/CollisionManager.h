#pragma once
#include <Features/Collision/Collider/Collider.h>
#include <Features/Collision/Detector/CollisionDetector.h>
#include <Features/Collision/Tree/QuadTree.h>
#include <Features/Collision/SpiralHashGird/SpatialHashGrid.h>
#include <vector>
#include <unordered_map>
#include <functional>


namespace Engine {

class CollisionManager
{
public:
    // インスタンスを取得する
    static CollisionManager* GetInstance();

    // 初期化
    void Initialize(const Vector2& _fieldSize, uint32_t _level, const Vector2& _leftBottom, float _gridSize = 1.0f);

    // 終了処理
    void Finalize();

    // 毎フレームの更新
    void Update();

    // コライダーを登録する
    void RegisterCollider(Collider* _collider);

    // コライダーを登録する（静的コライダー用 地形など)
    void RegisterStaticCollider(Collider* _collider);

    // 衝突判定を実行する
    void CheckCollisionsWithBroadPhase();

    // 衝突判定を実行する（ブロードフェーズを使用しない）
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

    // コライダーの登録解除
    void UnregisterCollider(Collider* _collider);

    // デバッグUI
    void ImGui(bool* _oopen);

private:
    // 衝突応答を実行する
    void ResolveCollision(const CollisionPair& _pair);

    // 空間分割などの最適化のためのユーティリティ
    void UpdateBroadPhase();

    // 衝突判定の範囲処理
    void CheckCollisionsRange();

    // 即座削除の内部メソッド
    void RemoveColliderImmediate(Collider* _collider);

private:
    // コライダーのリスト
    std::vector<Collider*> colliders_;
    std::vector<Collider*> staticColliders_; // 静的コライダーのリスト

    // 衝突ペアのリスト
    std::vector<CollisionPair> collisionPairs_;

    // デバッグ描画の有効/無効
    bool isDrawEnabled_;

    // 衝突ペアのリスト（衝突する可能性があるpair）
    std::vector<std::pair<Collider*, Collider*>> potentialCollisions_;

    // 空間分割のための四分木
    std::unique_ptr<QuadTree> quadTree_;

    // スパイラルハッシュグリッド
    std::unique_ptr<SpatialHashGrid> spiralHashGrid_;

#ifdef _DEBUG
    // デバッグ用の衝突検出器
    int32_t colliderCount_ = 0; // 登録されたコライダーの数
    int32_t collisionPairCount_ = 0; // 衝突ペアの数
#endif // _DEBUG
    bool enableBroadPhase_ = true; // ブロードフェーズを使用するかどうか

private:
    // コンストラクタ
    CollisionManager();

    // デストラクタ
    ~CollisionManager() = default;

    // コピーコンストラクタとコピー代入演算子を禁止
    CollisionManager(const CollisionManager&) = delete;
    CollisionManager& operator=(const CollisionManager&) = delete;
};

} // namespace Engine
