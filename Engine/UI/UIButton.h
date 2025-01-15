#pragma once

#include "UIBase.h"

enum class PadButton;

class UIButton :public UIBase
{
public:

    UIButton() = default;
    ~UIButton() = default;

    void Initialize(const std::string& _label);
    void Update() override;
    void Draw() override;

    bool IsPressed() const;
    bool IsPressed(PadButton _button) const;

    Vector2 GetSize() const { return size_; }

private:

    std::string group_ = "";


};


