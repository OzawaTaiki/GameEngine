#pragma once


#include <Features/Model/Mesh/MargedMesh.h>
#include <Features/Model/Animation/ModelAnimation.h>
#include <Features/Model/Animation/SkinCluster/SkinCluster.h>
#include <Features/Model/Animation/Skeleton/Skeleton.h>
#include <Features/Model/Animation/SkinningCS.h>

#include <memory>
#include <utility>


class Model;
class AnimationController
{

public:

    AnimationController(Model* _model);
    ~AnimationController() = default;

    // 初期化
    void Initialize();

    // 更新
    void Update(float _deltaTime);

    // アニメーションをセット
    void SetAnimation(const std::string& _name, bool _loop = false);

    // アニメーションを変更
    void ChangeAnimation(const std::string& _name, float _blendTime, bool _loop = false);

    // アニメーションを停止
    void StopAnimation();


    // アニメーションを停止
    void ToIdle(float _timeToIdle);

    // スケルトンの描画
    void DrawSkeleton(const Matrix4x4& _worldMat);

    // スケルトンのワールド空間行列を取得
    Matrix4x4 GetSkeletonSpaceMatrix(uint32_t _index = 0) const;

    // margedMeshを取得
    MargedMesh* GetMargedMesh() const { return margedMesh_.get(); }

    // アニメーションが再生中かどうか
    bool IsAnimationPlaying() const;


    void ImGui();
    
private:

    Model* model_ = nullptr;

    std::unique_ptr<MargedMesh> margedMesh_ = nullptr;

    Skeleton skeleton_ = {};

    std::unique_ptr<ModelAnimation> currentAnimation_ = {};

    SkinCluster skinCluster_ = {};

    std::unique_ptr<SkinningCS> skinningCS_ = nullptr;

};