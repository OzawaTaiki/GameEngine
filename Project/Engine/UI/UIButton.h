#pragma once

#include "UIBase.h"


class UIButton :public UIBase
{
public:

    UIButton() = default;
    ~UIButton() = default;

    void Initialize(const std::string& _label);
    void Update() override;
    void Draw() override;

    bool IsClicked() const;

    Vector2 GetSize() const { return size_; }

private:


};


