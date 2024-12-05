#pragma once

#include "UIBase.h"

class UISprite : public UIBase
{
public:

    UISprite() = default;
    ~UISprite() = default;

    void Initialize(const std::string& _label);
    void Update() override;
    void Draw() override;


private:


};