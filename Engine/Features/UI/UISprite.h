#pragma once

#include "UIBase.h"
#include "UIText.h"


namespace Engine {

class UISprite final : public UIBase
{
public:

    UISprite() = default;
    ~UISprite() = default;

    void Initialize(const std::string& _label);
    void Initialize(const std::string& _label, const std::wstring& _text);
    void Update() override;
    void Draw() override;

    UIText* GetTextObject() { return textUI_.get(); }

private:

    std::shared_ptr<UIText> textUI_ = nullptr;
};

} // namespace Engine
