#include <Features/Model/AnimationModel.h>

#include <Features/Model/Manager/ModelManager.h>
#include <Core/DXCommon/DXCommon.h>
#include <Debug/ImGuiDebugManager.h>

AnimationModel::AnimationModel(const std::string& _name)
{
    name_ = ImGuiDebugManager::GetInstance()->AddDebugWindow(_name, [&]() {ImGui(); });
}

AnimationModel::~AnimationModel()
{
    ImGuiDebugManager::GetInstance()->RemoveDebugWindow(name_);
}

void AnimationModel::Initialize(const std::string& _filePath)
{
    model_ = Model::CreateFromObj(_filePath);

    worldTransform_.Initialize();
    objectColor_ = std::make_unique<ObjectColor>();
    objectColor_->Initialize();

    gameTime_ = GameTime::GetInstance();
}

void AnimationModel::Update()
{
    model_->Update(gameTime_->GetChannel(timeChannel).GetDeltaTime<float>());

    worldTransform_.transform_ = translate_;
    worldTransform_.scale_ = scale_;

    if (useQuaternion_)
        worldTransform_.quaternion_ = quaternion_;
    else
        worldTransform_.rotate_ = euler_;

    worldTransform_.UpdateData(useQuaternion_);
}

void AnimationModel::Draw(const Camera* _camera, const Vector4& _color)
{
    objectColor_->SetColor(_color);

    ModelManager::GetInstance()->PreDrawForAnimationModel();

    auto commandList = DXCommon::GetInstance()->GetCommandList();
    _camera->QueueCommand(commandList, 0);
    worldTransform_.QueueCommand(commandList, 1);
    objectColor_->QueueCommand(commandList, 3);
    model_->QueueCommandAndDraw(commandList, true);// BVB IBV MTL2 TEX4 LIGHT567

    //model_->DrawSkeleton(worldTransform_.matWorld_);
}

void AnimationModel::Draw(const Camera* _camera, uint32_t _textureHandle, const Vector4& _color)
{
    objectColor_->SetColor(_color);

    ModelManager::GetInstance()->PreDrawForAnimationModel();

    auto commandList = DXCommon::GetInstance()->GetCommandList();
    _camera->QueueCommand(commandList, 0);
    worldTransform_.QueueCommand(commandList, 1);
    objectColor_->QueueCommand(commandList, 3);
    model_->QueueCommandAndDraw(commandList, _textureHandle, true);// BVB IBV MTL2 TEX4 LIGHT567
}

void AnimationModel::SetAnimation(const std::string& _name,bool _isLoop)
{
    model_->SetAnimation(_name,_isLoop);
}


void AnimationModel::ImGui()
{
#ifdef _DEBUG
    ImGui::PushID(this);
    ImGui::DragFloat3("Translate", &translate_.x, 0.01f);
    ImGui::DragFloat3("Scale", &scale_.x, 0.01f);
    if (useQuaternion_)
        ImGui::DragFloat4("Quaternion", &quaternion_.x, 0.01f);
    else
        ImGui::DragFloat3("Rotate", &euler_.x, 0.01f);
    ImGui::Checkbox("UseQuaternion", &useQuaternion_);
    ImGui::PopID();
#endif // _DEBUG
}
