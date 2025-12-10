#include "UIElement.h"

UIElement::UIElement(const std::string& name):
    name_(name),
    parent_(nullptr),
    position_(50.0f, 50.0f),
    size_(100.0f, 50.0f),
    pivot_(0.5f, 0.5f),
    anchor_(0.5f, 0.5f),
    order_(0),
    isVisible_(true),
    isEnabled_(true)
{
}

UIElement::~UIElement()
{
}

void UIElement::Update()
{
    for (auto& component : components_)
    {
        component->Update();
    }
    for (auto& child : children_)
    {
        child->Update();
    }
}

void UIElement::Draw()
{
    if (!isVisible_)
        return;

    // 全コンポーネント描画（追加順に描画される）
    for (auto& component : components_)
    {
        component->Draw();
    }

    // 全子要素描画
    for (auto& child : children_)
    {
        child->Draw();
    }
}

Vector2 UIElement::GetWorldPosition() const
{
    if (parent_ == nullptr)
    {
        return position_;
    }

    // 親要素のサイズを取得
    Vector2 parentSize = parent_->GetSize();
    // アンカーオフセットを計算
    Vector2 anchorOffset = CalculateAnchorOffset(anchor_, parentSize);
    // 親要素のLTワールド座標を取得
    Vector2 parentLTWorldPos = parent_->GetWorldPosition() - Vector2(parentSize.x * parent_->GetPivot().x,
                                                                     parentSize.y * parent_->GetPivot().y);
    // ワールド座標を計算
    Vector2 worldPos = parentLTWorldPos + anchorOffset + position_;
    return worldPos;
}

void UIElement::SetParent(UIElement* parent)
{
    if (parent_ != nullptr)
    {
        // 以前の親要素から自分を削除
        parent_->RemoveChild(this);
    }
    parent_ = parent;
}

void UIElement::RemoveParent()
{
    if (parent_ == nullptr)
    {
        return;
    }

    parent_->RemoveChild(this);
    parent_ = nullptr;
}

UIElement* UIElement::AddChild(std::unique_ptr<UIElement> child)
{
    child->SetParent(this);
    children_.push_back(std::move(child));

    return children_.back().get();
}


std::unique_ptr<UIElement> UIElement::RemoveChild(UIElement* child)
{
    auto it = std::remove_if(children_.begin(), children_.end(),
                             [child](const std::unique_ptr<UIElement>& ptr)
                             {
                                 return ptr.get() == child;
                             });

    if (it != children_.end())
    {
        children_.erase(it, children_.end());
        child->SetParent(nullptr);

        return std::move(*it); // 所有権を返す
    }
    return nullptr;
}

void UIElement::DrawImGuiTree()
{
}

void UIElement::DrawImGuiInspector()
{
}

Vector2 UIElement::CalculateAnchorOffset(const Vector2& anchor, const Vector2& parentSize) const
{
    return Vector2(parentSize.x * anchor.x, parentSize.y * anchor.y);
}

Vector2 UIElement::CalculateAnchorOffset(Anchor anchor, const Vector2& parentSize) const
{
    Vector2 anchorVec = GetAnchorVector(anchor);
    return CalculateAnchorOffset(anchorVec, parentSize);
}

Vector2 UIElement::GetAnchorVector(Anchor anchor) const
{
    switch (anchor)
    {
        case Anchor::TopLeft:       return Vector2(0.0f, 0.0f);
        case Anchor::TopCenter:     return Vector2(0.5f, 0.0f);
        case Anchor::TopRight:      return Vector2(1.0f, 0.0f);
        case Anchor::CenterLeft:    return Vector2(0.0f, 0.5f);
        case Anchor::Center:        return Vector2(0.5f, 0.5f);
        case Anchor::CenterRight:   return Vector2(1.0f, 0.5f);
        case Anchor::BottomLeft:    return Vector2(0.0f, 1.0f);
        case Anchor::BottomCenter:  return Vector2(0.5f, 1.0f);
        case Anchor::BottomRight:   return Vector2(1.0f, 1.0f);
        default:                    return Vector2(0.0f, 0.0f);
    };
}
