#pragma once

#include "UIBase.h"

class UISprite : public UIBase
{
public:

    UISprite() = default;
    ~UISprite() = default;

    void Initialize(const std::string& _label);
    void Initialize(const std::string& _label, const std::wstring& _text);
    void Update() override;
    void Draw() override;

    void SetText(const std::wstring& _text) { text_ = _text; }
    const std::wstring& GetText() const { return text_; }
    void SetTextParam(const TextParam& _param) { textParam_ = _param; }
    void SetTextOffset(const Vector2& _offset) { textOffset_ = _offset; }



private:

    TextGenerator textGenerator_;
    std::wstring text_ = L"";
    TextParam textParam_;
    Vector2 textOffset_ = { 0,0 };
};
