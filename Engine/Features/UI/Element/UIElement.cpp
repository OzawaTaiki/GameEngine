#include "UIElement.h"
#include <Math/Matrix/MatrixFunction.h>
#include <Debug/ImGuiDebugManager.h>


namespace Engine {

UIElement::UIElement(const std::string& name, [[maybe_unused]]bool child):
    name_(name),
    parent_(nullptr),
    position_(50.0f, 50.0f),
    size_(100.0f, 50.0f),
    rotation_(0.0f),
    scale_(1.0f, 1.0f),
    pivot_(0.5f, 0.5f),
    anchor_(0.5f, 0.5f),
    order_(0),
    isVisible_(true),
    isEnabled_(true),
    worldMatrix_(MakeIdentity4x4())
{
#ifdef _DEBUG
    if (child)
        return;
    ImGuiDebugManager::GetInstance()->AddDebugWindow(
        "UIElement: " + name_,
        [this]() { DrawImGuiInspector(); }
    );
#endif

}

UIElement::~UIElement()
{
#ifdef _DEBUG
    ImGuiDebugManager::GetInstance()->RemoveDebugWindow("UIElement: " + name_);
#endif
}

void UIElement::Initialize()
{
    InitializeJsonBinder();
}

void UIElement::Update()
{
    // 親より先に行列を更新（WorldTransform::UpdateData() 相当）
    // isEnabled に関わらず実行し、子への伝播を保証する
    UpdateMatrix();

    if (!isEnabled_)
        return;

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

void UIElement::UpdateMatrix()
{
    Vector2 t;
    if (parent_ == nullptr)
    {
        t = position_;
    }
    else
    {
        // 親の pivot 基準でアンカー位置を求める（UI 固有処理）
        Vector2 anchorOffset = (anchor_ - parent_->GetPivot()) * parent_->GetSize();
        t = anchorOffset + position_;
    }

    Vector3 s     = { scale_.x, scale_.y, 1.0f };
    Vector3 r     = { 0.0f, 0.0f, rotation_ };
    Vector3 trans = { t.x, t.y, 0.0f };
    worldMatrix_  = MakeAffineMatrix(s, r, trans);

    // 親行列を右から掛ける（WorldTransform の matWorld_ *= parent_->matWorld_ 相当）
    if (parent_ != nullptr)
    {
        worldMatrix_ *= parent_->worldMatrix_;
    }
}

Vector2 UIElement::GetWorldPosition() const
{
    return { worldMatrix_.m[3][0], worldMatrix_.m[3][1] };
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
#ifdef _DEBUG
    // ツリーノードを表示（クリックで展開/折りたたみ）
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
    if (children_.empty())
        flags |= ImGuiTreeNodeFlags_Leaf;

    bool node_open = ImGui::TreeNodeEx(name_.c_str(), flags);

    // クリックで選択（Inspectorに表示する対象を記録）
    if (ImGui::IsItemClicked())
    {
        // TODO: 選択状態を保存（後で実装）
    }

    if (node_open)
    {
        // 子要素を再帰的に表示
        for (auto& child : children_)
        {
            child->DrawImGuiTree();
        }
        ImGui::TreePop();
    }
#endif
}

void UIElement::DrawImGuiInspector()
{
#ifdef _DEBUG
    ImGui::PushID(this);
    if(ImGui::TreeNode("baseElement"))
    {
        ImGui::Text("UIElement: %s", name_.c_str());
        ImGui::SeparatorText("Transform");
        ImGui::Indent();
        {
            ImGui::DragFloat2("Position", &position_.x, 1.0f);
            ImGui::DragFloat2("Size", &size_.x, 1.0f);
            ImGui::DragFloat2("Scale", &scale_.x, 0.01f);
            ImGui::DragFloat("Rotation", &rotation_, 0.01f);
            ImGui::DragFloat2("Pivot", &pivot_.x, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat2("Anchor", &anchor_.x, 0.01f, 0.0f, 1.0f);
        }
        ImGui::Unindent();

        int order = order_;
        if (ImGui::InputInt("Order", &order))
        {
            order_ = static_cast<uint16_t>(order);
        }

        ImGui::Checkbox("Visible", &isVisible_);
        ImGui::Checkbox("Enabled", &isEnabled_);

        ImGui::Text("Parent : %s", parent_ ? parent_->GetName().c_str() : "None");
        ImGui::SeparatorText("Children");
        for (auto& child : children_)
        {
            child->DrawImGuiInspector();
        }
        ImGui::SeparatorText("Components");
        for (auto& component : components_)
        {
            component->DrawImGui();
        }

        // 保存ボタン
        if (jsonBinder_)
        {
            ImGui::SeparatorText("Save");
            if (ImGui::Button("Save"))
            {
                Save();
            }
        }
        else
        {
            ImGui::TextWrapped("JsonBinder is not initialized. Cannot save data.");
            ImGui::TextWrapped("need to call InitializeJsonBinder() in Initialize().");
        }
        ImGui::TreePop();
    }
    ImGui::PopID();

#endif
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

void UIElement::InitializeJsonBinder(const std::string& directory)
{
    jsonBinder_ = std::make_unique<JsonBinder>(name_, directory);

    RegisterVariable("position", &position_);
    RegisterVariable("size", &size_);
    RegisterVariable("rotation", &rotation_);
    RegisterVariable("scale", &scale_);
    RegisterVariable("pivot", &pivot_);
    RegisterVariable("anchor", &anchor_);
    RegisterVariable("order", &order_);
    RegisterVariable("isVisible", &isVisible_);
    RegisterVariable("isEnabled", &isEnabled_);
}

void UIElement::Save()
{
    if (jsonBinder_)
        jsonBinder_->Save();

    for (auto& component : components_)
        component->Save();

    for (auto& child : children_)
        child->Save();

}

} // namespace Engine
