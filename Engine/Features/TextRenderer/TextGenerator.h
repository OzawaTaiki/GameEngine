#pragma once

#include <Features/TextRenderer/AtlasData.h>
#include <Features/TextRenderer/TextParam.h>
#include <Math/Rect/Rect.h>


#ifdef DrawText
#undef DrawText
#endif // DrawText


class TextRenderer;
class TextGenerator
{
public:

    void Initialize(const FontConfig& _config);

    void Draw(const std::wstring& _text, const Vector2& _pos, const Vector4& _color = { 1,1,1,1 }, uint16_t _order = 0);

    void Draw(const std::wstring& _text, const TextParam& _param, uint16_t _order = 0);

    //void Draw(const std::wstring& _text, const Rect& _rect, const TextParam& _param, uint16_t _order = 0);

    const AtlasData* GetAtlasData() const { return atlasData_; }
private:

    TextRenderer* renderer_ = nullptr;

    AtlasData* atlasData_ = {};

};
