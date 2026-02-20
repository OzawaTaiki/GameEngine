#pragma once

#include <Features/TextRenderer/AtlasData.h>
#include <Features/TextRenderer/TextParam.h>
#include <Math/Rect/Rect.h>


#ifdef DrawText
#undef DrawText
#endif // DrawText



namespace Engine {

class TextRenderer;
class TextGenerator
{
public:

    void Initialize(const FontConfig& _config);

    void Draw(const std::wstring& _text, const Vector2& _pos, const Vector4& _color = { 1,1,1,1 }, uint16_t _order = 0);

    void Draw(const std::wstring& _text, const TextParam& _param, uint16_t _order = 0);

    void Draw(const std::wstring& _text, const Rect& _rect, const Vector2& _pos, const Vector2& _piv ={ 0.5f,0.5f}, const Vector4& _color = { 1,1,1,1 }, uint16_t _order = 0);

    void Draw(const std::wstring& _text, const Rect& _rect, const TextParam& _param, uint16_t _order = 0);

    const AtlasData* GetAtlasData() const { return atlasData_; }


public: // 静的関数たち
    static void Draw(const std::string& text, const FontConfig& conf, const Vector2& pos, const Vector4& color = { 1,1,1,1 }, uint16_t order = 0);

    static void Draw(const std::string& text, const FontConfig& conf, const TextParam& param, uint16_t order = 0);

    static void Draw(const std::string& text, const FontConfig& conf, const Rect& rect, const Vector2& pos, const Vector2& piv = { 0.5f,0.5f }, const Vector4& color = { 1,1,1,1 }, uint16_t order = 0);

    static void Draw(const std::string& text, const FontConfig& conf, const Rect& rect, const TextParam& param, uint16_t order = 0);
private:

    TextRenderer* renderer_ = nullptr;

    AtlasData* atlasData_ = {};

};

} // namespace Engine
