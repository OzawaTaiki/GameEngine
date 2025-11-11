#include "TextGenerator.h"

#include <Features/TextRenderer/FontCache.h>
#include <Features/TextRenderer/TextRenderer.h>

void TextGenerator::Initialize(const FontConfig& _config)
{
    auto fontCache = FontCache::GetInstance();

    atlasData_ = fontCache->GetAtlasData(_config.fontFilePath, _config.fontSize);

    renderer_ = TextRenderer::GetInstance();
}

void TextGenerator::Draw(const std::wstring& _text, const Vector2& _pos, const Vector4& _color, uint16_t _order)
{
    renderer_->DrawText(_text, atlasData_, _pos, _order, _color);
}

void TextGenerator::Draw(const std::wstring& _text, const TextParam& _param, uint16_t _order)
{
    renderer_->DrawText(_text, atlasData_, _param, _order);
}
//
//void TextGenerator::Draw(const std::wstring& _text, const Rect& _rect, const TextParam& _param, uint16_t _order)
//{
//    renderer_->DrawText(_text, atlasData_, _rect, _param, _order);
//}
