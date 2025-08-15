#include "AnimationController.h"

#include <Features/Model/Model.h>

AnimationController::AnimationController(Model* _model) :
    model_(_model),
    skeleton_(),
    currentAnimation_(),
    skinCluster_()
{
}

void AnimationController::Initialize(ID3D12PipelineState* _computePipeline, ID3D12RootSignature* _rootSignature)
{
    if (!model_)
        return;


    skeleton_.CreateSkeleton(model_->GetNode());

    margedMesh_ = std::make_unique<MargedMesh>();
    margedMesh_->Initialize(model_->GetMeshes());

    uint32_t vertexCount = static_cast<uint32_t>(margedMesh_->GetVertexCount());

    skinCluster_ = model_->GetSkinCluster();
    skinCluster_.CreateResources(
        static_cast<uint32_t>(skeleton_.GetJoints().size()),
        vertexCount,
        skeleton_.GetJointMap());

    margedMesh_->SetSkinnedVertexBufferView(SkinningCS::CreateOutputVertexResource(vertexCount));

    skinningCS_ = std::make_unique<SkinningCS>(_computePipeline, _rootSignature);

    skinningCS_->CreateSRVForInputVertexResource(margedMesh_->GetVertexResource(), vertexCount);
    skinningCS_->CreateSRVForInfluenceResource(skinCluster_.GetInfluenceResource(), vertexCount);
    skinningCS_->CreateSRVForOutputVertexResource(margedMesh_->GetSkinnedVertexResource(), vertexCount);
    skinningCS_->CreateSRVForMatrixPaletteResource(skinCluster_.GetPaletteResource(), static_cast<uint32_t>(skeleton_.GetJoints().size()));

    currentAnimation_ = std::make_unique<ModelAnimation>();
    currentAnimation_->Initialize();


    skeleton_.Update();
    skinCluster_.Update(skeleton_.GetJoints());
    if (skinningCS_)
        skinningCS_->Execute();
}

void AnimationController::Update(float _deltaTime)
{
    if (!model_ || !currentAnimation_)
        return;

    currentAnimation_->Update(skeleton_.GetJoints(), _deltaTime);

    // アニメーションが終わったらアニメーションを解除
    if (!currentAnimation_->IsPlaying())
    {
        return;
    }

    skeleton_.Update();
    skinCluster_.Update(skeleton_.GetJoints());
    if (skinningCS_)
        skinningCS_->Execute();
}

void AnimationController::SetAnimation(const std::string& _name, bool _loop)
{
    if (_name.empty())
        return;

    if (!currentAnimation_)
        return;

    auto animation = model_->GetAnimation(_name);

    if (!animation)
        return;

    currentAnimation_->Reset();
    currentAnimation_->SetAnimation(animation->GetAnimation());
    currentAnimation_->SetLoop(_loop);
    currentAnimation_->Update(skeleton_.GetJoints(), 0.0f);
}

void AnimationController::ChangeAnimation(const std::string& _name, float _blendTime, bool _loop)
{
    if (_name.empty())
        return;

    if (!currentAnimation_)
        return;

    auto animation = model_->GetAnimation(_name);
    if (!animation)
        return;

    currentAnimation_->Reset();
    currentAnimation_->ChangeAnimation(animation->GetAnimation(), _blendTime);
    currentAnimation_->SetLoop(_loop);
}

void AnimationController::StopAnimation()
{
    if (!currentAnimation_)
        return;

    currentAnimation_->Reset();
    currentAnimation_->SetLoop(false);
    currentAnimation_->ToIdle(0.0f); // アイドル状態にする
}

void AnimationController::DrawSkeleton(const Matrix4x4& _worldMat)
{
    if (!currentAnimation_)
        return;
    skeleton_.Draw(_worldMat);
}

void AnimationController::ToIdle(float _timeToIdle)
{
    if (!currentAnimation_)
        return;

    currentAnimation_->ToIdle(_timeToIdle);
}

const Matrix4x4* AnimationController::GetSkeletonSpaceMatrix(uint32_t _index) const
{
    return skeleton_.GetSkeletonSpaceMatrix(_index);
}

bool AnimationController::IsAnimationPlaying() const
{
    return currentAnimation_ && currentAnimation_->IsPlaying();
}

const Matrix4x4* AnimationController::GetSkeletonSpaceMatrix(const std::string& _name) const
{
    return skeleton_.GetSkeletonSpaceMatrix(_name);
}

void AnimationController::ImGui()
{
#ifdef _DEBUG

    skeleton_.ImGui();


#endif // _DEBUG
}
