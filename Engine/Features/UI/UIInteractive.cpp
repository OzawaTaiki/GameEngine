#include "UIInteractive.h"
#include <Features/UI/Collider/UIRentangleCollider.h>
#include <Features/UI/Collider/UIColliderFactory.h>

#include <System/Input/Input.h>


UIInteractive::UIInteractive()
{
    // デフォルトで矩形コライダーを使用
    collider_ = std::make_unique<UIRentangleCollider>();
    colliderType_ = ColliderType::Rectangle;
}

void UIInteractive::UpdateSelf()
{
    // Colliderのキャッシュを更新
    if (collider_)
    {
        collider_->UpdateCache(this);
    }

    Input* input = Input::GetInstance();

    // マウスポインタがUI要素内にあるか判定
    wasHovered_ = isHovered_;
    isHovered_ = IsMousePointerInside();

    // ホバー状態の変化を検出
    if (isHovered_ && !wasHovered_)
    {
        // ホバー開始
        OnHoverEnter();
    }
    if (isHovered_)
    {
        // ホバー中
        OnHovering();
    }
    if (!isHovered_ && wasHovered_)
    {
        // ホバー終了
        OnHoverExit();
    }

    if (isHovered_)
    {
        // マウスがUI要素内にある場合の処理
        if (input->IsMouseTriggered(0))
        {
            isPressed_ = true;
            OnMouseDown();
        }
        if (input->IsMouseReleased(0))
        {
            if (isPressed_)
            {
                OnClick();
                OnMouseUp();
                isPressed_ = false;
            }
        }
    }
    else
    {
        // マウスがUI要素外にある場合の処理
        if (input->IsMouseReleased(0))
        {
            // 押下状態をリセット
            // コールバックは呼ばない
            isPressed_ = false;
        }
    }
}

void UIInteractive::SetCollider(std::unique_ptr<IUICollider> _collider)
{
    collider_ = std::move(_collider);
    if (collider_)
    {
        collider_->UpdateCache(this);
    }
}

bool UIInteractive::IsMousePointerInside() const
{
    Vector2 mousePos = Input::GetInstance()->GetMousePosition();

    // Colliderで判定
    if (collider_)
    {
        return collider_->IsPointInside(mousePos);
    }

    // Colliderがない場合はfalse（通常はありえない）
    return false;
}


void UIInteractive::ImGuiContent()
{
#ifdef _DEBUG
    if (ImGui::TreeNode("UIInteractive"))
    {
        ImGui::Text("Functions:");
        std::string str = onClickCallback_ ? " [Set]" : " [Not Set]";
        ImGui::Text((" - OnClick()" + str).c_str());
        str = onHoverEnterCallback_ ? " [Set]" : " [Not Set]";
        ImGui::Text((" - OnHoverEnter()" + str).c_str());
        str = onHoveringCallback_ ? " [Set]" : " [Not Set]";
        ImGui::Text((" - OnHovering()" + str).c_str());
        str = onHoverExitCallback_ ? " [Set]" : " [Not Set]";
        ImGui::Text((" - OnHoverExit()" + str).c_str());

        ImGui::TreePop();
    }

    // Colliderのデバッグ表示と選択UI
    if (ImGui::TreeNode("Collider"))
    {
        // ファクトリを使ってColliderタイプを選択
        auto newCollider = UIColliderFactory::ImGuiSelectCollider(colliderType_);
        if (newCollider)
        {
            SetCollider(std::move(newCollider));
        }

        // 現在のColliderのパラメータ表示
        if (collider_)
        {
            ImGui::Separator();
            ImGui::Text("Current Parameters:");
            collider_->ImGui();
        }

        ImGui::TreePop();
    }
#endif // _DEBUG
}

void UIInteractive::OnClick()
{
    if (onClickCallback_)
    {
        onClickCallback_();
    }
}

void UIInteractive::OnHoverEnter()
{
    if (onHoverEnterCallback_)
    {
        onHoverEnterCallback_();
    }
}

void UIInteractive::OnHovering()
{
    if (onHoveringCallback_)
    {
        onHoveringCallback_();
    }
}

void UIInteractive::OnHoverExit()
{
    if (onHoverExitCallback_)
    {
        onHoverExitCallback_();
    }
}

void UIInteractive::OnMouseDown()
{
    // 派生クラスでオーバーライド可能
}

void UIInteractive::OnMouseUp()
{
    // 派生クラスでオーバーライド可能
}
