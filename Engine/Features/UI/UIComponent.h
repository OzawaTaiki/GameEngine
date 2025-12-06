#pragma once


class UIElement;

class UIComponent
{
public:
    UIComponent() = default;
    virtual ~UIComponent() = default;

    virtual void Initialize() {}
    virtual void Update() {}
    virtual void Draw() {}

    virtual void DrawImGui() {};

    //------------------
    // 状態

    void SetEnabled(bool isEnabled) { isEnabled_ = isEnabled; }
    bool IsEnabled() const { return isEnabled_; }

    //------------------
    // オーナー設定
    void SetOwner(UIElement* owner) { owner_ = owner; }
    UIElement* GetOwner() const { return owner_; }

protected:

    UIElement* owner_ = nullptr;

    bool isEnabled_ = true;
};