#include "TextGenerator.h"

#include <Features/TextRenderer/FontCache.h>
#include <Features/TextRenderer/TextRenderer.h>
#include <Utility/ConvertString/ConvertString.h>

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

void TextGenerator::Draw(const std::wstring& _text, const Rect& _rect, const Vector2& _pos, const Vector2& _piv, const Vector4& _color, uint16_t _order)
{
    renderer_->DrawText(_text, atlasData_, _rect, _pos, _piv, _order, _color);
}


void TextGenerator::Draw(const std::wstring& _text, const Rect& _rect, const TextParam& _param, uint16_t _order)
{
    renderer_->DrawText(_text, atlasData_, _rect, _param, _order);
}

// -- -----------------
// -- 静的関数たち
// -- -----------------

void TextGenerator::Draw(const std::string& text, const FontConfig& conf, const Vector2& pos, const Vector4& color, uint16_t order)
{
    // string を wstring に変換
    std::wstring wtext = ConvertString(text);

    // FontCacheからAtlasDataを取得
    auto fontCache = FontCache::GetInstance();
    AtlasData* atlasData = fontCache->GetAtlasData(conf.fontFilePath, conf.fontSize);

    // TextRendererで描画
    auto renderer = TextRenderer::GetInstance();
    renderer->DrawText(wtext, atlasData, pos, order, color);
}

void TextGenerator::Draw(const std::string& text, const FontConfig& conf, const TextParam& param, uint16_t order)
{
    // string を wstring に変換
    std::wstring wtext = ConvertString(text);

    // FontCacheからAtlasDataを取得
    auto fontCache = FontCache::GetInstance();
    AtlasData* atlasData = fontCache->GetAtlasData(conf.fontFilePath, conf.fontSize);

    // TextRendererで描画
    auto renderer = TextRenderer::GetInstance();
    renderer->DrawText(wtext, atlasData, param, order);
}

void TextGenerator::Draw(const std::string& text, const FontConfig& conf, const Rect& rect, const Vector2& pos, const Vector2& piv, const Vector4& color, uint16_t order)
{
    // string を wstring に変換
    std::wstring wtext = ConvertString(text);

    // FontCacheからAtlasDataを取得
    auto fontCache = FontCache::GetInstance();
    AtlasData* atlasData = fontCache->GetAtlasData(conf.fontFilePath, conf.fontSize);

    // TextRendererで描画
    auto renderer = TextRenderer::GetInstance();
    renderer->DrawText(wtext, atlasData, rect, pos, piv, order, color);
}

void TextGenerator::Draw(const std::string& text, const FontConfig& conf, const Rect& rect, const TextParam& param, uint16_t order)
{
    // string を wstring に変換
    std::wstring wtext = ConvertString(text);

    // FontCacheからAtlasDataを取得
    auto fontCache = FontCache::GetInstance();
    AtlasData* atlasData = fontCache->GetAtlasData(conf.fontFilePath, conf.fontSize);

    // TextRendererで描画
    auto renderer = TextRenderer::GetInstance();
    renderer->DrawText(wtext, atlasData, rect, param, order);
}

