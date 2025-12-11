#include "UIElement.h"
#include <Debug/ImGuiDebugManager.h>

UIElement::UIElement(const std::string& name, [[maybe_unused]]bool child):
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

    for (auto& child : children_)
        child->Save();
}
