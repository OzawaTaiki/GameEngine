#include "TextGenerator.h"

#include <Features/TextRenderer/FontCache.h>
#include <Features/TextRenderer/TextRenderer.h>

void TextGenerator::Initialize(const FontConfig& _config)
{
    auto fontCache = FontCache::GetInstance();

    atlasData_ = fontCache->GetAtlasData(_config.fontFilePath, _config.fontSize);

    renderer_ = TextRenderer::GetInstance();
}

void TextGenerator::Draw(const std::wstring& _text, const Vector2& _pos, const Vector4& _color)
{
    renderer_->DrawText(_text, atlasData_,_pos, _color);
}

void TextGenerator::Draw(const std::wstring& _text, const TextParam& _param)
{
    renderer_->DrawText(_text, atlasData_, _param);
}
