#pragma once
#include <Features/UI/Component/UIComponent.h>
#include <Math/Vector/Vector2.h>
#include <Features/Json/JsonBinder.h>

#include <typeinfo>
#include <algorithm>
#include <string>
#include <vector>
#include <memory>

// 前方宣言

namespace Engine {

class UIComponent;

// Anchor定義
enum class Anchor
{
    TopLeft, TopCenter, TopRight,
    CenterLeft, Center, CenterRight,
    BottomLeft, BottomCenter, BottomRight
};

class UIElement
{
public:
    UIElement(const std::string& name,bool child = false);
    virtual ~UIElement();

    virtual void Initialize();
    virtual void Update();
    virtual void Draw();

    //------------------
    // レイアウト
    void SetPosition(const Vector2& position) { position_ = position; }
    const Vector2& GetPosition() const { return position_; }

    void SetSize(const Vector2& size) { size_ = size; }
    const Vector2& GetSize() const { return size_; }
    bool HasSize() const { return size_.x > 0 && size_.y > 0; }

    void SetPivot(const Vector2& pivot) { pivot_ = pivot; }
    const Vector2& GetPivot() const { return pivot_; }

    void SetAnchor(Vector2 anchor) { anchor_ = anchor; }
    Vector2 GetAnchor() const { return anchor_; }

    void SetOrder(uint16_t order) { order_ = order; }
    uint16_t GetOrder() const { return order_; }

    //------------------
    // 状態
    void SetVisible(bool isVisible) { isVisible_ = isVisible; }
    bool IsVisible() const { return isVisible_; }

    void SetEnabled(bool isEnabled) { isEnabled_ = isEnabled; }
    bool IsEnabled() const { return isEnabled_; }

    //------------------
    // 名前
    void SetName(const std::string& name) { name_ = name; }
    const std::string& GetName() const { return name_; }

    //------------------
    // ワールド座標の取得
    Vector2 GetWorldPosition() const;

    //------------------
    // 親子関係
    bool HasParent() const { return parent_ != nullptr; }
    bool HasChildren() const { return !children_.empty(); }
    size_t GetChildCount() const { return children_.size(); }

    void SetParent(UIElement* parent);
    UIElement* GetParent() const { return parent_; }
    void RemoveParent();

    UIElement* AddChild(std::unique_ptr<UIElement> child);  // 所有権を受け取る
    const std::vector<std::unique_ptr<UIElement>>& GetChildren() const { return children_; }
    std::unique_ptr<UIElement> RemoveChild(UIElement* child);

    //------------------
    // Component管理
    template<typename T, typename... Args>
    T* AddComponent(Args&&... args);

    template<typename T>
    T* GetComponent() const;

    template<typename T>
    bool HasComponent() const;

    template<typename T>
    void RemoveComponent();

    //------------------
    // ImGui
    virtual void DrawImGuiTree();
    virtual void DrawImGuiInspector();

    //------------------
    // 保存機能（JsonBinder）
    void InitializeJsonBinder(const std::string& directory = "Resources/Data/UI/");

    template<typename T>
    void RegisterVariable(const std::string& name, T* variablePtr);

    template<typename T>
    void GetVariableValue(const std::string& name, T& outValue) const;

    template<typename T>
    void SetVariableValue(const std::string& name, const T& value);
    void Save();

protected:
    // 名前
    std::string name_;

    // 親子関係
    UIElement* parent_ = nullptr;
    std::vector<std::unique_ptr<UIElement>> children_;

    // Component
    std::vector<std::unique_ptr<UIComponent>> components_;

    // 保存機能
    std::unique_ptr<JsonBinder> jsonBinder_;

    // レイアウト
    Vector2 position_;
    Vector2 size_;
    Vector2 pivot_;
    Vector2 anchor_;

    uint16_t order_; // 描画順序

    // 状態
    bool isVisible_;
    bool isEnabled_;


private:
    // Anchor計算用
    Vector2 CalculateAnchorOffset(const Vector2& anchor, const Vector2& parentSize) const;
    Vector2 CalculateAnchorOffset(Anchor anchor, const Vector2& parentSize) const;
    Vector2 GetAnchorVector(Anchor anchor) const;
};

template<typename T, typename... Args>
inline T* UIElement::AddComponent(Args&&... args)
{
    static_assert(std::is_base_of<UIComponent, T>::value,
                  "T must inherit from UIComponent");

    auto component = std::make_unique<T>(std::forward<Args>(args)...);
    component->SetOwner(this);

    T* ptr = component.get();
    components_.push_back(std::move(component));

    // Initialize呼び出し
    ptr->Initialize();

    return ptr;
}

template<typename T>
inline T* UIElement::GetComponent() const
{
    for (const auto& component : components_)
    {
        T* casted = dynamic_cast<T*>(component.get());
        if (casted)
        {
            return casted;
        }
    }
    return nullptr;
}

template<typename T>
inline bool UIElement::HasComponent() const
{
    return GetComponent<T>() != nullptr;
}

template<typename T>
inline void UIElement::RemoveComponent()
{
    auto it = std::remove_if(components_.begin(), components_.end(),
                             [](const std::unique_ptr<UIComponent>& component)
                             {
                                 return dynamic_cast<T*>(component.get()) != nullptr;
                             });
    components_.erase(it, components_.end());
}

template<typename T>
inline void UIElement::RegisterVariable(const std::string& name, T* variablePtr)
{
    if (jsonBinder_)
        jsonBinder_->RegisterVariable(name, variablePtr);
}

template<typename T>
inline void UIElement::GetVariableValue(const std::string& name, T& outValue) const
{
    if (jsonBinder_)
        jsonBinder_->GetVariableValue(name, outValue);
}

template<typename T>
inline void UIElement::SetVariableValue(const std::string& name, const T& value)
{
    if (jsonBinder_)
        jsonBinder_->SendVariable(name, value);
}

} // namespace Engine
