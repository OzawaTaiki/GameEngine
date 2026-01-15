#include "UISliderComponent.h"
#include <Features/UI/UIElement.h>
#include <Features/UI/Collider/UIColliderComponent.h>
#include <Features/UI/Collider/UICollisionManager.h>
#include <System/Input/Input.h>
#include <algorithm>

#ifdef _DEBUG
#include <Debug/ImGuiDebugManager.h>
#endif


namespace Engine {

void UISliderComponent::Initialize()
{
    owner_->RegisterVariable("value", &value_);
    owner_->RegisterVariable("minValue", &minValue_);
    owner_->RegisterVariable("maxValue", &maxValue_);
    owner_->RegisterVariable("isEnabled", &isEnabled_);
    owner_->RegisterVariable("Step", &step_);

    // 初期ハンドル位置を設定
    UpdateHandlePosition();
}

void UISliderComponent::Update()
{
    if (!isEnabled_ || !track_ || !handle_)
        return;

    auto* input = Input::GetInstance();
    Vector2 mousePos = input->GetMousePosition();

    // ドラッグ開始判定
    if (input->IsMouseTriggered(static_cast<uint8_t>(MouseButton::Left)))
    {
        if (IsMouseOverHandle(mousePos))
        {
            isDragging_ = true;
        }
        else if (IsMouseOverTrack(mousePos))
        {
            // トラッククリック：その位置に即座に移動
            UpdateValueFromMousePosition(mousePos);
            isDragging_ = true;
        }
    }

    // ドラッグ中
    if (isDragging_)
    {
        if (input->IsMouseTriggered(static_cast<uint8_t>(MouseButton::Left))|| 
            input->IsMousePressed(static_cast<uint8_t>(MouseButton::Left)))
        {
            UpdateValueFromMousePosition(mousePos);
        }
        else
        {
            // ドラッグ終了
            isDragging_ = false;
        }
    }
}

void UISliderComponent::DrawImGui()
{
#ifdef _DEBUG
    ImGui::PushID(this);
    if (ImGui::TreeNode("UISliderComponent"))
    {
        if (ImGui::SliderFloat("Value", &value_, minValue_, maxValue_))
        {
            SetValue(value_);
        }
        if(ImGui::DragFloatRange2("Range", &minValue_, &maxValue_, 0.1f))
        {
            SetRange(minValue_, maxValue_);
        }
        ImGui::Checkbox("Enabled", &isEnabled_);
        ImGui::BeginDisabled(1);
        ImGui::Checkbox("Is Dragging", &isDragging_);
        ImGui::EndDisabled();


        ImGui::TreePop();
    }
    ImGui::PopID();
#endif
}

void UISliderComponent::SetValue(float value)
{
    // 範囲制限
    float newValue = std::clamp(value, minValue_, maxValue_);
    // ステップに基づいて値を調整
    if (step_ > 0.0f)
    {
        newValue = std::round(newValue / step_) * step_;
    }

    if (value_ != newValue)
    {
        value_ = newValue;
        UpdateHandlePosition();

        // コールバック呼び出し
        if (onValueChanged_)
        {
            onValueChanged_(value_);
        }
    }
}

void UISliderComponent::SetRange(float min, float max)
{
    minValue_ = min;
    maxValue_ = max;

    // 現在の値を範囲内に制限
    SetValue(value_);
}

void UISliderComponent::UpdateValueFromMousePosition(const Vector2& mousePos)
{
    if (!track_)
        return;

    // トラックのワールド座標とサイズを取得
    Vector2 trackPos = track_->GetWorldPosition();
    Vector2 trackSize = track_->GetSize();
    Vector2 trackPivot = track_->GetPivot();

    // トラックの左端と右端を計算
    float trackLeft = trackPos.x - trackPivot.x * trackSize.x;
    float trackRight = trackPos.x + (1.0f - trackPivot.x) * trackSize.x;

    // マウス位置からトラック内の相対位置を計算 (0.0 ~ 1.0)
    float normalizedPos = (mousePos.x - trackLeft) / (trackRight - trackLeft);
    normalizedPos = std::clamp(normalizedPos, 0.0f, 1.0f);

    // 正規化位置から値を計算
    float newValue = minValue_ + normalizedPos * (maxValue_ - minValue_);
    // ステップに基づいて値を調整
    if (step_ > 0.0f)
    {
        newValue = std::round(newValue / step_) * step_;
    }
    SetValue(newValue);
}

void UISliderComponent::UpdateHandlePosition()
{
    if (!handle_ || !track_)
        return;

    // 値から正規化位置を計算 (0.0 ~ 1.0)
    float normalizedValue = (value_ - minValue_) / (maxValue_ - minValue_);

    // トラックのサイズを取得
    Vector2 trackSize = track_->GetSize();

    // ハンドルの新しいX座標を計算
    float handleX = trackSize.x * normalizedValue;
        //trackLeft + normalizedValue * (trackRight - trackLeft);

    // ハンドルの位置を更新（Y座標はトラックの中央）
    Vector2 handlePos;
    handlePos.x = handleX;
    handlePos.y = 0.0f;

    handle_->SetPosition(handlePos);
}

bool UISliderComponent::IsMouseOverHandle(const Vector2& mousePos) const
{
    if (!handle_)
        return false;

    auto* collider = handle_->GetComponent<UIColliderComponent>();
    if (!collider || !collider->GetCollider())
        return false;

    return collider->GetCollider()->IsPointInside(mousePos);
}

bool UISliderComponent::IsMouseOverTrack(const Vector2& mousePos) const
{
    if (!track_)
        return false;

    auto* collider = track_->GetComponent<UIColliderComponent>();
    if (!collider || !collider->GetCollider())
        return false;

    return collider->GetCollider()->IsPointInside(mousePos);
}

} // namespace Engine
